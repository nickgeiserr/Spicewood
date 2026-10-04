// Imports a Dumper-7 .idmap file (the "IDAMappings" output) into the current program.
//
// Names Unreal's native exec functions, vtables and engine globals (GObjects, ProcessEvent...),
// and can also create every reflected struct and enum as Ghidra data types under /Dumper7.
// Names you've set yourself are never overwritten, so it's safe to run again after a redump.
//
// Addresses in the file are relative to the image base, so this works on the exe or on a
// memory dump as long as the program's image base is set correctly.
//
//@category Unreal
//@menupath Tools.Unreal.Import Dumper-7 IDMap

import java.io.File;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import ghidra.app.script.GhidraScript;
import ghidra.app.util.NamespaceUtils;
import ghidra.program.model.address.Address;
import ghidra.program.model.data.*;
import ghidra.program.model.listing.Function;
import ghidra.program.model.symbol.Namespace;
import ghidra.program.model.symbol.SourceType;
import ghidra.program.model.symbol.Symbol;
import ghidra.program.model.symbol.SymbolUtilities;

public class ImportDumper7IdMap extends GhidraScript {

	private static final int MAGIC = 0xD7;
	private static final int NO_STRING = 0xFFFFFFFF;
	private static final int NOT_A_BITFIELD = 0xFF;
	private static final CategoryPath CATEGORY = new CategoryPath("/Dumper7");

	private ByteBuffer buf;
	private int stringBase;
	private Address imageBase;
	private DataTypeManager dtm;

	private final Map<String, DataType> enums = new HashMap<>();
	private final Map<String, Structure> structs = new HashMap<>();
	private final Map<String, DataType> primitives = new HashMap<>();

	private int named, skippedUser, notInMemory, failed;

	private static class Member {
		String type, name;
		int offset, size, arrayDim, bitCount;
		boolean isPointer;
	}

	private static class StructRecord {
		String name, superName;
		int size;
		List<Member> members = new ArrayList<>();
		Structure dt;
	}

	@Override
	protected void run() throws Exception {
		File file;
		boolean importTypes;

		String[] args = getScriptArgs();
		if (args.length > 0) {
			file = new File(args[0]);
			importTypes = args.length < 2 || Boolean.parseBoolean(args[1]);
		}
		else {
			file = askFile("Select the Dumper-7 .idmap file", "Import");
			importTypes = askYesNo("Import types too?",
				"Also create Dumper-7's structs and enums as Ghidra data types (under /Dumper7)?\n\n" +
				"This takes a few minutes. Functions, vtables and globals are named either way.");
		}

		buf = ByteBuffer.wrap(Files.readAllBytes(file.toPath())).order(ByteOrder.LITTLE_ENDIAN);
		if ((buf.get(0) & 0xFF) != MAGIC) {
			printerr("Not a Dumper-7 .idmap file (bad magic byte).");
			return;
		}

		int version = buf.get(1) & 0xFF;
		stringBase = buf.getInt(7);
		int numEnums = buf.getInt(11), enumData = buf.getInt(15);
		int numStructs = buf.getInt(19), structData = buf.getInt(23);
		int numGlobals = buf.getInt(27), globalData = buf.getInt(31);
		int numVTables = buf.getInt(35), vtableData = buf.getInt(39);
		int numExecs = buf.getInt(43), execData = buf.getInt(47);

		imageBase = currentProgram.getImageBase();
		dtm = currentProgram.getDataTypeManager();

		println(String.format("idmap v%d: %d enums, %d structs, %d globals, %d vtables, %d exec functions",
			version, numEnums, numStructs, numGlobals, numVTables, numExecs));
		println("Using image base " + imageBase + " (addresses in the file are relative to this)");

		if (importTypes) {
			setupPrimitives();
			importEnums(numEnums, enumData);
			importStructs(numStructs, structData);
		}

		importGlobals(numGlobals, globalData);
		importVTables(numVTables, vtableData);
		importExecFunctions(numExecs, execData, version >= 2 ? 20 : 12);

		println(String.format("Done. Named %d, kept %d of your own names, %d not in memory, %d failed.",
			named, skippedUser, notInMemory, failed));
		if (notInMemory > numExecs / 2) {
			printerr("Most addresses weren't in memory. Check that the program's image base matches the " +
				"module base the dump was taken at (Window > Memory Map > Set Image Base).");
		}
	}

	private String str(int offset) {
		if (offset == NO_STRING) {
			return null;
		}
		int at = stringBase + offset;
		int length = Short.toUnsignedInt(buf.getShort(at));
		byte[] bytes = new byte[length];
		buf.get(at + 2, bytes);
		return new String(bytes, StandardCharsets.UTF_8);
	}

