#include "mcbeinject.h"

/*
   This procedure returns the process ID of the running instance of Minecraft BE.
   0 is returned as the PID if we failed to create a valid process ID.
 */
DWORD CreateMCBEProc(void)
{
    HRESULT hr;
    DWORD pid = 0;

    // While IApplicationActivationManager does not require a multithreaded apartment, using an MTA has some
    // benefits; we do not need a message pump nor do we need unnecessary serialisation.
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    // If the call to initialise COM does succeed, we must balance this once we are done with a call to
    // CoUninitialize.
    BOOL comInitSuccess = SUCCEEDED(hr);

    // If it is true that COM was initialised by a previous call, even if it does not use a compatible
    // concurrency mode, we may proceed nonetheless.
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE)
    {
        MessageBoxW(NULL, L"Failed to initialise COM.", L"MCBEInject", MB_OK | MB_ICONERROR);
        return 0;
    }

    // CreateProcessW does not work on Store apps; therefore in this case, we should use the
    // IApplicationActivationManager COM interface to do so instead.
    // https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nn-shobjidl_core-iapplicationactivationmanager
    IApplicationActivationManager* pAAM = NULL;

    hr = CoCreateInstance(
        &CLSID_ApplicationActivationManager,
        NULL,
        CLSCTX_LOCAL_SERVER,
        &IID_IApplicationActivationManager,
        (void**)&pAAM
    );

    if (FAILED(hr))
    {
        // If CoInitializeEx had returned RPC_E_CHANGED_MODE, we do not owe COM a CoUninitialize, but we would
        // have still reached this point. If it returned a value greater than or equal to 0, we do owe COM a 
        // CoUninitialize. 
        if (comInitSuccess)
        {
            CoUninitialize();
        }

        MessageBoxW(NULL, L"Failed to create an instance of ApplicationActivationManager.", L"MCBEInject", MB_OK | MB_ICONERROR);
        return 0;
    }

    hr = pAAM->lpVtbl->ActivateApplication(
        pAAM, // (usually implicit) this
        L"Microsoft.MinecraftUWP_8wekyb3d8bbwe!Game", // Seems that Microsoft still uses the old UWP AUMID...
        NULL,
        AO_NONE,
        &pid
    );

    pAAM->lpVtbl->Release(pAAM);
    pAAM = NULL;

    if (comInitSuccess)
    {
        CoUninitialize();
    }

    if (FAILED(hr))
    {
        MessageBoxW(NULL, L"It appears that Minecraft BE cannot be launched.", L"MCBEInject", MB_OK | MB_ICONERROR);
        return 0;
    }

    return pid;
}
