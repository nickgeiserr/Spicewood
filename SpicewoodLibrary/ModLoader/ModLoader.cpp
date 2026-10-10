#include "pch.h"
#include "ModLoader.h"
#include <Scripting/Scripting.h>
#include <Core/Console.h>
#include <exception>
#include <filesystem>
#include <fstream>
#include <format>
#include <nlohmann/json.hpp>
#include <system_error>
#include <string>
#include <utility>
#include <vector>

namespace {
	using Json = nlohmann::json;
	std::vector<Mod::Mod> loadedMods;

	std::filesystem::path GetModsRoot() {
		std::wstring exePath(32768, L'\0');
		DWORD length = GetModuleFileNameW(nullptr, exePath.data(), static_cast<DWORD>(exePath.size()));

		if (length == 0 || static_cast<std::size_t>(length) >= exePath.size())
			return {};

		exePath.resize(length);
		return std::filesystem::path(exePath).parent_path().parent_path().parent_path() / L"Mods";
	}

	void LoadMod(const std::filesystem::path& modFolder) {
		try {
			std::ifstream file(modFolder / L"mod.json");
			if (!file) {
				Print(PrintType::Warning, std::format("Could not open mod.json in {}", modFolder.string()));
				return;
			}

			Json manifest = Json::parse(file);
			std::string id = manifest.at("id").get<std::string>();
			std::string name = manifest.at("name").get<std::string>();
			std::string author = manifest.at("author").get<std::string>();
			std::string modFileName = manifest.at("modfile").get<std::string>();

			std::u8string modFileNameUtf8;
			modFileNameUtf8.reserve(modFileName.size());
			for (unsigned char character : modFileName)
				modFileNameUtf8.push_back(static_cast<char8_t>(character));
			std::filesystem::path relativeScript(modFileNameUtf8);
			if (relativeScript.empty() || relativeScript.is_absolute() || relativeScript.has_parent_path()) {
				Print(PrintType::Warning, std::format("Mod {} has an invalid modfile entry", name));
				return;
			}

			std::filesystem::path scriptPath = modFolder / relativeScript;
			std::error_code fileError;
			if (!std::filesystem::is_regular_file(scriptPath, fileError) || fileError) {
				Print(PrintType::Warning, std::format("Mod {} script is missing or inaccessible: {}", name, scriptPath.string()));
				return;
			}

			Mod::Mod mod(id, name, author, scriptPath.string());
			if (mod.initialize() != Scripting::LoadResult::Success) {
				Print(PrintType::Warning, std::format("Mod Failed to load: {}", name));
				return;
			}

			loadedMods.push_back(std::move(mod));
			Print(PrintType::Info, std::format("Mod Loaded: {}", name));
		}
		catch (const std::exception& error) {
			Print(PrintType::Error, std::format("Failed to load mod in {}: {}", modFolder.string(), error.what()));
		}
	}
}

namespace ModLoader {
	std::size_t LoadMods() {
		std::filesystem::path modsRoot;
		try {
			modsRoot = GetModsRoot();
		}
		catch (const std::exception& error) {
			Print(PrintType::Error, std::format("Could not resolve the Mods folder: {}", error.what()));
			return 0;
		}

		if (modsRoot.empty()) {
			Print(PrintType::Error, "Could not resolve the game executable path; mods will not be loaded");
			return 0;
		}

		std::error_code rootError;
		std::filesystem::file_status rootStatus = std::filesystem::status(modsRoot, rootError);
		if (rootError) {
			Print(PrintType::Error, std::format("Cannot access Mods folder {}: {}", modsRoot.string(), rootError.message()));
			return 0;
		}

		if (!std::filesystem::exists(rootStatus)) {
			Print(PrintType::Warning, std::format("Mods folder does not exist: {}", modsRoot.string()));
			return 0;
		}

		if (!std::filesystem::is_directory(rootStatus)) {
			Print(PrintType::Error, std::format("Mods path is not a directory: {}", modsRoot.string()));
			return 0;
		}

		std::error_code iteratorError;
		std::filesystem::directory_iterator iterator(modsRoot, iteratorError);
		if (iteratorError) {
			Print(PrintType::Error, std::format("Cannot enumerate Mods folder {}: {}", modsRoot.string(), iteratorError.message()));
			return 0;
		}

		const std::filesystem::directory_iterator end;
		while (iterator != end) {
			const std::filesystem::directory_entry entry = *iterator;
			std::error_code entryError;
			if (entry.is_directory(entryError))
				LoadMod(entry.path());
			else if (entryError)
				Print(PrintType::Warning, std::format("Cannot inspect mod entry {}: {}", entry.path().string(), entryError.message()));

			iterator.increment(iteratorError);
			if (iteratorError) {
				Print(PrintType::Error, std::format("Error while enumerating Mods folder {}: {}", modsRoot.string(), iteratorError.message()));
				break;
			}
		}

		return loadedMods.size();
	}

	void Shutdown() {
		loadedMods.clear();
	}
}
