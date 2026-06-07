// Main entry point for event log interfacing via Event Tracing for Windows (ETW)

#include "EventLog-Parser.h"
#include "Help.h"

// Callback for our consumer events
VOID WINAPI EventCallback(EVENT_RECORD* event)
{
	std::wstring err, info;
	const auto& header = event->EventHeader;
	FILETIME fileTime = { 0 };
	SYSTEMTIME systemTime = { 0 };
	ULONG bufferSize = 0;
	std::unique_ptr<BYTE[]> buffer;
	TRACE_EVENT_INFO* eventInfo;

	if (!FileTimeToLocalFileTime((FILETIME*)&header.TimeStamp, &fileTime))
	{
		err = L"EventLog-Parser::EventCallback - ERROR; Failed FileTimeToLocalFileTime error ";
		err += std::to_wstring(GetLastError());
		OutputDebugStringW(err.c_str());
		err.clear();
		return;
	}

	if (!FileTimeToSystemTime(&fileTime, &systemTime))
	{
		err = L"EventLog-Parser::EventCallback - ERROR; Failed FileTimeToSystemTime error ";
		err += std::to_wstring(GetLastError());
		OutputDebugStringW(err.c_str());
		err.clear();
		return;
	}

	info = L"ProcessID: " + std::to_wstring(header.ProcessId);
	info += L" ThreadID " + std::to_wstring(header.ThreadId);

	OutputDebugStringW(info.c_str());
	info.clear();

	// Need to employ the use of the Trace Data Helper (TDH) API to get info on our event
	TdhGetEventInformation(event, 0, NULL, NULL, &bufferSize);
	buffer = std::make_unique<BYTE[]>(bufferSize);
	eventInfo = (TRACE_EVENT_INFO*)(buffer.get());
	TdhGetEventInformation(event, 0, NULL, eventInfo, &bufferSize);

	if (eventInfo->EventNameOffset)
	{
		auto name = (PCWSTR)((PBYTE)eventInfo + eventInfo->EventNameOffset);
		info += L"EventName: " + (*name);
	}

	OutputDebugStringW(info.c_str());
	info.clear();

	return;
}

// User will call into ordinal 1 to create a new trace session 
	// Used for direct interaction via rundll32
BOOL CreateTraceSessionCmd(HWND hwnd, HINSTANCE hinst, LPWSTR cmdLine, int cmdShow)
{
	INT32 numArgs = 0;
	INT64 logfilenamelen = 0, loggernamelen = 0;
	EventTraceProps data = { 0 };
	TRACEHANDLE hTrace = { 0 }, hParse = { 0 };
	GUID providerGUID = { 0 };
	EVENT_TRACE_LOGFILE etl = { 0 };
	std::wstring sessionName, err, info;

	// Parse user provided input
		// Trace session name
		// GUID for provider we want to send us input from 
	LPWSTR* args = CommandLineToArgvW(GetCommandLineW(), &numArgs);

	if (args && numArgs == 4)
	{
		sessionName = args[2];	

		// What provider GUID does the user want to provide us events with
		if (CLSIDFromString(args[3], &providerGUID) != ERROR_SUCCESS)
		{
			err = L"EventLog-Parser::CreateTraceSessionCmd - ERROR; Failed CLSIDFromString error ";
			err += std::to_wstring(GetLastError());
			OutputDebugStringW(err.c_str());
			err.clear();
			return FALSE;
		}

		LocalFree(args);
	}

	else
	{
		OutputDebugStringW(L"EventLog-Parser::CreateTraceSessionCmd - ERROR; Incorrect number of args passed!\n");

		if (!PrintHelpCreateTraceSession())
		{
			OutputDebugStringW(L"EventLog-Parser::CreateTraceSessionCmd - ERROR; Failed PrintHelpCreateTraceSession!\n");
		}

		return FALSE;
	}

	// Now with that configure our trace session - these are what is required at minimum for a trace session
	data.props.Wnode.BufferSize = sizeof(data); // number of bytes for the event tracing session properties (plus session name string and log file name)
	data.props.Wnode.Guid = GUID_NULL; // allow ETW to generate a unique GUID for us to use
	data.props.Wnode.ClientContext = 1; // query performance counter (high resolution time stamp)
	data.props.Wnode.Flags = WNODE_FLAG_TRACED_GUID; // must contain this

	data.props.LogFileMode = EVENT_TRACE_REAL_TIME_MODE; // don't write output to file

	// Start our session
	if (StartTraceW(&hTrace, sessionName.c_str(), &data.props) != ERROR_SUCCESS)
	{
		err = L"EventLog-Parser::CreateTraceSessionCmd - ERROR; Failed StartTraceW error ";
		err += std::to_wstring(GetLastError());
		OutputDebugStringW(err.c_str());
		err.clear();
		return FALSE;
	}

	info = L"EventLog-Parser::CreateTraceSessionCmd - INFO; Trace session started: ";
	info += sessionName.c_str();
	OutputDebugStringW(info.c_str());
	info.clear();

	// Next add our provider so we actually collect stuff 
	if (EnableTraceEx2(hTrace, &providerGUID, EVENT_CONTROL_CODE_ENABLE_PROVIDER, TRACE_LEVEL_VERBOSE, 0, 0, 0, NULL) != ERROR_SUCCESS)
	{
		err = L"EventLog-Parser::CreateTraceSessionCmd - ERROR; Failed EnableTraceEx2 error ";
		err += std::to_wstring(GetLastError());
		OutputDebugStringW(err.c_str());
		err.clear();
		return FALSE;
	}

	// Create a consumer so we comsume stuff
	etl.LoggerName = (PWSTR)sessionName.c_str();
	etl.ProcessTraceMode = PROCESS_TRACE_MODE_REAL_TIME | PROCESS_TRACE_MODE_EVENT_RECORD;
	etl.EventRecordCallback = EventCallback;

	hParse = OpenTrace(&etl);
	if (hParse == INVALID_PROCESSTRACE_HANDLE)
	{
		err = L"EventLog-Parser::CreateTraceSessionCmd - ERROR; Failed OpenTrace error ";
		err += std::to_wstring(GetLastError());
		OutputDebugStringW(err.c_str());
		err.clear();
		return FALSE;
	}

	OutputDebugStringW(L"EventLog-Parser::CreateTraceSessionCmd - INFO; Starting event processing...\n");

	// Start processing events
	if (ProcessTrace(&hParse, 1, NULL, NULL) != ERROR_SUCCESS)
	{
		err = L"EventLog-Parser::CreateTraceSessionCmd - ERROR; Failed ProcessTrace error ";
		err += std::to_wstring(GetLastError());
		OutputDebugStringW(err.c_str());
		err.clear();
		return FALSE;
	}


	return TRUE;
}

