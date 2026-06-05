#pragma once
#pragma comment(lib, "tdh.lib")

#include <Windows.h>
#include <evntrace.h>
#include <evntcons.h>
#include <shellapi.h>
#include <tdh.h>

#include <iostream>
#include <vector>
#include <string>
#include <format>

BOOL CreateTraceSessionCmd(HWND hwnd, HINSTANCE hinst, LPWSTR cmdLine, int cmdShow);
BOOL CreateTraceSessionPy(__in WCHAR* sessionName, __in WCHAR* providerGuid);

BOOL DeleteTraceSessionCmd(HWND hwnd, HINSTANCE hinst, LPWSTR cmdLine, int cmdShow);
BOOL DeleteTraceSessionPy(__in WCHAR* sessionName);

BOOL ListTraceSessionsCmd(HWND hwnd, HINSTANCE hinst, LPWSTR cmdLine, int cmdShow);
BOOL ListTraceSessionsPy(__inout WCHAR* sessionString);

// We need our logger name and log file name in memory following our EVENT_TRACE_PROPERTIES struct
typedef struct _EventTraceProps
{
	EVENT_TRACE_PROPERTIES props;
	WCHAR loggerName[128];
	WCHAR logFileName[1024];
} EventTraceProps, *PEventTraceProps;
