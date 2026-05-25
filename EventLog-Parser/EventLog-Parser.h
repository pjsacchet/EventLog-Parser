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

BOOL CreateTraceSession(HWND hwnd, HINSTANCE hinst, LPWSTR cmdLine, int cmdShow);

BOOL DeleteTraceSession(HWND hwnd, HINSTANCE hinst, LPWSTR cmdLine, int cmdShow);

BOOL ListTraceSessions(HWND hwnd, HINSTANCE hinst, LPWSTR cmdLine, int cmdShow);

// We need our logger name and log file name in memory following our EVENT_TRACE_PROPERTIES struct
typedef struct _EventTraceProps
{
	EVENT_TRACE_PROPERTIES props;
	WCHAR loggerName[128];
	WCHAR logFileName[1024];
} EventTraceProps, *PEventTraceProps;
