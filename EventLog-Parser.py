# This file serves as a wrapper around our DLL using ctypes
    # This will improve user experience by taking away the need for the user to employ tools
    # like rundll32 and dbgview, eliminating extra applications and conditions that need to be met for 
    # this tool to work properly

import ctypes 
import enum
import sys 
import os 

# Path to our DLL
    # Assuming we're loading the x64 debug version, but could make that a selection at runtime 
dll_path = None

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
    CreateTraceSession = dll_path.CreateTraceSessionPy
    CreateTraceSession.argtypes = [ctypes.c_wchar_p, ctypes.c_wchar_p]
    CreateTraceSession.restype = ctypes.c_bool
    trace_name_c = ctypes.c_wchar_p(trace_name)
    provider_guid_c = ctypes.c_wchar_p(provider_guid)

    print(f"Attempting to create trace seesion with name {trace_name} and provider {provider_guid}")

    result = CreateTraceSession(trace_name_c, provider_guid_c)
    if (not result):
        print("Failed CreateTraceSession!")
        return False
    else:
        print("Successfully created trace session")
        return True

# Function call for stopping/deleting existing trace sessions; will call into our DLL with provided args
def handleStopTraceSession(trace_name : str) -> bool:
    # Grab a pointer to our exported function and define our primitive args
    DeleteTraceSession = dll_path.DeleteTraceSessionPy
    DeleteTraceSession.argtypes = [ctypes.c_wchar_p, ctypes.c_wchar_p]
    DeleteTraceSession.restype = ctypes.c_bool
    trace_name_c = ctypes.c_wchar_p(trace_name)

    print(f"Attempting to stop trace seesion with name {trace_name}")

    result = DeleteTraceSession(trace_name_c)
    if (not result):
        print("Failed DeleteTraceSession!")
        return False
    else:
        print("Successfully deleted trace session")
        return True

# Function call for listing all active trace sessions
def handleListTraceSessions() -> bool:
    # Grab a pointer to our exported function and define our primitive args
    ListTraceSessions = dll_path.ListTraceSessionsPy
    ListTraceSessions.argtypes = [ctypes.POINTER(ctypes.c_uint32), ctypes.POINTER(ctypes.POINTER(ctypes.c_wchar_p))]
    ListTraceSessions.restype = ctypes.c_bool
    num_traces = ctypes.c_uint32(0)
    trace_names = ctypes.POINTER(ctypes.c_wchar_p)()

    print(f"Attempting to list all active trace sessions")

    result = ListTraceSessions(ctypes.byref(num_traces), ctypes.byref(trace_names))
    if (not result):
        print("Failed ListTraceSessions!")
        return False
    elif (num_traces.value == 0):
        print("ERROR; supposedly found 0 traces!")
    else:
        print(f"Found {num_traces.value} traces")
        count = 0
        while (count <= num_traces.value):
            #print(f"{trace_names[count]}")
            try:
                # Re-encode as UTF-16 allowing surrogates, then decode back to valid Unicode
                fixed_string = trace_names[count].encode('utf-16', 'surrogatepass').decode('utf-16')
                print(fixed_string)
            except UnicodeEncodeError:
                # If it fails, the data might be genuinely corrupted
                print("Data contains invalid surrogate sequences.")
            count += 1
        return True

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
            case values.STOP_TRACE.value:
                print("Please input session name: ")
                trace_name = input("\t> ")
                if (not handleStopTraceSession(trace_name)):
                    print("Failed to stop trace session")
                else:
                    print(f"Stopped trace session {trace_name}")
            case values.LIST_TRACES.value:
                if (not handleListTraceSessions()):
                    print("Failed to list trace sessions")
                else:
                    print("Successfully listed trace sessions")
            case _:
                print("ERROR; invalid selection; try again")


if __name__ == '__main__':
    if (len(sys.argv) != 2):
        print("ERROR; Need to provide path to DLL upon execution")
        exit()
    else:
        if (os.path.exists(sys.argv[1])):
            os.add_dll_directory(os.getcwd())
            dll_path = ctypes.WinDLL(sys.argv[1])
            main()