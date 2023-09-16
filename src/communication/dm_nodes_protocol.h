// dm_nodes_protocol.h: defines protocols between nodes
//      - messages send over TCP/UDP sockets as follows
//          - DM_PUT_FWD, DM_GET_FWD: over UDP
//          - DM_WHAT_DATA: over TCP as a reaction to DM_PUT_FWD
//          - DM_PUT_REPLY_DATA: over TCP as an answer to DM_WHAT_DATA
//          - DM_GET_REPLY_DATA: over TCP as a reaction to DM_GET_FWD

#ifndef _DEFS_DM_NODES_PROTOCOL_
#define _DEFS_DM_NODES_PROTOCOL_

#include <stdint.h>
#include <iostream>

#include "../misc/conversions.h"

enum class DM_message_types { DM_NO_MSG,
                              DM_PUT_FWD,        // forwarded request to fix location (over UDP)
                              DM_GET_FWD,        // forwarded ...
                              DM_DEL_FWD,        // ...
                              DM_WHAT_DATA,      // ask for data over TCP to the Originator of DM_PUT_
                              DM_PUT_REPLY_DATA, // reply to DM_WHAT_DATA
                              DM_GET_REPLY_DATA, // reply to DM_GET_FWD (from the Location to Originator
                              DM_DEL_REPLY_DATA, // reply to DM_DEL_FWD (from the Location to Originator
                              DM_STOP_SERVICE    // used as a stop signal; might check as well the sender
                            } ;

struct DM_message_ {
   unsigned int   _pid{(int)DM_message_types::DM_NO_MSG} ;      // DM_message_types::
   int            _k{} ;              // follows DM_enquiry: as an example/simulation of the key data
   int            _d{} ;              // ...
   int            _r{} ;              // the result: -1 - error, 0 - not found, 1 - found

   uint32_t       _ip{} ;             // IPv4 address
   unsigned int   _port{} ;           // port # with a Node

   DM_message_() : _pid{(int)DM_message_types::DM_NO_MSG}, _k{}, _d{}, _ip{}, _port{} {}

   // create a message for: _PUT_FWD, _GET_FWD
   DM_message_(DM_message_types mt, int k, uint32_t ip, unsigned int pp)
              : _pid{(unsigned int)mt}, _k{k}, _ip{ip}, _port{pp} {}
   // create a message for : _REPLY_DATA, _WHAT_DATA
   DM_message_(DM_message_types mt, int k, int data = 0)
              : _pid{(unsigned int)mt}, _k{k}, _d{data} {}

   std::string get_ip_addr() const& { return get_abcd_ip_format(_ip) ; }
   std::string get_port() const&    { return std::to_string(_port) ; }

   friend std::ostream& operator<< (std::ostream& os, const DM_message_& m) {
       std::string   mt {} ; std::string host_port {} ;
       switch (m._pid)   {
          case (unsigned int)DM_message_types::DM_PUT_FWD: mt = "_PUT_FWD" ;
              host_port = get_abcd_ip_format(m._ip) + ':' + std::to_string(m._port) ; break ;
          case (unsigned int)DM_message_types::DM_GET_FWD: mt = "_GET_FWD" ;
              host_port = get_abcd_ip_format(m._ip) + ':' + std::to_string(m._port) ; break ;
          case (unsigned int)DM_message_types::DM_DEL_FWD: mt = "_DEL_FWD" ;
              host_port = get_abcd_ip_format(m._ip) + ':' + std::to_string(m._port) ; break ;
          case (unsigned int)DM_message_types::DM_WHAT_DATA: mt = "_GET_DATA" ; break ;
          case (unsigned int)DM_message_types::DM_PUT_REPLY_DATA: mt = "_PUT_REPLY" ; break ;
          case (unsigned int)DM_message_types::DM_GET_REPLY_DATA: mt = "_GET_REPLY" ; break ;
          case (unsigned int)DM_message_types::DM_DEL_REPLY_DATA: mt = "_DEL_REPLY" ; break ;
          default: mt = "_NO_MSG" ; break ;
       }

       os << "{" << mt << ": key (" << m._k << ") origin (" << host_port << ")}";

       return os ;
   }
};

// process a received message: _FWD_
bool process_message(DM_message_& msg) ;
bool process_answer(void* handle) ;


// Sockets Server running in background
void dm_node_srv(void* handle) ;

_DEFS_DM_NODES_PROTOCOL_
#endif
// eof dm_nodes_protocol.h
