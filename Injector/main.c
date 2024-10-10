#include <Windows.h>
#include <Psapi.h>
#include <sddl.h>
#include <stdio.h>

#pragma comment(lib, "advapi32.lib")

HANDLE GetProcessByName(LPWSTR procname, DWORD *pdwPID)
{
	DWORD need;
	DWORD pids[2048] = { 0 };

	if (!EnumProcesses(pids, sizeof(pids), &need))
		return NULL;

	for (DWORD i = 0; i < need / sizeof(DWORD); ++i) {
		DWORD dw;
		HMODULE mod;
		HANDLE proc;
		WCHAR pn[MAX_PATH] = { 0 };

		proc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pids[i]);
		if (proc) {
			if (EnumProcessModules(proc, &mod, sizeof(mod), &dw)) {
				GetModuleBaseNameW(proc, mod, pn, ARRAYSIZE(pn));
				if (_wcsicmp(pn, procname) == 0) {
					CloseHandle(proc);
					*pdwPID = pids[i];
					return OpenProcess(PROCESS_ALL_ACCESS, FALSE, pids[i]);
				}
			}
			CloseHandle(proc);
		}
	}
	return NULL;
}

int wmain(int argc, LPWSTR argv[])
{
	DWORD dw;

	DWORD dwPID;
	HANDLE hThread;
	HANDLE hProcess;
	PVOID pvBuffer;
	PTHREAD_START_ROUTINE psrLoadLibrary;

	HANDLE hPipe;
	WCHAR wszPipeName[256] = { L'\0' };
	WCHAR wszNetDLLInfo[4096] = { L'\0' };
	SECURITY_ATTRIBUTES sa = {
		.nLength = sizeof(SECURITY_ATTRIBUTES),
		.bInheritHandle = FALSE
	};

	if (argc != 7) {
		wprintf(L"usage: Injector.exe \"Process name\" \"NativeDLL path\" \".Net DLL path\" \".Net Type Name\" \".Net Method Name\" \".Net Method Parameter\"");
		return EXIT_FAILURE;
	}

	wprintf(L"Finding process %s\n", argv[1]);
	hProcess = GetProcessByName(argv[1], &dwPID);
	if (!hProcess) {
		wprintf(L"Process %s not found.\n", argv[1]);
		return EXIT_FAILURE;
	}
	wprintf(L"Found process %s\n", argv[1]);

	wprintf(L"Allocating memory in %s\n", argv[1]);
	pvBuffer = VirtualAllocEx(hProcess, NULL, (wcslen(argv[2]) + 1) * sizeof(WCHAR), MEM_COMMIT, PAGE_READWRITE);
	if (!pvBuffer) {
		wprintf(L"VirtualAllocEx failed. LastErrorCode: %X\n", GetLastError());
		return EXIT_FAILURE;
	}
	wprintf(L"Memory allocated %p in %s\n", pvBuffer,  argv[1]);

	wprintf(L"Writing %s at %p\n", argv[2], pvBuffer);
	if (!WriteProcessMemory(hProcess, pvBuffer, (LPVOID)argv[2], (wcslen(argv[2]) + 1) * sizeof(WCHAR), NULL)) {
		wprintf(L"WriteProcessMemory failed. LastErrorCode: %X\n", GetLastError());
		return EXIT_FAILURE;
	}

	wprintf(L"Finding LoadLibraryW\n");
	psrLoadLibrary = (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(L"Kernel32"), "LoadLibraryW");
	if (!psrLoadLibrary) {
		wprintf(L"GetProcAddress failed. LastErrorCode: %X\n", GetLastError());
		return EXIT_FAILURE;
	}
	wprintf(L"Found LoadLibraryW at %p\n", psrLoadLibrary);

	ConvertStringSecurityDescriptorToSecurityDescriptor(
		L"D:(A;OICI;GRGWGX;;;WD)",
		SDDL_REVISION_1,
		&(sa.lpSecurityDescriptor),
		NULL);

	wsprintf(wszPipeName, L"\\\\.\\pipe\\NetDLL%X", dwPID);
	wprintf(L"Creating named pipe %s\n", wszPipeName);
	hPipe = CreateNamedPipe(wszPipeName, PIPE_ACCESS_OUTBOUND, PIPE_TYPE_BYTE, PIPE_UNLIMITED_INSTANCES, 4096 * sizeof(WCHAR), 4096 * sizeof(WCHAR), 0, &sa);
	if (hPipe == INVALID_HANDLE_VALUE) {
		wprintf(L"CreateNamedPipe failed. LastErrorCode: %X\n", GetLastError());
		return EXIT_FAILURE;
	}

	wprintf(L"Creating remote thread\n");
	hThread = CreateRemoteThread(hProcess, NULL, 0, psrLoadLibrary, pvBuffer, 0, NULL);
	if (!hThread) {
		wprintf(L"CreateRemoteThread failed. LastErrorCode: %X\n", GetLastError());
		return EXIT_FAILURE;
	}
	wprintf(L"Remote thread created %p\n", hThread);

	wsprintf(wszNetDLLInfo, L"%s|%s|%s|%s|", argv[3], argv[4], argv[5], argv[6]);

	wprintf(L"Connecting named pipe\n");
	ConnectNamedPipe(hPipe, NULL);

	wprintf(L"Writing named pipe %s\n", wszNetDLLInfo);
	WriteFile(hPipe, wszNetDLLInfo, sizeof(wszNetDLLInfo), &dw, NULL);

	wprintf(L"Success.\n");
	WaitForSingleObject(hThread, INFINITE);

	CloseHandle(hThread);
	CloseHandle(hProcess);
	CloseHandle(hPipe);

	return 0;
}