// Used for calls from our python usermode code 
	// Only need the desired session name and provider GUID for our session thats being created
BOOL CreateTraceSessionPy(__in WCHAR* sessionName, __in WCHAR* providerGuid)
{
	INT64 logfilenamelen = 0, loggernamelen = 0;
	EventTraceProps data = { 0 };
	TRACEHANDLE hTrace = { 0 }, hParse = { 0 };
	GUID providerGUID = { 0 };
	EVENT_TRACE_LOGFILE etl = { 0 };
	std::wstring err, info;

	// TODO: make a easily callable print function that takes a number of varied args and a string 
	info = L"EventLog-Parser::CreateTraceSessionPy - INFO; User passed args for session name/provider GUID";
	info += sessionName;
	info += providerGuid;
	OutputDebugStringW(info.c_str());
	info.clear();

	// Now with that configure our trace session - these are what is required at minimum for a trace session
	data.props.Wnode.BufferSize = sizeof(data); // number of bytes for the event tracing session properties (plus session name string and log file name)
	data.props.Wnode.Guid = GUID_NULL; // allow ETW to generate a unique GUID for us to use
	data.props.Wnode.ClientContext = 1; // query performance counter (high resolution time stamp)
	data.props.Wnode.Flags = WNODE_FLAG_TRACED_GUID; // must contain this

	data.props.LogFileMode = EVENT_TRACE_REAL_TIME_MODE; // don't write output to file

	// Start our session
	if (StartTraceW(&hTrace, sessionName, &data.props) != ERROR_SUCCESS)
	{
		err = L"EventLog-Parser::CreateTraceSessionPy - ERROR; Failed StartTraceW error ";
		err += std::to_wstring(GetLastError());
		OutputDebugStringW(err.c_str());
		err.clear();
		return FALSE;
	}

	info = L"EventLog-Parser::CreateTraceSessionPy - INFO; Trace session started: ";
	info += sessionName; // TODO: can I do this 
	OutputDebugStringW(info.c_str());
	info.clear();

	// Next add our provider so we actually collect stuff 
	if (EnableTraceEx2(hTrace, &providerGUID, EVENT_CONTROL_CODE_ENABLE_PROVIDER, TRACE_LEVEL_VERBOSE, 0, 0, 0, NULL) != ERROR_SUCCESS)
	{
		err = L"EventLog-Parser::CreateTraceSessionPY - ERROR; Failed EnableTraceEx2 error ";
		err += std::to_wstring(GetLastError());
		OutputDebugStringW(err.c_str());
		err.clear();
		return FALSE;
	}

	// Create a consumer so we comsume stuff
	etl.LoggerName = sessionName;
	etl.ProcessTraceMode = PROCESS_TRACE_MODE_REAL_TIME | PROCESS_TRACE_MODE_EVENT_RECORD;
	etl.EventRecordCallback = EventCallback;

	hParse = OpenTrace(&etl);
	if (hParse == INVALID_PROCESSTRACE_HANDLE)
	{
		err = L"EventLog-Parser::CreateTraceSessionPy - ERROR; Failed OpenTrace error ";
		err += std::to_wstring(GetLastError());
		OutputDebugStringW(err.c_str());
		err.clear();
		return FALSE;
	}

	OutputDebugStringW(L"EventLog-Parser::CreateTraceSessionPy - INFO; Starting event processing...\n");

	// Start processing events
	if (ProcessTrace(&hParse, 1, NULL, NULL) != ERROR_SUCCESS)
	{
		err = L"EventLog-Parser::CreateTraceSessionPy - ERROR; Failed ProcessTrace error ";
		err += std::to_wstring(GetLastError());
		OutputDebugStringW(err.c_str());
		err.clear();
		return FALSE;
	}


	return TRUE;
}

