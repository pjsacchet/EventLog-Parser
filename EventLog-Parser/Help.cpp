// Handles help information and interface for the user

#include "Help.h"

BOOL PrintHelpCreateTraceSession()
{
	OutputDebugStringW(L"Usage: rundll32 EventLog-Parser.dll,#1 <name of trace session> <GUID string of requested provider>\n");

	return TRUE;
}

BOOL PrintHelpDeleteTraceSession()
{


	return TRUE;
}

BOOL PrintHelpListTraceSessions()
{


	return TRUE;
}