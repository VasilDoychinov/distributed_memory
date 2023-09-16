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
#   Presented as a Qt CMake project: tested with gcc ...
#
#
#   Basic idea: 
#
#   NB:
#       - the service is to be started and then run as a daemon 
#       - enquiries to be forwarded to the service via thread communication mechanics
#         & the result return to the Sender
#       - Just a Prototype: 
#          - only one enquiry at a time can be generated to a Node (throught the Test program currently)
#          - error handling is just indicated
#          - all messages to the User are to STDOUT and Logger is used (represented by Log_to(0, ...))
#       - topology: ring topology wth hard-stated # of tokens with a common IP address
#       - tokens communicate through UDP and TCP protocols, ports are set at launch
#         NB: Socket system calls to be compatible with Winsock and Berkley sockets and portable to
#             a tool chain supporting those
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
#       - dm_localDB[cpp/h]: simulates a local DB through some simple queries
#       - src/communication: dm_communication.[h/cpp]: example/prototype of Signals to be used
#       - src/communication: dm_nodes_protocol.h: example/prototype of Potocol to be used
#       - src/communication: dm_nodes_communication.cpp: communication betweet nodes (TCP, IDP sockets)
#       - src/misc: conversions.[cpp/h]: some IPv4:port conversions and calcs
#