// User will call into ordinal 2 to stop an active trace session
	// Used for direct interaction via rundll32
BOOL DeleteTraceSessionCmd(HWND hwnd, HINSTANCE hinst, LPWSTR cmdLine, int cmdShow)
{
	INT32 numArgs = 0;
	std::wstring sessionName, info, err;
	EventTraceProps data = { 0 };

	// Parse user provided input
		// Trace session name for what we want to close 
	LPWSTR* args = CommandLineToArgvW(GetCommandLineW(), &numArgs);

	if (args && numArgs == 3)
	{
		sessionName = args[2];

		LocalFree(args);
	}

	else
	{
		OutputDebugStringW(L"EventLog-Parser::DeleteTraceSessionCmd - ERROR; Incorrect number of args passed!\n");

		if (!PrintHelpDeleteTraceSession())
		{
			OutputDebugStringW(L"EventLog-Parser::DeleteTraceSessionCmd - ERROR; Failed PrintHelpCreateTraceSession!\n");
		}

		return FALSE;
	}

	data.props.Wnode.BufferSize = sizeof(data);
	data.props.Wnode.Flags = WNODE_FLAG_TRACED_GUID;
	data.props.LoggerNameOffset = offsetof(EventTraceProps, loggerName);
	
	if (ControlTraceW(NULL, sessionName.c_str(), &data.props, EVENT_TRACE_CONTROL_STOP) != ERROR_SUCCESS)
	{
		err = L"EventLog-Parser::DeleteTraceSessionCmd - ERROR; Failed ControlTraceW error ";
		err += std::to_wstring(GetLastError());
		OutputDebugStringW(err.c_str());
		err.clear();
		return FALSE;
	}

	OutputDebugStringW(L"EventLog-Parser::DeleteTraceSessionCmd - INFO; Successfully closed trace session\n");

	return TRUE;
}

// Used for calls from our python usermode code 
	// Only need the session name we want to delete
BOOL DeleteTraceSessionPy(__in WCHAR* sessionName)
{
	std::wstring info, err;
	EventTraceProps data = { 0 };

	data.props.Wnode.BufferSize = sizeof(data);
	data.props.Wnode.Flags = WNODE_FLAG_TRACED_GUID;
	data.props.LoggerNameOffset = offsetof(EventTraceProps, loggerName);

	if (ControlTraceW(NULL, sessionName, &data.props, EVENT_TRACE_CONTROL_STOP) != ERROR_SUCCESS)
	{
		err = L"EventLog-Parser::DeleteTraceSessionPy - ERROR; Failed ControlTraceW error ";
		err += std::to_wstring(GetLastError());
		OutputDebugStringW(err.c_str());
		err.clear();
		return FALSE;
	}

	OutputDebugStringW(L"EventLog-Parser::DeleteTraceSessionPy - INFO; Successfully closed trace session\n");

	return TRUE;
}

// User will call into ordinal 3 to list all active trace sessions
	// Used for direct interaction via rundll32
