# EventLog-Parser
Tool meant to help blue-teamers better understand the inner-workings of the Windows Event Log via Event Tracing for Windows (ETW).

ETW is a powerful kernel-level utility that offers increidbly granularity to listening to events from different applications + services.

Most anti-virus software products heavily employ ETW to identify malicious software and activity; understanding how one could utilize this at a programmatic level would give any blue-teamer a leg-up on the adversary. 

## Setup & Execution
*Please note: user must be running with admin privs to use this tool*

I've included the .sln file for those who may wish to build on their own machines (I am using Visual Studio 2022 currently). I've also included the built binary found under ...\EventLog-Parser\EventLog-Parser\x64\Debug\EventLog-Parser.dll

To run, the user can specify one of three exports this DLL offers and run it via rundll32 EventLog-Parser.dll,<ordinal number> [args...]. For best experience, it is recommended the user run this tool with Administrator privileges, in addition to DbgView to view all printed output to console:

### Create a trace session (ordinal #1)
   `> rundll32 EventLog-Parser.dll,#1 <session name> <GUID for provider>`
### Stop a trace session (ordinal #2)
   `> rundll32 EventLog-Parser.dll,#2 <session name>`
### List all active trace sessions (ordinal #3)
   `> rundll32 EventLog-Parser.dll,#3`

## Useful commands
In order to help troubleshoot and explore, the user may make use fo the following commands to help aid their understanding and troubleshoot via built-in Windows utilities:

### Query for all active trace sessions:
`> logman query -ets`

### Stop active trace session (equivalent to ordinal #2):
`> logman stop "session name" -ets`

### Query all providers:
`> logman query providers`

## Useful tools
Recommended tools for troubleshooting:
- DbgView (https://learn.microsoft.com/en-us/sysinternals/downloads/debugview)
  - *All output from tool is printed to debug so this tool is needed for troubleshooting!*
- ProcessHacker (https://github.com/PKRoma/ProcessHacker)
  - Easy tool to identify loaded DLLs in case a trace session goes rogue...

## Helpful ETW Providers
If you are stuck trying to choose from all the different providers available on Windows to listen to, here are a few specific ones for blue-teamers:
- Microsoft-Windows-Kernel-Process (Tracks every process creation, termination, and image (DLL/executable) load and unload)
  - {22FB2CD6-0E7B-422B-A0C7-2FAD1FD0E716}
- Microsoft-Windows-Kernel-File (Monitors all file system operations, I/O requests, and file modifications in real-time)
  - {EDD08927-9CC4-4E65-B970-C2560FB5C289}
- Microsoft-Windows-Kernel-Network (Logs deep packet metadata, connection tracking, and socket events)
  - {7DD42A49-5329-4832-8DFD-43D979153A88}
- Microsoft-Windows-Kernel-Registry (Outputs every key creation, deletion, and value modification across the Windows Registry)
  - {70EB4F03-C1DE-4F73-A051-33D13D5413BD}
- Microsoft-Windows-Threat-Intelligence (A specialized provider that captures privileged OS operations, such as attempts to write to another process's memory space - usually employed by anti-virus products)
  - {F4E1897C-BB5D-5668-F1D8-040F4D8DD344}

## References:
- https://learn.microsoft.com/en-us/windows/win32/etw/event-tracing-sessions?source=recommendations
- https://learn.microsoft.com/en-us/windows/win32/etw/consuming-events
- https://learn.microsoft.com/en-us/windows/win32/api/evntrace/nf-evntrace-controltracew
- https://learn.microsoft.com/en-us/windows/win32/api/evntrace/nf-evntrace-queryalltracesw
- https://learn.microsoft.com/en-us/windows/win32/api/evntrace/nf-evntrace-processtrace
- https://learn.microsoft.com/en-us/windows/win32/api/evntrace/nf-evntrace-opentracew
- https://trainsec.net/library/windows-internals/capture-etw-events-with-c-part-1/
- https://trainsec.net/library/windows-internals/capture-etw-events-with-c-part-2/
- https://tokmakov.me/2022/04/25/event-tracing-for-windows-c-example/

## TODO:
- Add Python wrapper to enable easy interaction via command line by user
- Optionally write output to file if user wants that
- Print more information regarding events in our callback 