# This file serves as a wrapper around our DLL using ctypes
    # This will improve user experience by taking away the need for the user to employ tools
    # like rundll32 and dbgview, eliminating extra applications and conditions that need to be met for 
    # this tool to work properly

import ctypes 
import enum

# Path to our DLL
    # Assuming we're loading the x64 debug version, but could make that a selection at runtime 
dll_path = ctypes.WinDLL('EventLog-Parser/x64/Debug/EventLog-Parser.dll')

# Command values
class values(enum.Enum):
    EXIT = 0
    START_TRACE = 1
    STOP_TRACE = 2
    LIST_TRACES = 3

# Print simple helper function 
def printHelp() -> str:
    return 'Please choose from the following options: \n' \
    '\t 0) Exit\n' \
    '\t 1) Start new trace session\n' \
    '\t 2) Stop an existing trace session\n' \
    '\t 3) List running trace sessions\n' 

# Function handler for creating new trace sessions; will call into our DLL with provided args
def handleNewTraceSession(trace_name : str, provider_guid:str) -> bool:
    # Grab a pointer to our exported function and define our primitive args
    CreateTraceSession = dll_path.CreateTraceSession
    CreateTraceSession.argtypes = [ctypes.c_wchar_p, ctypes.c_wchar_p]
    CreateTraceSession.restype = [ctypes.c_bool]

    print(f"Attempting to create trace seesion with name {trace_name} ane provider {provider_guid}")

    result = CreateTraceSession(trace_name, provider_guid)
    if (not result):
        print("Failed CreateTraceSession!")
        return False
    else:
        print("Successfully created trace session")
        return True

# Functino call for stopping/deleting existing trace sessions; will call into our DLL with provided args
def handleStopTraceSession():

    return

def handleListTraceSessions():


    return

def main():
    choice = -1
    while (True):
        print(printHelp())
        choice = int(input("> "))
        match choice:
            case values.EXIT.value:
                print("Exiting program...")
                return
            case values.START_TRACE.value:
                print("Please input session name: ")
                trace_name = input("\t> ")
                print("Please input GUID for provider: ")
                provider_guid = input("\t> ")
                if (not handleNewTraceSession(trace_name, provider_guid)):
                    print("Failed creating new trace session")
                else:
                    print(f"Created new trace session {trace_name} with provider GUID {provider_guid}")

                

            case _:
                print("ERROR; invalid selection; try again")


if __name__ == '__main__':
    main()