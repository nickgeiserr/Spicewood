#include "pch.h"
#include "Spicewood.h"
#include "Aegis.h"
#include <Hooks.h>

#pragma comment(linker, "/export:CloseDriver=c:\\windows\\system32\\winmm.CloseDriver")
#pragma comment(linker, "/export:DefDriverProc=c:\\windows\\system32\\winmm.DefDriverProc")
#pragma comment(linker, "/export:DriverCallback=c:\\windows\\system32\\winmm.DriverCallback")
#pragma comment(linker, "/export:DrvGetModuleHandle=c:\\windows\\system32\\winmm.DrvGetModuleHandle")
#pragma comment(linker, "/export:GetDriverModuleHandle=c:\\windows\\system32\\winmm.GetDriverModuleHandle")
#pragma comment(linker, "/export:NotifyCallbackData=c:\\windows\\system32\\winmm.NotifyCallbackData")
#pragma comment(linker, "/export:OpenDriver=c:\\windows\\system32\\winmm.OpenDriver")
#pragma comment(linker, "/export:PlaySound=c:\\windows\\system32\\winmm.PlaySound")
#pragma comment(linker, "/export:PlaySoundA=c:\\windows\\system32\\winmm.PlaySoundA")
#pragma comment(linker, "/export:PlaySoundW=c:\\windows\\system32\\winmm.PlaySoundW")
#pragma comment(linker, "/export:SendDriverMessage=c:\\windows\\system32\\winmm.SendDriverMessage")
#pragma comment(linker, "/export:WOW32DriverCallback=c:\\windows\\system32\\winmm.WOW32DriverCallback")
#pragma comment(linker, "/export:WOW32ResolveMultiMediaHandle=c:\\windows\\system32\\winmm.WOW32ResolveMultiMediaHandle")
#pragma comment(linker, "/export:WOWAppExit=c:\\windows\\system32\\winmm.WOWAppExit")
#pragma comment(linker, "/export:aux32Message=c:\\windows\\system32\\winmm.aux32Message")
#pragma comment(linker, "/export:auxGetDevCapsA=c:\\windows\\system32\\winmm.auxGetDevCapsA")
#pragma comment(linker, "/export:auxGetDevCapsW=c:\\windows\\system32\\winmm.auxGetDevCapsW")
#pragma comment(linker, "/export:auxGetNumDevs=c:\\windows\\system32\\winmm.auxGetNumDevs")
#pragma comment(linker, "/export:auxGetVolume=c:\\windows\\system32\\winmm.auxGetVolume")
#pragma comment(linker, "/export:auxOutMessage=c:\\windows\\system32\\winmm.auxOutMessage")
#pragma comment(linker, "/export:auxSetVolume=c:\\windows\\system32\\winmm.auxSetVolume")
#pragma comment(linker, "/export:joy32Message=c:\\windows\\system32\\winmm.joy32Message")
#pragma comment(linker, "/export:joyConfigChanged=c:\\windows\\system32\\winmm.joyConfigChanged")
#pragma comment(linker, "/export:joyGetDevCapsA=c:\\windows\\system32\\winmm.joyGetDevCapsA")
#pragma comment(linker, "/export:joyGetDevCapsW=c:\\windows\\system32\\winmm.joyGetDevCapsW")
#pragma comment(linker, "/export:joyGetNumDevs=c:\\windows\\system32\\winmm.joyGetNumDevs")
#pragma comment(linker, "/export:joyGetPos=c:\\windows\\system32\\winmm.joyGetPos")
#pragma comment(linker, "/export:joyGetPosEx=c:\\windows\\system32\\winmm.joyGetPosEx")
#pragma comment(linker, "/export:joyGetThreshold=c:\\windows\\system32\\winmm.joyGetThreshold")
#pragma comment(linker, "/export:joyReleaseCapture=c:\\windows\\system32\\winmm.joyReleaseCapture")
#pragma comment(linker, "/export:joySetCapture=c:\\windows\\system32\\winmm.joySetCapture")
#pragma comment(linker, "/export:joySetThreshold=c:\\windows\\system32\\winmm.joySetThreshold")
#pragma comment(linker, "/export:mci32Message=c:\\windows\\system32\\winmm.mci32Message")
#pragma comment(linker, "/export:mciDriverNotify=c:\\windows\\system32\\winmm.mciDriverNotify")
#pragma comment(linker, "/export:mciDriverYield=c:\\windows\\system32\\winmm.mciDriverYield")
#pragma comment(linker, "/export:mciExecute=c:\\windows\\system32\\winmm.mciExecute")
#pragma comment(linker, "/export:mciFreeCommandResource=c:\\windows\\system32\\winmm.mciFreeCommandResource")
#pragma comment(linker, "/export:mciGetCreatorTask=c:\\windows\\system32\\winmm.mciGetCreatorTask")
#pragma comment(linker, "/export:mciGetDeviceIDA=c:\\windows\\system32\\winmm.mciGetDeviceIDA")
#pragma comment(linker, "/export:mciGetDeviceIDW=c:\\windows\\system32\\winmm.mciGetDeviceIDW")
#pragma comment(linker, "/export:mciGetDeviceIDFromElementIDA=c:\\windows\\system32\\winmm.mciGetDeviceIDFromElementIDA")
#pragma comment(linker, "/export:mciGetDeviceIDFromElementIDW=c:\\windows\\system32\\winmm.mciGetDeviceIDFromElementIDW")
#pragma comment(linker, "/export:mciGetDriverData=c:\\windows\\system32\\winmm.mciGetDriverData")
#pragma comment(linker, "/export:mciGetErrorStringA=c:\\windows\\system32\\winmm.mciGetErrorStringA")
#pragma comment(linker, "/export:mciGetErrorStringW=c:\\windows\\system32\\winmm.mciGetErrorStringW")
#pragma comment(linker, "/export:mciGetYieldProc=c:\\windows\\system32\\winmm.mciGetYieldProc")
#pragma comment(linker, "/export:mciLoadCommandResource=c:\\windows\\system32\\winmm.mciLoadCommandResource")
#pragma comment(linker, "/export:mciSendCommandA=c:\\windows\\system32\\winmm.mciSendCommandA")
#pragma comment(linker, "/export:mciSendCommandW=c:\\windows\\system32\\winmm.mciSendCommandW")
#pragma comment(linker, "/export:mciSendStringA=c:\\windows\\system32\\winmm.mciSendStringA")
#pragma comment(linker, "/export:mciSendStringW=c:\\windows\\system32\\winmm.mciSendStringW")
#pragma comment(linker, "/export:mciSetDriverData=c:\\windows\\system32\\winmm.mciSetDriverData")
#pragma comment(linker, "/export:mciSetYieldProc=c:\\windows\\system32\\winmm.mciSetYieldProc")
#pragma comment(linker, "/export:midi32Message=c:\\windows\\system32\\winmm.midi32Message")
#pragma comment(linker, "/export:midiConnect=c:\\windows\\system32\\winmm.midiConnect")
#pragma comment(linker, "/export:midiDisconnect=c:\\windows\\system32\\winmm.midiDisconnect")
#pragma comment(linker, "/export:midiInAddBuffer=c:\\windows\\system32\\winmm.midiInAddBuffer")
#pragma comment(linker, "/export:midiInClose=c:\\windows\\system32\\winmm.midiInClose")
#pragma comment(linker, "/export:midiInGetDevCapsA=c:\\windows\\system32\\winmm.midiInGetDevCapsA")
#pragma comment(linker, "/export:midiInGetDevCapsW=c:\\windows\\system32\\winmm.midiInGetDevCapsW")
#pragma comment(linker, "/export:midiInGetErrorTextA=c:\\windows\\system32\\winmm.midiInGetErrorTextA")
#pragma comment(linker, "/export:midiInGetErrorTextW=c:\\windows\\system32\\winmm.midiInGetErrorTextW")
#pragma comment(linker, "/export:midiInGetNumDevs=c:\\windows\\system32\\winmm.midiInGetNumDevs")
#pragma comment(linker, "/export:midiInMessage=c:\\windows\\system32\\winmm.midiInMessage")
#pragma comment(linker, "/export:midiInOpen=c:\\windows\\system32\\winmm.midiInOpen")
#pragma comment(linker, "/export:midiInPrepareHeader=c:\\windows\\system32\\winmm.midiInPrepareHeader")
#pragma comment(linker, "/export:midiInReset=c:\\windows\\system32\\winmm.midiInReset")
#pragma comment(linker, "/export:midiInStart=c:\\windows\\system32\\winmm.midiInStart")
#pragma comment(linker, "/export:midiInStop=c:\\windows\\system32\\winmm.midiInStop")
#pragma comment(linker, "/export:midiInUnprepareHeader=c:\\windows\\system32\\winmm.midiInUnprepareHeader")
#pragma comment(linker, "/export:midiOutLongMsg=c:\\windows\\system32\\winmm.midiOutLongMsg")
#pragma comment(linker, "/export:midiOutMessage=c:\\windows\\system32\\winmm.midiOutMessage")
#pragma comment(linker, "/export:midiOutOpen=c:\\windows\\system32\\winmm.midiOutOpen")
#pragma comment(linker, "/export:midiOutPrepareHeader=c:\\windows\\system32\\winmm.midiOutPrepareHeader")
#pragma comment(linker, "/export:midiOutReset=c:\\windows\\system32\\winmm.midiOutReset")
#pragma comment(linker, "/export:midiOutSetVolume=c:\\windows\\system32\\winmm.midiOutSetVolume")
#pragma comment(linker, "/export:midiOutShortMsg=c:\\windows\\system32\\winmm.midiOutShortMsg")
#pragma comment(linker, "/export:midiOutUnprepareHeader=c:\\windows\\system32\\winmm.midiOutUnprepareHeader")
#pragma comment(linker, "/export:mmDrvInstall=c:\\windows\\system32\\winmm.mmDrvInstall")
#pragma comment(linker, "/export:mmGetCurrentTask=c:\\windows\\system32\\winmm.mmGetCurrentTask")
#pragma comment(linker, "/export:mmTaskBlock=c:\\windows\\system32\\winmm.mmTaskBlock")
#pragma comment(linker, "/export:mmTaskCreate=c:\\windows\\system32\\winmm.mmTaskCreate")
#pragma comment(linker, "/export:mmTaskSignal=c:\\windows\\system32\\winmm.mmTaskSignal")
#pragma comment(linker, "/export:mmTaskYield=c:\\windows\\system32\\winmm.mmTaskYield")
#pragma comment(linker, "/export:mmioAdvance=c:\\windows\\system32\\winmm.mmioAdvance")
#pragma comment(linker, "/export:mmioAscend=c:\\windows\\system32\\winmm.mmioAscend")
#pragma comment(linker, "/export:mmioClose=c:\\windows\\system32\\winmm.mmioClose")
#pragma comment(linker, "/export:mmioCreateChunk=c:\\windows\\system32\\winmm.mmioCreateChunk")
#pragma comment(linker, "/export:mmioDescend=c:\\windows\\system32\\winmm.mmioDescend")
#pragma comment(linker, "/export:mmioFlush=c:\\windows\\system32\\winmm.mmioFlush")
#pragma comment(linker, "/export:mmioGetInfo=c:\\windows\\system32\\winmm.mmioGetInfo")
#pragma comment(linker, "/export:mmioInstallIOProcA=c:\\windows\\system32\\winmm.mmioInstallIOProcA")
#pragma comment(linker, "/export:mmioInstallIOProcW=c:\\windows\\system32\\winmm.mmioInstallIOProcW")
#pragma comment(linker, "/export:mmioOpenA=c:\\windows\\system32\\winmm.mmioOpenA")
#pragma comment(linker, "/export:mmioOpenW=c:\\windows\\system32\\winmm.mmioOpenW")
#pragma comment(linker, "/export:mmioRead=c:\\windows\\system32\\winmm.mmioRead")
#pragma comment(linker, "/export:mmioRenameA=c:\\windows\\system32\\winmm.mmioRenameA")
#pragma comment(linker, "/export:mmioRenameW=c:\\windows\\system32\\winmm.mmioRenameW")
#pragma comment(linker, "/export:mmioSeek=c:\\windows\\system32\\winmm.mmioSeek")
#pragma comment(linker, "/export:mmioSetBuffer=c:\\windows\\system32\\winmm.mmioSetBuffer")
#pragma comment(linker, "/export:mmioSetInfo=c:\\windows\\system32\\winmm.mmioSetInfo")
#pragma comment(linker, "/export:mmioStringToFOURCCA=c:\\windows\\system32\\winmm.mmioStringToFOURCCA")
#pragma comment(linker, "/export:mmioStringToFOURCCW=c:\\windows\\system32\\winmm.mmioStringToFOURCCW")
#pragma comment(linker, "/export:mmioWrite=c:\\windows\\system32\\winmm.mmioWrite")
#pragma comment(linker, "/export:mmsystemGetVersion=c:\\windows\\system32\\winmm.mmsystemGetVersion")



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
        Print(PrintType::Debug, "Thread detached");
        break;
    case DLL_PROCESS_DETACH:
        Hooks::Shutdown();
        CleanupConsole();
        break;
    }
    return TRUE;
}