BOOL ListTraceSessionsCmd(HWND hwnd, HINSTANCE hinst, LPWSTR cmdLine, int cmdShow)
{
	DWORD status;
	std::wstring err, info;
	ULONG count = 64; // max of 64 sessions
	INT64 propSize = sizeof(EVENT_TRACE_PROPERTIES) + (1024 * sizeof(WCHAR) + (1024 * sizeof(WCHAR))); // session name and log path lengths are both max 1024 bytes
	std::vector<BYTE>buffer;
	std::vector<EVENT_TRACE_PROPERTIES*> sessions;

	do
	{
		sessions.resize(count);
		buffer.resize(propSize * count);

		for (UINT64 i = 0; i < count; i++)
		{
			sessions[i] = (EVENT_TRACE_PROPERTIES*)&buffer[i * propSize];
			sessions[i]->Wnode.BufferSize = propSize;
			sessions[i]->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);
			sessions[i]->LogFileNameOffset = sizeof(EVENT_TRACE_PROPERTIES) + (1024 * sizeof(CHAR));
		}

		status = QueryAllTracesW(&sessions[0], count, &count);
	} while (status == ERROR_MORE_DATA);

	if (status != ERROR_SUCCESS)
	{
		err = L"EventLog-Parser::ListTraceSessionsCmd - ERROR; Failed QueryAllTracesW error ";
		err += std::to_wstring(GetLastError());
		OutputDebugStringW(err.c_str());
		err.clear();
		return FALSE;
	}

	else
	{
		info = L"Session count: " + std::to_wstring(count);
		OutputDebugStringW(info.c_str());
		info.clear();

		for (UINT64 i = 0; i < count; i++)
		{
			auto name = (PCWSTR)((LPCBYTE)sessions[i] + sessions[i]->LoggerNameOffset);
			info = L"Session name: " + std::wstring(name); 

			OutputDebugStringW(info.c_str());
			info.clear();
		}
	}

	return TRUE;
}

// Used for calls from our python usermode code 
	// Allocate and format the output session string of all trace sessions on this machine 
BOOL ListTraceSessionsPy(__inout UINT32* sessionCount, WCHAR*** sessionNames)
{
	DWORD status;
	std::wstring err, info;
	ULONG count = 64; // max of 64 sessions
	INT64 propSize = sizeof(EVENT_TRACE_PROPERTIES) + (1024 * sizeof(WCHAR) + (1024 * sizeof(WCHAR))); // session name and log path lengths are both max 1024 bytes
	std::vector<BYTE>buffer;
	std::vector<EVENT_TRACE_PROPERTIES*> sessions;

	do
	{
		sessions.resize(count);
		buffer.resize(propSize * count);

		for (UINT64 i = 0; i < count; i++)
		{
			sessions[i] = (EVENT_TRACE_PROPERTIES*)&buffer[i * propSize];
			sessions[i]->Wnode.BufferSize = propSize;
			sessions[i]->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);
			sessions[i]->LogFileNameOffset = sizeof(EVENT_TRACE_PROPERTIES) + (1024 * sizeof(CHAR));
		}

		status = QueryAllTracesW(&sessions[0], count, &count);
	} while (status == ERROR_MORE_DATA);

	if (status != ERROR_SUCCESS)
	{
		err = L"EventLog-Parser::ListTraceSessionsPy - ERROR; Failed QueryAllTracesW error ";
		err += std::to_wstring(GetLastError());
		OutputDebugStringW(err.c_str());
		err.clear();
		return FALSE;
	}

	else
	{
		info = L"Session count: " + std::to_wstring(count);
		OutputDebugStringW(info.c_str());
		info.clear();

		// Store for output, and allocate array accordingly 
		*sessionCount = count;

		*sessionNames = (WCHAR**)malloc(sizeof(WCHAR*) * count);
		if (*sessionNames == NULL)
		{
			err = L"EventLog-Parser:: ListTraceSessionsPy - ERROR; OOM!\N";
			OutputDebugStringW(err.c_str());
			return FALSE;
		}

		for (UINT64 i = 0; i < count; i++)
		{
			auto name = (PCWSTR)((LPCBYTE)sessions[i] + sessions[i]->LoggerNameOffset);
			info = L"Session name: " + std::wstring(name);

			OutputDebugStringW(info.c_str());
			info.clear();

			(*sessionNames)[i] = (WCHAR*)malloc(sizeof(WCHAR) * std::wstring(name).length());
			if ((*sessionNames)[i] == NULL)
			{
				err = L"EventLog-Parser:: ListTraceSessionsPy - ERROR; OOM!\N";
				OutputDebugStringW(err.c_str());
				return FALSE;
			}

			memcpy((*sessionNames)[i], std::wstring(name).c_str(), std::wstring(name).length());
		}
	}

	// TODO: goto statement here to cleanup memory before we bail so we don't leak 

	return TRUE;
}