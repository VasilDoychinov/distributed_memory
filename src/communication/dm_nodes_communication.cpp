// dm_nodes_communication.cpp: implementation of node-to-node communication
//

#include "dm_nodes_protocol.h"

#include "tcp_sockets.h"
#include "udp_sockets.h"

#include "../_dm_task.h"

void dm_node_srv(void* handle)
{
   assert(handle) ;

   static DM_service_*  h = static_cast<DM_service_ *>(handle) ;
   // ??? DM_enquiry*   enq ;
   DM_message_   message{} ;    // message buffer

   int       num_sockets{} ;
   // timeval   time_out ; memset(&time_out, 0, sizeof(time_out)) ; for a non-blocking select() is to be used

   Log_to(0, "> Sockets Server is launched: #", std::this_thread::get_id()) ;
   Log_to(0, "> sockets in use on node ", h->_node_ind,
             ": tcp (", h->_tcp_ms, "), udp (", h->_udp_ms, "), udp to remote (", h->_udp_ps, ")") ;

   auto s_tcp = h->_tcp_ms ;
   auto s_udp = h->_udp_ms ;
   auto s_next = h->_udp_ps ;
   auto s_max = std::max(s_tcp, std::max(s_udp, s_next)) ;

   fd_set  master ; FD_ZERO(&master) ;
   FD_SET(s_tcp, &master), FD_SET(s_udp, &master) ;

   while (true) {
      // Log_to(0, "### socket server active...: ", h->_stop) ;
      // no use with blocking select(): if (h->_stop.load(std::memory_order_acquire))   break ;

      fd_set   reads = master ;   // majors only: the TCP connection is one iteration only
      num_sockets = select_readsNB(s_max + 1, &reads) ; // for Non-blocking call use: , &time_out) ;
      // if (select_reads(s_max + 1, &reads)) { Log_to(0, "\n> dm_node_srv() error...") ; }
      // NB: with a blocking call - this_thread is just terminated upon exit

      if (num_sockets < 0)   { Log_to(0, "\n> dm_node_srv(): error on select()...") ; break ; }
      else if (num_sockets == 0) { std::this_thread::yield() ; continue ; } // not happening on blocking select()

      // analyze "to be read" sockets
         // UDP
      if (is_present(s_udp, &reads))   { // REMOTE enquiry received (see the Proocol)
         int   flags = 0 ;   // model the behaviour of recvfrom()
         auto  res = recvfrom (s_udp, &message, sizeof(message), flags,
                              (struct sockaddr*)&(h->_prev_node_addr), &(h->_prev_node_addr_len)) ;  // here is the only update
         if (res != sizeof(message))   { Log_to(0, "\n> dm_node_srv(): error on UDP recvfrom()") ; }
                                                               // Log_to(0, "> msg ", message, "-> ", res, " bytes received on socket(", s_udp, ")",
                                                               //           ": ", res == sizeof(message) ? "OK" : "with ERROR") ;
         if (message._pid == (unsigned int)DM_message_types::DM_STOP_SERVICE)   break ;   // STOP signal

         DM_enquiry   we{DM_enquiry_type::DM_stop, message._k, message._d} ;
         if (h->is_localDM(we))   {                            // Log_to(0, "\n: this node will process ", message) ;
            process_message(message) ;   // _FWD_: use TCP to request data, process, return result to the Origin
         }
         else   {
            h->dm_udp_client(message) ;
                                                               // Log_to(0, "\n: sending msg ", message,
                                                               //           " further: ", h->dm_udp_client(message) ? "OK" : "ERROR") ;
         }
      }
         // TCP
      if (is_present(s_tcp, &reads))   { //  connection requested on TCP
         if (h->_comm == nullptr) Log_to(0, "\n> dm_node_srv() : error before process_answer() h->_comm is nullptr") ;
         if (!process_answer(h))   break ;  // would indicate bad synchronization or that heavy problems
      }
      // Log_to(0, "### socket server active...") ;
   }

   Log_to(0, "\n> Sockets Server exits...") ;
}

#include "../dm_localDB.h"


bool process_answer(void* handle)  // handles messages received on the Originator of a request (TCP)
{
   assert(handle) ;

   static DM_service_*  h = static_cast<DM_service_ *>(handle) ;

   DM_signal* sig = h->_comm.load(std::memory_order_relaxed) ;  // limited within two spin-locks
   if (!sig) { Log_to(0, "\n> process_answer(): missing current signal: DEEP shit") ; return false ; }
   auto       cenq = sig->get_enquiry() ;                       // ??? old: h->dm_current_enquiry() ;
   if (!cenq)  { Log_to(0, "\n> process_answer(): missing current enquiry: DEEP shit") ; return false ; }       // deep shit

   DM_message_    msg{} ;

   struct sockaddr_storage    addr ;
   socklen_t                  alen = sizeof(addr) ;
   SOCKET   s = tcp_socket_conn(h->_tcp_ms, &addr, &alen) ;   // accept() on master TCP

   auto  res = recv(s, &msg, sizeof(msg), 0) ;
   if (!ISVALID_SOCKET(s) || res != sizeof(msg))   {
      sig->set_exception(), clear_signal(h) ;
      CLOSE_SOCKET(s) ;
      return false ;
   }

   // process the answer/request, etc
   if (msg._pid == (unsigned int)DM_message_types::DM_GET_REPLY_DATA)  { // answer to a GET enquiry received
                                                               // Log_to(0, "> processing received ", msg) ;
      DM_enquiry  enq{DM_enquiry_type::DM_retrieve, msg._k, msg._d, msg._r} ;
      sig->set_value(enq), clear_signal(h) ;
   } else if (msg._pid == (unsigned int)DM_message_types::DM_WHAT_DATA) {
      // prepare the answer to data request
      msg._pid = (unsigned int)DM_message_types::DM_PUT_REPLY_DATA ;
      msg._d = cenq->_d ;
      // answer to the data request
      res = socket_send(s, (const char *)&msg, sizeof(msg)) ;
      if (res == sizeof(msg))   {
         // wait for and GET the Result as a _PUT_REPLY message
         res = recv(s, &msg, sizeof(msg), 0) ;
         if (res == sizeof(msg))   {
            DM_enquiry  enq{DM_enquiry_type::DM_update, msg._k, msg._d, msg._r} ;
            sig->set_value(enq) ;
         } else  sig->set_exception() ;
      } else     sig->set_exception() ;
      clear_signal(h) ;
   } else if (msg._pid == (unsigned int)DM_message_types::DM_DEL_REPLY_DATA)  { // answer to a DEL enquiry received
                                                               // Log_to(0, "> processing received ", msg) ;
      DM_enquiry  enq{DM_enquiry_type::DM_remove, msg._k, msg._d, msg._r} ;
      sig->set_value(enq), clear_signal(h) ;
   }

   CLOSE_SOCKET(s) ;
   return true ;
} // process_answer()


