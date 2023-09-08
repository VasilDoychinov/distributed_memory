// _dm_task.h: private header for dm_task.cpp
//


#ifndef _DEFS__DM_TASK_
#define _DEFS__DM_TASK_

#include <atomic>

#include "tcp_sockets.h"
#include "udp_sockets.h"

#include "dm_task.h"
#include "dm_communication.h"


#include "Logger_decl.hpp"
#include "Logger_helpers.hpp"

/*
struct DM_data_ {
   // DM_enquiry   _enq {DM_enquiry::DM_stop} ; skip it to achieve lock-free status 64-bit architecture
   unsigned int _key {0} ;      // to keep it simple
   int          _val {0} ;      // ...
}; */

struct DM_service_ {
   static constexpr unsigned int  _NCONN_TO_LISTEN = DM_TASK_NUMBER_OF_NODES * 4 ; // oob

   std::atomic<DM_signal *>   _comm {nullptr} ;

   unsigned int            _nodes_number {0} ;

   SOCKET           _tcp_ms {INVALID_SOCKET} ;   // TCP master socket: local(server)
   SOCKET           _udp_ms {INVALID_SOCKET} ;   // UDP ...
   SOCKET           _udp_ps {INVALID_SOCKET} ;   // UDP connection to successor node
   struct addrinfo* _next_node {nullptr} ;       // to the address of successor node

   DM_service_(unsigned int nn) ;
   ~DM_service_() ;

   void clear_signal() { Log_to(0, "> clearing a signal") ; _comm = nullptr ; }
} ;


// void _task_dm_service(std::promise<)
_DEFS__DM_TASK_
#endif
