// dllmain.cpp : Defines the entry point for the DLL application.
#include <Windows.h>
#include <sddl.h>
#include <metahost.h>
#include "mscoree.h"

#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "mscoree.lib")

ICLRRuntimeHost* GetCLRRuntimeHost()
{
	ICLRMetaHost* clrMetaHost = NULL;
	ICLRRuntimeInfo* clrRuntimeInfo = NULL;
	ICLRRuntimeHost* clrRuntimeHost = NULL;

	if (CLRCreateInstance(&CLSID_CLRMetaHost, &IID_ICLRMetaHost, &clrMetaHost) != S_OK) {
		MessageBox(NULL, L"Cannot obtain clr meta host.", L"Error", MB_OK | MB_ICONERROR);
		goto exit;
	}

	if (clrMetaHost->lpVtbl->GetRuntime(clrMetaHost, L"v4.0.30319", &IID_ICLRRuntimeInfo, &clrRuntimeInfo) != S_OK) {
		MessageBox(NULL, L"Cannot obtain clr runtime info.", L"Error", MB_OK | MB_ICONERROR);
		goto exit;
	}

	if (clrRuntimeInfo->lpVtbl->GetInterface(clrRuntimeInfo, &CLSID_CLRRuntimeHost, &IID_ICLRRuntimeHost, &clrRuntimeHost) != S_OK) {
		MessageBox(NULL, L"Cannot obtain clr runtime host.", L"Error", MB_OK | MB_ICONERROR);
		goto exit;
	}

exit:
	if (clrRuntimeInfo) {
		clrRuntimeInfo->lpVtbl->Release(clrRuntimeInfo);
	}
	if (clrMetaHost) {
		clrMetaHost->lpVtbl->Release(clrMetaHost);
	}

	return clrRuntimeHost;
}

BOOL ParseNetDLLInfo(PWSTR pwszNetDLLInfo, PWSTR* ppwszAssemblyPath, PWSTR* ppwszAssemblyTypeName, PWSTR* ppwszAssemblyMethodName, PWSTR* ppwszAssemblyArgument)
{
	PWSTR p = pwszNetDLLInfo;

	*ppwszAssemblyPath = p;
	p = wcsstr(p, L"|");
	if (!p) {
		return FALSE;
	}
	*p = L'\0';
	++p;

	*ppwszAssemblyTypeName = p;
	p = wcsstr(p, L"|");
	if (!p) {
		return FALSE;
	}
	*p = L'\0';
	++p;

	*ppwszAssemblyMethodName = p;
	p = wcsstr(p, L"|");
	if (!p) {
		return FALSE;
	}
	*p = L'\0';
	++p;

	*ppwszAssemblyArgument = p;
	p = wcsstr(p, L"|");
	if (!p) {
		return FALSE;
	}
	*p = L'\0';
	++p;

	return TRUE;
}

BOOL ReadNetDLLInfo(PWSTR* ppwszAssemblyPath, PWSTR* ppwszAssemblyTypeName, PWSTR* ppwszAssemblyMethodName, PWSTR* ppwszAssemblyArgument)
{
	DWORD dw;
	BOOL bRet  = FALSE;
	HANDLE hPipe;
	WCHAR wszPipeName[256] = { L'\0' };
	static WCHAR wszNetDLLInfo[4096] = { L'\0' };
	SECURITY_ATTRIBUTES sa = {
		.nLength = sizeof(SECURITY_ATTRIBUTES),
		.bInheritHandle = FALSE
	};

	wsprintf(wszPipeName, L"\\\\.\\pipe\\NetDLL%X", GetCurrentProcessId());

	ConvertStringSecurityDescriptorToSecurityDescriptor(
		L"D:(A;OICI;GRGWGX;;;WD)",
		SDDL_REVISION_1,
		&(sa.lpSecurityDescriptor),
		NULL);
	hPipe = CreateFile(wszPipeName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, &sa, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hPipe == INVALID_HANDLE_VALUE) {
		MessageBox(NULL, L"Cannot open named pipe.", L"Error", MB_OK | MB_ICONERROR);
		goto exit;
	}

	if (!ReadFile(hPipe, wszNetDLLInfo, sizeof(wszNetDLLInfo), &dw, NULL)) {
		MessageBox(NULL, L"Cannot read from named pipe.", L"Error", MB_OK | MB_ICONERROR);
		goto exit;
	}

	if (!ParseNetDLLInfo(wszNetDLLInfo, ppwszAssemblyPath, ppwszAssemblyTypeName, ppwszAssemblyMethodName, ppwszAssemblyArgument)) {
		MessageBox(NULL, L"Cannot parse net dll info string.", L"Error", MB_OK | MB_ICONERROR);
		goto exit;
	}

	bRet = TRUE;

exit:
	if (hPipe != INVALID_HANDLE_VALUE) {
		CloseHandle(hPipe);
	}

	return bRet;
}

void ExecuteNetDLL()
{
	PWSTR pwszAssemblyPath;
	PWSTR pwszAssemblyTypeName;
	PWSTR pwszAssemblyMethodName;
	PWSTR pwszAssemblyArgument;

	DWORD dwReturn;
	ICLRRuntimeHost* clrRuntimeHost = NULL;

	if (!ReadNetDLLInfo(&pwszAssemblyPath, &pwszAssemblyTypeName, &pwszAssemblyMethodName, &pwszAssemblyArgument)) {
		goto exit;
	}

	clrRuntimeHost = GetCLRRuntimeHost();
	if (!clrRuntimeHost) {
		goto exit;
	}
	
	if (clrRuntimeHost->lpVtbl->ExecuteInDefaultAppDomain(clrRuntimeHost, pwszAssemblyPath, pwszAssemblyTypeName, pwszAssemblyMethodName, pwszAssemblyArgument, &dwReturn) != S_OK) {
		MessageBox(NULL, L"ExecuteInDefaultAppDomain error.", L"Error", MB_OK | MB_ICONERROR);
		goto exit;
	}

exit:
	if (clrRuntimeHost) {
		clrRuntimeHost->lpVtbl->Release(clrRuntimeHost);
	}
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  dwReason,
	LPVOID lpReserved
)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		ExecuteNetDLL();
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