	private Address addr(int offset) {
		return imageBase.add(Integer.toUnsignedLong(offset));
	}

	// ---------------------------------------------------------------- names

	private void importGlobals(int count, int at) throws Exception {
		monitor.setMessage("Naming globals");
		for (int i = 0; i < count; i++) {
			int offset = buf.getInt(at + i * 12);
			String type = str(buf.getInt(at + i * 12 + 4));
			String name = str(buf.getInt(at + i * 12 + 8));
			Address a = addr(offset);

			if (type != null && type.contains("(*)")) {
				nameFunction(a, name, type);
			}
			else {
				label(a, name, type);
				applyGlobalType(a, type);
			}
			println("  " + name + " @ " + a);
		}
	}

	private void importVTables(int count, int at) throws Exception {
		monitor.setMessage("Naming vtables");
		monitor.initialize(count);
		for (int i = 0; i < count; i++) {
			monitor.checkCancelled();
			monitor.setProgress(i);
			int offset = buf.getInt(at + i * 12);
			String name = str(buf.getInt(at + i * 12 + 8));
			label(addr(offset), name, null);
		}
	}

	private void importExecFunctions(int count, int at, int stride) throws Exception {
		// Identical exec thunks can be folded into one address by the linker, so group names by address
		// and give the address one name, listing the rest in its comment.
		Map<Long, List<String[]>> byAddress = new LinkedHashMap<>();
		for (int i = 0; i < count; i++) {
			int rec = at + i * stride;
			String mangled = str(buf.getInt(rec));
			String unmangled = str(buf.getInt(rec + 4));
			int offset = buf.getInt(rec + 8);
			String signature = stride >= 20 ? str(buf.getInt(rec + 12)) : null;

			String name = execName(mangled, unmangled);
			byAddress.computeIfAbsent(Integer.toUnsignedLong(offset), k -> new ArrayList<>())
				.add(new String[] { name, unmangled, signature });
		}

		monitor.setMessage("Naming native functions");
		monitor.initialize(byAddress.size());
		int progress = 0;
		for (Map.Entry<Long, List<String[]>> entry : byAddress.entrySet()) {
			monitor.checkCancelled();
			monitor.setProgress(progress++);

			List<String[]> names = entry.getValue();
			StringBuilder comment = new StringBuilder();
			for (String[] n : names) {
				if (comment.length() > 0) {
					comment.append("\n\n");
				}
				comment.append("Native thunk for UFunction ").append(n[1]);
				if (n[2] != null && !n[2].isEmpty()) {
					comment.append("\n").append(n[2]);
				}
			}
			nameFunction(imageBase.add(entry.getKey()), names.get(0)[0], comment.toString());
		}
	}

	// "_ZN6AActor16execAddComponentEv" -> "AActor::execAddComponent"
	private String execName(String mangled, String unmangled) {
		if (mangled != null && mangled.startsWith("_ZN")) {
			List<String> parts = new ArrayList<>();
			int i = 3;
			while (i < mangled.length() && Character.isDigit(mangled.charAt(i))) {
				int j = i;
				while (j < mangled.length() && Character.isDigit(mangled.charAt(j))) {
					j++;
				}
				int length = Integer.parseInt(mangled.substring(i, j));
				if (j + length > mangled.length()) {
					break;
				}
				parts.add(mangled.substring(j, j + length));
				i = j + length;
			}
			if (parts.size() >= 2) {
				return String.join("::", parts);
			}
		}
		if (unmangled != null) {
			int split = unmangled.lastIndexOf("::");
			return split < 0 ? "exec" + unmangled
				: unmangled.substring(0, split + 2) + "exec" + unmangled.substring(split + 2);
		}
		return null;
	}

	private void nameFunction(Address a, String qualifiedName, String comment) {
		if (qualifiedName == null) {
			return;
		}
		if (!currentProgram.getMemory().contains(a)) {
			notInMemory++;
			return;
		}
		try {
			Function fn = getFunctionAt(a);
			if (fn == null) {
				if (getInstructionAt(a) == null) {
					disassemble(a);
				}
				fn = createFunction(a, null);
			}
			if (fn == null) {
				label(a, qualifiedName, comment);
				return;
			}
			if (fn.getSymbol().getSource() == SourceType.USER_DEFINED) {
				skippedUser++;
				return;
			}

			int split = qualifiedName.lastIndexOf("::");
			String name = SymbolUtilities.replaceInvalidChars(
				split < 0 ? qualifiedName : qualifiedName.substring(split + 2), true);
			Namespace ns = split < 0 ? currentProgram.getGlobalNamespace()
				: NamespaceUtils.createNamespaceHierarchy(qualifiedName.substring(0, split), null,
					currentProgram, SourceType.IMPORTED);

			fn.setParentNamespace(ns);
			fn.setName(name, SourceType.IMPORTED);
			if (comment != null) {
				setPlateComment(a, comment);
			}
			named++;
		}
		catch (Exception e) {
			failed++;
		}
	}

