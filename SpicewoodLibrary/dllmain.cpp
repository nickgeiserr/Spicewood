#include "pch.h"
#include "Spicewood.h"
#include "Aegis.h"
#include <Hooks.h>

#pragma comment(linker, "/export:curl_easy_cleanup=xcurl_orig.curl_easy_cleanup")
#pragma comment(linker, "/export:curl_easy_duphandle=xcurl_orig.curl_easy_duphandle")
#pragma comment(linker, "/export:curl_easy_escape=xcurl_orig.curl_easy_escape")
#pragma comment(linker, "/export:curl_easy_getinfo=xcurl_orig.curl_easy_getinfo")
#pragma comment(linker, "/export:curl_easy_init=xcurl_orig.curl_easy_init")
#pragma comment(linker, "/export:curl_easy_perform=xcurl_orig.curl_easy_perform")
#pragma comment(linker, "/export:curl_easy_reset=xcurl_orig.curl_easy_reset")
#pragma comment(linker, "/export:curl_easy_setopt=xcurl_orig.curl_easy_setopt")
#pragma comment(linker, "/export:curl_easy_strerror=xcurl_orig.curl_easy_strerror")
#pragma comment(linker, "/export:curl_easy_unescape=xcurl_orig.curl_easy_unescape")
#pragma comment(linker, "/export:curl_escape=xcurl_orig.curl_escape")
#pragma comment(linker, "/export:curl_formadd=xcurl_orig.curl_formadd")
#pragma comment(linker, "/export:curl_formfree=xcurl_orig.curl_formfree")
#pragma comment(linker, "/export:curl_formget=xcurl_orig.curl_formget")
#pragma comment(linker, "/export:curl_free=xcurl_orig.curl_free")
#pragma comment(linker, "/export:curl_getdate=xcurl_orig.curl_getdate")
#pragma comment(linker, "/export:curl_global_cleanup=xcurl_orig.curl_global_cleanup")
#pragma comment(linker, "/export:curl_global_init=xcurl_orig.curl_global_init")
#pragma comment(linker, "/export:curl_global_init_mem=xcurl_orig.curl_global_init_mem")
#pragma comment(linker, "/export:curl_global_sslset=xcurl_orig.curl_global_sslset")
#pragma comment(linker, "/export:curl_mime_addpart=xcurl_orig.curl_mime_addpart")
#pragma comment(linker, "/export:curl_mime_data=xcurl_orig.curl_mime_data")
#pragma comment(linker, "/export:curl_mime_data_cb=xcurl_orig.curl_mime_data_cb")
#pragma comment(linker, "/export:curl_mime_encoder=xcurl_orig.curl_mime_encoder")
#pragma comment(linker, "/export:curl_mime_filedata=xcurl_orig.curl_mime_filedata")
#pragma comment(linker, "/export:curl_mime_filename=xcurl_orig.curl_mime_filename")
#pragma comment(linker, "/export:curl_mime_free=xcurl_orig.curl_mime_free")
#pragma comment(linker, "/export:curl_mime_headers=xcurl_orig.curl_mime_headers")
#pragma comment(linker, "/export:curl_mime_init=xcurl_orig.curl_mime_init")
#pragma comment(linker, "/export:curl_mime_name=xcurl_orig.curl_mime_name")
#pragma comment(linker, "/export:curl_mime_subparts=xcurl_orig.curl_mime_subparts")
#pragma comment(linker, "/export:curl_mime_type=xcurl_orig.curl_mime_type")
#pragma comment(linker, "/export:curl_multi_add_handle=xcurl_orig.curl_multi_add_handle")
#pragma comment(linker, "/export:curl_multi_cleanup=xcurl_orig.curl_multi_cleanup")
#pragma comment(linker, "/export:curl_multi_info_read=xcurl_orig.curl_multi_info_read")
#pragma comment(linker, "/export:curl_multi_init=xcurl_orig.curl_multi_init")
#pragma comment(linker, "/export:curl_multi_perform=xcurl_orig.curl_multi_perform")
#pragma comment(linker, "/export:curl_multi_poll=xcurl_orig.curl_multi_poll")
#pragma comment(linker, "/export:curl_multi_remove_handle=xcurl_orig.curl_multi_remove_handle")
#pragma comment(linker, "/export:curl_multi_setopt=xcurl_orig.curl_multi_setopt")
#pragma comment(linker, "/export:curl_multi_strerror=xcurl_orig.curl_multi_strerror")
#pragma comment(linker, "/export:curl_multi_wait=xcurl_orig.curl_multi_wait")
#pragma comment(linker, "/export:curl_multi_wakeup=xcurl_orig.curl_multi_wakeup")
#pragma comment(linker, "/export:curl_share_cleanup=xcurl_orig.curl_share_cleanup")
#pragma comment(linker, "/export:curl_share_init=xcurl_orig.curl_share_init")
#pragma comment(linker, "/export:curl_share_setopt=xcurl_orig.curl_share_setopt")
#pragma comment(linker, "/export:curl_share_strerror=xcurl_orig.curl_share_strerror")
#pragma comment(linker, "/export:curl_slist_append=xcurl_orig.curl_slist_append")
#pragma comment(linker, "/export:curl_slist_free_all=xcurl_orig.curl_slist_free_all")
#pragma comment(linker, "/export:curl_unescape=xcurl_orig.curl_unescape")
#pragma comment(linker, "/export:curl_url=xcurl_orig.curl_url")
#pragma comment(linker, "/export:curl_url_cleanup=xcurl_orig.curl_url_cleanup")
#pragma comment(linker, "/export:curl_url_dup=xcurl_orig.curl_url_dup")
#pragma comment(linker, "/export:curl_url_get=xcurl_orig.curl_url_get")
#pragma comment(linker, "/export:curl_url_set=xcurl_orig.curl_url_set")
#pragma comment(linker, "/export:curl_version=xcurl_orig.curl_version")
#pragma comment(linker, "/export:curl_version_info=xcurl_orig.curl_version_info")
#pragma comment(linker, "/export:xcurl_global_init_mem=xcurl_orig.xcurl_global_init_mem")
#pragma comment(linker, "/export:xcurl_global_resume=xcurl_orig.xcurl_global_resume")
#pragma comment(linker, "/export:xcurl_global_set_request_limit=xcurl_orig.xcurl_global_set_request_limit")
#pragma comment(linker, "/export:xcurl_global_suspend=xcurl_orig.xcurl_global_suspend")


BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH: {
        DisableThreadLibraryCalls(hModule);
        HANDLE thread = CreateThread(nullptr, 0, MainThread, hModule, 0, nullptr);
        CloseHandle(thread);

        break;
    }

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        H_Shutdown();
        CleanupConsole();
        break;
    }
    return TRUE;
}


