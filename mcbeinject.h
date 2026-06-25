#pragma once

#include "stdafx.h"
#include "resource.h"

BOOL DllInjectW(DWORD procId, LPWSTR dllPath);
DWORD CreateMCBEProc(void);