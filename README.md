# EventLog-Parser
Tool meant to help blue-teamers better understand the inner-workings of the Windows Event Log via Event Tracing for Windows (ETW).

ETW is a powerful kernel-level utility that offers increidbly granularity to listening to events from different applications + services.

Most anti-virus software products heavily employ ETW to identify malicious software and activity; understanding how one could utilize this at a programmatic level would give any blue-teamer a leg-up on the adversary. 

## Setup & Execution
I've included the .sln file for those who may wish to build on their own machines (I am using Visual Studio 2022 currently). I've also included the built binary found under ...\EventLog-Parser\EventLog-Parser\x64\Debug\EventLog-Parser.dll

To run, the user can specify one of three exports this DLL offers and run it via rundll32 EventLog-Parser.dll,<ordinal number> [args...]:

1. Create a trace session
   `> rundll32 EventTrace-Parser.dll,#1 <session name> <GUID for provider>`
2. Stop a trace session
   `> rundll32 EventTrace-Parser.dll,#2 <session name>`
3. List all active trace sessions
   `> rundll32 EventTrace-Parser.dll,#3`

## Useful commands
In order to help troubleshoot and explore, the user may make use fo the following commands to help aid their understanding and troubleshoot via built-in Windows utilities:

### Query for all active trace sessions:
`> logman query -ets`

### Stop active trace session (equivalent to ordinal #2):
`> logman stop "session name" -ets`

### Query all providers:
`> logman query providers`

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