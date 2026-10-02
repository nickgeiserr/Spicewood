#include "pch.h"
#include "Aegis.h"


void UnlinkDllFromPEB(HINSTANCE hModule) {
#if defined(_WIN64)
    PPEB_CUSTOM peb = (PPEB_CUSTOM)__readgsqword(0x60);
#else
    PPEB_CUSTOM peb = (PPEB_CUSTOM)__readfsdword(0x30);
#endif

    PPEB_LDR_DATA_CUSTOM ldr = peb->Ldr;
    PLIST_ENTRY currentEntry = ldr->InLoadOrderModuleList.Flink;

    while (currentEntry != &ldr->InLoadOrderModuleList) {
        PLDR_DATA_TABLE_ENTRY_CUSTOM ldrEntry = CONTAINING_RECORD(currentEntry, LDR_DATA_TABLE_ENTRY_CUSTOM, InLoadOrderLinks);

        if (ldrEntry->DllBase == hModule) {
            ldrEntry->InLoadOrderLinks.Blink->Flink = ldrEntry->InLoadOrderLinks.Flink;
            ldrEntry->InLoadOrderLinks.Flink->Blink = ldrEntry->InLoadOrderLinks.Blink;

            ldrEntry->InMemoryOrderLinks.Blink->Flink = ldrEntry->InMemoryOrderLinks.Flink;
            ldrEntry->InMemoryOrderLinks.Flink->Blink = ldrEntry->InMemoryOrderLinks.Blink;

            ldrEntry->InInitializationOrderLinks.Blink->Flink = ldrEntry->InInitializationOrderLinks.Flink;
            ldrEntry->InInitializationOrderLinks.Flink->Blink = ldrEntry->InInitializationOrderLinks.Blink;

            break;
        }
        currentEntry = currentEntry->Flink;
    }
}