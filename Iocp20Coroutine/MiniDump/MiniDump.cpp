#include "pch.h"
#include "MiniDump.h"
#include <Windows.h>
#include <DbgHelp.h>
#include <format>
#pragma comment(lib,"Dbghelp.lib")

LONG WINAPI UnhandledExceptionFilter_SpawnDmp(struct _EXCEPTION_POINTERS* ExceptionInfo)
{
	const auto strFileName = std::format("±¿¿£{0}.dmp", std::rand());

	HANDLE   hFile = CreateFileA(strFileName.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		MINIDUMP_EXCEPTION_INFORMATION   ExInfo;

		ExInfo.ThreadId = ::GetCurrentThreadId();
		ExInfo.ExceptionPointers = ExceptionInfo;
		ExInfo.ClientPointers = NULL;

		//   write   the   dump
		BOOL   bOK = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ExInfo, NULL, NULL);
		CloseHandle(hFile);
	}
	::PostThreadMessage(GetCurrentThreadId(), WM_QUIT, 0, 0);
	return EXCEPTION_EXECUTE_HANDLER;
}

void MiniDump::Install(const std::string& strFileName)
{
	SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)UnhandledExceptionFilter_SpawnDmp);

}
