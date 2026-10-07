#pragma once

#include <cstdint>

namespace PE_Hook {

    constexpr int ProcessEventIdx = 0x4C;

    inline bool ValidPtr(const void* ptr) {
        const auto address = reinterpret_cast<uintptr_t>(ptr);

        return address > 0xFFFFFF &&
            address < 0x7FFFFFFFFFFF;
    }

    inline bool ValidPtr(uintptr_t address) {
        return address > 0xFFFFFF &&
            address < 0x7FFFFFFFFFFF;
    }

    class VTable {
    public:
        int GetVTableSize();
        void FreeVTableCache();

        uintptr_t m_class = 0;
        uintptr_t* m_vtable = nullptr;
        int m_vtablesize = 0;
        uintptr_t* m_vtable_cache = nullptr;
    };

    class ProcessEventHook : public VTable {
    public:
        explicit ProcessEventHook(uintptr_t pClass) {
            m_class = pClass;
            m_vtable = *reinterpret_cast<uintptr_t**>(pClass);
            m_vtablesize = GetVTableSize();
        }

        void ApplyHook(uintptr_t classPtr,
            uintptr_t pOrgFunc,
            uintptr_t pFunc);

        int FindProcessEventIndex();

        ~ProcessEventHook() {
            FreeVTableCache();
        }

    private:
        int m_eventindex = 0;
    };
}
