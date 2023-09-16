// _dm_task.h: private header for dm_task.cpp
//


#ifndef _DEFS__DM_TASK_
#define _DEFS__DM_TASK_

#include <atomic>
#include <stdint.h>

#include "tcp_sockets.h"
#include "udp_sockets.h"

#include "dm_task.h"
#include "communication/dm_communication.h"
#include "communication/dm_nodes_protocol.h"


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
   // std::atomic<bool>          _stop {false} ; // no use with blocking select()

   unsigned int     _nodes_number {0} ;          // # of nodes in the topology
   unsigned int     _node_ind{0} ;               // index of this node
   uint32_t         _ip_addr{} ;                 // IP of this node (here same for all)
   unsigned int     _port{} ;                    // tcp port # of this node

   SOCKET           _tcp_ms {INVALID_SOCKET} ;   // TCP master socket: local(server)
   SOCKET           _udp_ms {INVALID_SOCKET} ;   // UDP ...
   SOCKET           _udp_ps {INVALID_SOCKET} ;   // UDP connection to successor node
   struct addrinfo* _this_node{nullptr} ;        // the address for this node
   struct addrinfo* _next_node {nullptr} ;       // to the address of successor node
   struct sockaddr_storage  _prev_node_addr ;    // store the address of predicessor
   socklen_t                _prev_node_addr_len {sizeof(_prev_node_addr)} ;

   DM_service_(unsigned int nn, unsigned int nind, uint32_t ip, unsigned int port) ;
   ~DM_service_() ;

   void clear_signal() { _comm.store(nullptr, std::memory_order_release) ; /*Log_to(0, "> a signal cleared") ; */ }

   // if the enquiry is to addressed to the local DM (DB, etc.)
   bool is_localDM(DM_enquiry& enq) const& ;

   // @return: pointer to the current enquiry, nullptr - if none set
   DM_enquiry* dm_current_enquiry() ;

   // tasks that run in background to provide communication between nodes of a ring topology

   // udp client (runs in background): relay a message to the next node (ring topology)
   // @return: true - OK, false - otherwise
   void dm_stop_servers() const& ;
   bool dm_udp_client(DM_enquiry& enq) const& ;
   bool dm_udp_client(DM_message_& msg) const& ;
} ;


_DEFS__DM_TASK_
#endif

// eof _dm_task.h
