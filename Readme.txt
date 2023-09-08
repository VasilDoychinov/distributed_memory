# Distributed memory provider: simulation
#   basics: count of Servers: N
#   topology:                 Ring - each knows only about the next (initially)
#   based on:                 transport layer: TCP and UDP sockets
#   
#   All source code is with eductional purposes only, no liability whatsoever to be ...
#   
#   tool-chain: standard C++ to be easily convertible to C code
#
#   developed through gcc and CMake
#   
#   Initiated: Sept, 2023
#
#
#   Basic idea: 
#
#   NB:
#       - the service is to be started and run in parallel mode
#       - enquiries to be forwarded to the service via thread communication mechanics
#       - topology: ring topology wth hard-stated # of tokens with a common IP address
#       - tokens communicate through UDP and TCP protocols, ports are set at launch
#       - each node is aware only of: the IP address, own TCP and UDP ports,
#         UDP port of the next token
#       - enquiry memory resources are allocated in the main() thread -> signal to the
#         running in bakground Service is constructed and passeed through atomic<ptr> 
#       - the promise<> contained in the Signal returns the result to the calling (main()) thread
#
#   Files:
#       - test_dm.cpp: testing API; launches the Service, creates enquiries, show Results
#       - src/dm_task.h: the Public header. Holds a simple Enquiry definition as well
#       - src/_dm_task.h: the Private header
#       - src/dm_task.cpp: the implementation
#       - src/dm_communication.[h/cpp]: the simple Signals handling
#

