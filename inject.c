#include "mcbeinject.h"

/*
   Given a process ID and a path to a DLL, this procedure injects the DLL into the process which is
   identified by the given process ID.

   One should note that it is assumed to be the case that the caller provides a valid path to the DLL;
   it is also assumed to be true that the provided process ID is valid.
 */

BOOL DllInjectW(DWORD procId, LPWSTR dllPath)
{
	// Since it is the case that lstrlenW returns the size in characters, we must
	// multiply that size by the size of a wide character in order to get the
	// actual size in bytes, as is used in VirtualAllocEx() and WriteProcessMemory().
	// 1 is added to account for the null-terminator.
	size_t dllPathLenBytes = (size_t)((lstrlenW(dllPath) + 1) * sizeof(WCHAR));

	HANDLE hProc = OpenProcess(
		PROCESS_CREATE_THREAD |
		PROCESS_VM_OPERATION |
		PROCESS_VM_WRITE |
		PROCESS_QUERY_INFORMATION |
		PROCESS_VM_READ,
		FALSE, procId
	);
	if (hProc == NULL)
	{
		// Obviously, we could do more detailed error logging with GetLastError();
		// however it is the case that I do not consider it worthwhile at the present
		// time to do so. The same applies to subsequent error handling.
		MessageBoxW(NULL, L"OpenProcess failed.", L"MCBEInject", MB_OK | MB_ICONERROR);
		return FALSE;
	}

	LPVOID memBaseAddr = VirtualAllocEx(hProc, NULL, dllPathLenBytes, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (memBaseAddr == NULL)
	{
		MessageBoxW(NULL, L"VirtualAllocEx failed.", L"MCBEInject", MB_OK | MB_ICONERROR);
		CloseHandle(hProc);
		return FALSE;
	}

	size_t bytesWritten = 0;
	BOOL result = WriteProcessMemory(hProc, memBaseAddr, dllPath, dllPathLenBytes, &bytesWritten);
	if (result == FALSE || bytesWritten != dllPathLenBytes)
	{
		MessageBoxW(NULL, L"WriteProcessMemory failed.", L"MCBEInject", MB_OK | MB_ICONERROR);
		goto cleanup2;
	}

	// While it is indeed true that one would be well-advised to determine the address of LoadLibraryW by
	// determining where kernel32.dll is loaded in the target process, and then find the relative virtual address 
	// of LoadLibraryW and take the sum of these two values in order to determine it's whereabouts, it is well-known 
	// that system DLLs are consistently mapped between processes in Windows; therefore as of now it makes little 
	// sense to take such a detour that has little in the way of practical benefits.
	HANDLE hThread = CreateRemoteThread(hProc, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibraryW, memBaseAddr, 0, NULL);

	if (hThread == NULL)
	{
		MessageBoxW(NULL, L"CreateRemoteThread failed.", L"MCBEInject", MB_OK | MB_ICONERROR);
		goto cleanup2;
	}

	// We do not wish to destroy the existing resources before the thread may be able to do it's work; consequently,
	// we wait for the thread to terminate before cleaning up the objects in question.
	// The default security descriptor as used above is sufficient, as it includes the SYNCHRONIZE access right.
	WaitForSingleObject(hThread, INFINITE);

	// The following is the resource cleanup upon success. Accordingly, we do not check whether the resources exist,
	// because if they did not, the procedure would have already terminated due to one of the earlier error checks.
	CloseHandle(hThread);
	VirtualFreeEx(hProc, memBaseAddr, 0, MEM_RELEASE);
	CloseHandle(hProc);
	return TRUE;

	// The cleanup2 label exists to deal with the numerous occurences of the situation wherein
	// the following are both true: memory has been allocated and the handle for the process is open, but where it
	// is untrue that the handle to the remote thread is open.
cleanup2:
	VirtualFreeEx(hProc, memBaseAddr, 0, MEM_RELEASE);
	CloseHandle(hProc);
	return FALSE;
}