	private void label(Address a, String name, String comment) {
		if (name == null) {
			return;
		}
		if (!currentProgram.getMemory().contains(a)) {
			notInMemory++;
			return;
		}
		try {
			Symbol existing = getSymbolAt(a);
			if (existing != null && existing.getSource() == SourceType.USER_DEFINED) {
				skippedUser++;
				return;
			}
			createLabel(a, SymbolUtilities.replaceInvalidChars(name, true), true, SourceType.IMPORTED);
			if (comment != null) {
				setPlateComment(a, comment);
			}
			named++;
		}
		catch (Exception e) {
			failed++;
		}
	}

	private void applyGlobalType(Address a, String type) {
		if (type == null || !currentProgram.getMemory().contains(a)) {
			return;
		}
		Structure s = structs.get(type.replace("struct ", "").replace("class ", "").trim());
		if (s == null || s.getLength() == 0) {
			return;
		}
		try {
			clearListing(a, a.add(s.getLength() - 1));
			createData(a, s);
		}
		catch (Exception e) {
			// leave the label without a type
		}
	}

	// ---------------------------------------------------------------- types

	private void setupPrimitives() {
		primitives.put("bool", BooleanDataType.dataType);
		primitives.put("char", CharDataType.dataType);
		primitives.put("wchar_t", WideChar16DataType.dataType);
		primitives.put("float", FloatDataType.dataType);
		primitives.put("double", DoubleDataType.dataType);
		primitives.put("int", IntegerDataType.dataType);
		primitives.put("unsigned int", UnsignedIntegerDataType.dataType);
		primitives.put("__int8", SignedByteDataType.dataType);
		primitives.put("unsigned __int8", ByteDataType.dataType);
		primitives.put("__int16", ShortDataType.dataType);
		primitives.put("unsigned __int16", UnsignedShortDataType.dataType);
		primitives.put("__int32", IntegerDataType.dataType);
		primitives.put("unsigned __int32", UnsignedIntegerDataType.dataType);
		primitives.put("__int64", LongLongDataType.dataType);
		primitives.put("unsigned __int64", UnsignedLongLongDataType.dataType);
		primitives.put("void", VoidDataType.dataType);
	}

	private void importEnums(int count, int at) throws Exception {
		monitor.setMessage("Creating enums");
		monitor.initialize(count);
		int p = at;
		for (int i = 0; i < count; i++) {
			monitor.checkCancelled();
			monitor.setProgress(i);

			String name = str(buf.getInt(p));
			int size = buf.get(p + 4) & 0xFF;
			int numValues = buf.getInt(p + 5);
			p += 9;

			if (size != 1 && size != 2 && size != 4 && size != 8) {
				size = 4;
			}
			EnumDataType e = new EnumDataType(CATEGORY, uniqueTypeName(name), size, dtm);
			for (int v = 0; v < numValues; v++) {
				String valueName = str(buf.getInt(p));
				long value = buf.getLong(p + 4);
				p += 12;
				try {
					String n = valueName == null ? "Value" + v : valueName;
					if (e.contains(n)) {
						n = n + "_" + v;
					}
					e.add(n, value);
				}
				catch (Exception ignored) {
					// value doesn't fit the enum's size
				}
			}
			enums.putIfAbsent(name, dtm.addDataType(e, DataTypeConflictHandler.REPLACE_HANDLER));
		}
		println("  created " + enums.size() + " enums");
	}