bool
process_message(DM_message_& msg)   // received via UDP remote request
{                                   // contacts the originator regarding the request
   // the three casses follow all steps (separately), for simplicity's sake
                                                               // Log_to(0, "\n> processing ", msg) ;
   DM_enquiry   enq{DM_enquiry_type::DM_stop, msg._k, msg._r} ;
   if (msg._pid == (int)DM_message_types::DM_GET_FWD)   {
      DM_enquiry   enq{DM_enquiry_type::DM_stop, msg._k, msg._r} ;

      msg._pid = (unsigned int)DM_message_types::DM_GET_REPLY_DATA ;
      msg._r = dm_db_retrieve(enq) ? 1 : 0 ;    // as a result from quering the DB for now
      msg._k = enq._k, msg._d = enq._d ;

      // connect to the Origin (as in the message) and send reply via TCP
      auto sconn = tcp_connect(msg.get_ip_addr().c_str(), msg.get_port().c_str()) ;
      auto res = socket_send(sconn, (const char*)&msg, sizeof(msg)) ;
      if (res != sizeof(msg))   Log_to(0, "\n> process_message(): ", msg, " - error on send _GET_REPLY") ;
      // else                      Log_to(0, "\n> message ", msg, " sent to the Originator") ;
      CLOSE_SOCKET(sconn);
   } else if (msg._pid == (int)DM_message_types::DM_PUT_FWD)  {
      msg._pid = (unsigned int)DM_message_types::DM_WHAT_DATA ;
      // connect to the Origin (as in the message) and send reply via TCP
      auto sconn = tcp_connect(msg.get_ip_addr().c_str(), msg.get_port().c_str()) ;
      auto res = socket_send(sconn, (const char*)&msg, sizeof(msg)) ;

      if (res == sizeof(msg))   { // move on
         res = recv(sconn, (char *)&msg, sizeof(msg), 0) ;
         if (res != sizeof(msg) || msg._pid != (unsigned int)DM_message_types::DM_PUT_REPLY_DATA)  {
            // error processing is scheduled for the next stage :)
            // like retrun a message to the Originator, throw an exception, or ...
                                                               // Log_to(0, "\n> process_message(): error on send _PUT_REPLY") ;
         } else {
            // update the local DB ??? and return result (old value) to the Originator
            DM_enquiry  enq{DM_enquiry_type::DM_update, msg._k, msg._d} ;
            msg._r = dm_db_update(enq) ? 1 : 0 ;
            msg._k = enq._k, msg._d = enq._d ;
            if (msg._r == 1)   msg._d = enq._d ;  // store the old data
         }
      } else {
         Log_to(0, "\n> process_message(): error on send _WHAT_DATA") ;
         msg._r = -1 ; // error
      }

      // return the result (and the old data eventually)
      res = socket_send(sconn, (const char*)&msg, sizeof(msg)) ;
      if (res != sizeof(msg))   Log_to(0, "\n> process_message(): error on send _PUT_REPLY as a RESULT") ;

      CLOSE_SOCKET(sconn);
   } else if (msg._pid == (int)DM_message_types::DM_DEL_FWD)  {   // follows _GET_FWD
      DM_enquiry   enq{DM_enquiry_type::DM_stop, msg._k, msg._r} ;

      msg._pid = (unsigned int)DM_message_types::DM_DEL_REPLY_DATA ;
      dm_db_delete(enq) ;
      msg._r = enq._r, msg._k = enq._k, msg._d = enq._d ;

      // connect to the Origin (as in the message) and send reply via TCP
      auto sconn = tcp_connect(msg.get_ip_addr().c_str(), msg.get_port().c_str()) ;
      auto res = socket_send(sconn, (const char*)&msg, sizeof(msg)) ;
      if (res != sizeof(msg))   Log_to(0, "\n> process_message(): ", msg, " - error on send _DEL_REPLY") ;
      // else                      Log_to(0, "\n> message ", msg, " sent to the Originator") ;
      CLOSE_SOCKET(sconn);
   }

   return true ;
} // process_message()

// eof dm_nodes_protocol.cpp
