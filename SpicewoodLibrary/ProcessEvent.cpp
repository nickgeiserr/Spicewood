#include "pch.h"
#include "ProcessEvent.h"
#include <winnt.h>
#include <memoryapi.h>
#include <cstdlib>
#include <cstring>
#include <Engine/Offsets.h>

using namespace PE_Hook;

int VTable::GetVTableSize() {
    if (!ValidPtr(m_vtable))
        return 0;

    int count = 0;
    MEMORY_BASIC_INFORMATION mbi{};

    while (VirtualQuery(
        reinterpret_cast<LPCVOID>(m_vtable[count]),
        &mbi,
        sizeof(mbi)
    )) {
        if (mbi.State != MEM_COMMIT)
            break;

        if (mbi.Protect & PAGE_NOACCESS)
            break;

        if (!(mbi.Protect & PAGE_EXECUTE) &&
            !(mbi.Protect & PAGE_EXECUTE_READ) &&
            !(mbi.Protect & PAGE_EXECUTE_READWRITE) &&
            !(mbi.Protect & PAGE_EXECUTE_WRITECOPY))
            break;

        if (!ValidPtr(m_vtable[count]))
            break;

        count++;
    }

    return count;
}

void VTable::FreeVTableCache() {
    if (m_vtable_cache) {
        free(m_vtable_cache);
        m_vtable_cache = nullptr;
    }
}

int ProcessEventHook::FindProcessEventIndex()
{
    if (!ValidPtr((void*)m_vtable) || this->m_vtablesize == -1)
        return NULL;

    static auto module_base = (std::uintptr_t)GetModuleHandleA(NULL);

    for (int index = 0; index <= this->m_vtablesize; index++)
    {
        auto function = *reinterpret_cast<std::uintptr_t*>(this->m_vtable + (index * 0x8));

        if (!ValidPtr((void*)function))
            continue;

        if (function == (module_base + PROCESS_EVENT))
        {
            return index;
        }
    }

    return -1;
}

void ProcessEventHook::ApplyHook(
    uintptr_t pClass,
    uintptr_t pOrgFunc,
    uintptr_t pFunc
) {
    if (m_eventindex < 0 || m_vtablesize <= m_eventindex)
        return;

    if (!ValidPtr(pClass))
        return;

    auto vtable = *reinterpret_cast<uintptr_t**>(pClass);

    if (!ValidPtr(vtable))
        return;

    m_vtable = vtable;

    const size_t tableSize =
        static_cast<size_t>(m_vtablesize) * sizeof(uintptr_t);

    m_vtable_cache =
        reinterpret_cast<uintptr_t*>(malloc(tableSize));

    if (!m_vtable_cache)
        return;

    memcpy(
        m_vtable_cache,
        m_vtable,
        tableSize
    );

    m_vtable_cache[m_eventindex] = pFunc;

    *reinterpret_cast<uintptr_t**>(pClass) = m_vtable_cache;

    m_class = pClass;
}