	private void importStructs(int count, int at) throws Exception {
		List<StructRecord> records = new ArrayList<>();
		int p = at;
		for (int i = 0; i < count; i++) {
			StructRecord r = new StructRecord();
			r.name = str(buf.getInt(p));
			r.superName = str(buf.getInt(p + 4));
			r.size = buf.getInt(p + 8);
			int numMembers = buf.getInt(p + 16);
			p += 20;
			for (int m = 0; m < numMembers; m++) {
				Member mem = new Member();
				mem.type = str(buf.getInt(p));
				mem.name = str(buf.getInt(p + 4));
				mem.offset = buf.getInt(p + 8);
				mem.size = buf.getInt(p + 12);
				mem.arrayDim = buf.getInt(p + 16);
				mem.isPointer = buf.get(p + 20) != 0;
				mem.bitCount = buf.get(p + 21) & 0xFF;
				p += 22;
				r.members.add(mem);
			}
			records.add(r);
		}

		// Pass 1: create every struct empty at its real size, so members can refer to any of them.
		monitor.setMessage("Creating structs");
		monitor.initialize(records.size());
		for (int i = 0; i < records.size(); i++) {
			monitor.checkCancelled();
			monitor.setProgress(i);
			StructRecord r = records.get(i);
			StructureDataType s = new StructureDataType(CATEGORY, uniqueTypeName(r.name), Math.max(r.size, 0), dtm);
			r.dt = (Structure) dtm.addDataType(s, DataTypeConflictHandler.REPLACE_HANDLER);
			structs.putIfAbsent(r.name, r.dt);
		}

		// Pass 2: fill in the parent class and the members.
		monitor.setMessage("Filling in struct members");
		monitor.initialize(records.size());
		int unresolved = 0;
		for (int i = 0; i < records.size(); i++) {
			monitor.checkCancelled();
			monitor.setProgress(i);
			StructRecord r = records.get(i);

			Structure parent = r.superName == null ? null : structs.get(r.superName);
			if (parent != null && parent.getLength() > 0 && parent.getLength() <= r.dt.getLength()) {
				tryReplace(r.dt, 0, parent, parent.getLength(), "Super", null);
			}

			Map<Integer, List<Member>> bitfields = new LinkedHashMap<>();
			for (Member m : r.members) {
				if (m.size <= 0 || m.offset < 0 || m.offset + m.size > r.dt.getLength()) {
					continue;
				}
				if (m.bitCount != NOT_A_BITFIELD) {
					bitfields.computeIfAbsent(m.offset, k -> new ArrayList<>()).add(m);
					continue;
				}
				DataType dt = resolve(m);
				String comment = null;
				if (dt == null) {
					unresolved++;
					dt = new ArrayDataType(Undefined1DataType.dataType, m.size, 1, dtm);
					comment = m.type;
				}
				tryReplace(r.dt, m.offset, dt, m.size, m.name, comment);
			}

			// Ghidra bitfields shift neighbouring fields in non-packed structs, so instead each byte of
			// bitfields becomes one plain field with the individual bits listed in its comment.
			for (Map.Entry<Integer, List<Member>> bf : bitfields.entrySet()) {
				List<Member> bits = bf.getValue();
				StringBuilder comment = new StringBuilder("bitfield:");
				for (Member b : bits) {
					comment.append(' ').append(b.name).append('(').append(b.bitCount).append(')');
				}
				int size = bits.get(0).size;
				DataType dt = size == 1 ? ByteDataType.dataType : new ArrayDataType(ByteDataType.dataType, size, 1, dtm);
				tryReplace(r.dt, bf.getKey(), dt, size, bits.get(0).name, comment.toString());
			}
		}
		println("  created " + structs.size() + " structs (" + unresolved + " members kept as raw bytes)");
	}

	private DataType resolve(Member m) {
		String t = m.type == null ? "" : m.type.trim();
		int pointers = m.isPointer ? 1 : 0;
		while (t.endsWith("*")) {
			pointers++;
			t = t.substring(0, t.length() - 1).trim();
		}
		for (String prefix : new String[] { "struct ", "class ", "enum " }) {
			if (t.startsWith(prefix)) {
				t = t.substring(prefix.length()).trim();
			}
		}

		DataType dt = primitives.get(t);
		if (dt == null) dt = enums.get(t);
		if (dt == null) dt = structs.get(t);
		if (dt == null && pointers > 0) dt = VoidDataType.dataType;
		if (dt == null) return null;

		for (int i = 0; i < pointers; i++) {
			dt = new PointerDataType(dt, 8, dtm);
		}
		if (m.arrayDim > 1 && dt.getLength() > 0 && dt.getLength() * m.arrayDim == m.size) {
			dt = new ArrayDataType(dt, m.arrayDim, dt.getLength(), dtm);
		}
		return dt.getLength() == m.size ? dt : null;
	}

	private void tryReplace(Structure s, int offset, DataType dt, int length, String name, String comment) {
		try {
			s.replaceAtOffset(offset, dt, length, name, comment);
		}
		catch (Exception e) {
			// overlaps something already placed; skip this member
		}
	}

	private final Map<String, Integer> typeNameCounts = new HashMap<>();

	private String uniqueTypeName(String name) {
		String base = name == null ? "Unnamed" : SymbolUtilities.replaceInvalidChars(name, true);
		int n = typeNameCounts.merge(base, 1, Integer::sum);
		return n == 1 ? base : base + "_" + n;
	}
}
