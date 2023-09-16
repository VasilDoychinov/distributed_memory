// dm_task.cpp: the Service to be run as a separate thread
//

#include "dm_task.h"
#include "_dm_task.h"

#include "Logger_decl.hpp"
#include "Logger_helpers.hpp"

#include "communication/dm_communication.h"
#include "communication/dm_nodes_protocol.h"

#include "dm_localDB.h"
#include "misc/conversions.h"

DM_service_::DM_service_(unsigned int nn, unsigned int ni, uint32_t ip, unsigned int port)
    : _nodes_number{nn}, _node_ind{ni}, _ip_addr{ip}, _port{port}
{
   Log_to(0, "\n: DM_service() with ", nn, " nodes") ;
   socket_init() ;      // initiate SOCKETS frame: Windows mostly
}

DM_service_::~DM_service_()
{
   // TODO: CLOSE all running service threads

   // close sockets
   Log_to(0, ": closing TCP master socket: ", _tcp_ms), Log_to(0, ": closing UDP master socket: ", _udp_ms),
   Log_to(0, ": closing UDP peer socket: ", _udp_ps, '\n') ;
   if (_next_node)   freeaddrinfo(_next_node);   // release udp address of the successor

   CLOSE_SOCKET(_tcp_ms), CLOSE_SOCKET(_udp_ms);
   socket_clear() ;     // ...
}

bool
DM_service_::is_localDM(DM_enquiry& enq) const&   // just a simulation, of course
{
   return this->_node_ind == enq._k % this->_nodes_number  ;
}


DM_enquiry*
DM_service_::dm_current_enquiry()
{
   DM_signal*    sig = this->_comm ;      // not modifying
   DM_enquiry*   enq ;

   return (sig && (enq = sig->get_enquiry())) ? enq : nullptr ;
}

//

// DM communication processes(tasks)

void
DM_service_::dm_stop_servers() const&
{
   DM_message_  m_fwd {DM_message_types::DM_STOP_SERVICE, 0, this->_ip_addr, this->_port } ;
   int flags = 0 ;
   sendto(_udp_ms, &m_fwd, sizeof(m_fwd), flags, _this_node->ai_addr, _this_node->ai_addrlen) ;
}

// udp client (runs in background): relay a message to the next node (ring topology)
bool
DM_service_::dm_udp_client(DM_enquiry& enq) const&
{
   // prepare a message from enq
   DM_message_  m_fwd{ enq._e == DM_enquiry_type::DM_update
                          ? DM_message_types::DM_PUT_FWD
                          : (enq._e == DM_enquiry_type::DM_remove
                             ? DM_message_types::DM_DEL_FWD
                             : DM_message_types::DM_GET_FWD
                            ),
                       enq._k,
                       this->_ip_addr, this->_port
                     } ;

   return this->dm_udp_client(m_fwd) ;
}

bool
DM_service_::dm_udp_client(DM_message_& m_fwd) const&
{
    // send message to the next mode via UDP
   int   flags = 0 ;   // model the behaviour of sendto()
   auto  res = sendto(_udp_ps, &m_fwd, sizeof(m_fwd), flags, _next_node->ai_addr, _next_node->ai_addrlen) ;
   // Log_to(0, "> msg ", m_fwd, "-> ", res, " bytes sent to socket(", _udp_ps, ")") ;

   return res == sizeof(m_fwd) ;
}

bool
dm_busy(void *handle)
{
   assert(handle) ;
   return static_cast<DM_service_ *>(handle)->_comm != nullptr ;
}

void dm_task(void* handle)
{
   assert(handle) ;
   static DM_service_*  h = static_cast<DM_service_ *>(handle) ;
                                                                         Log_to(0, "\n> the Service is launched: #",
                                                                                   std::this_thread::get_id()) ;
   static DM_signal*    sig ;   // get to a signal
   while (true) {

      DM_enquiry*   saved_enq = nullptr ;
      while (!(sig = h->_comm.load(std::memory_order_acquire))) std::this_thread::yield()  ; // spin until a signal appears

      DM_enquiry*   enq = sig->get_enquiry() ; // old ??? h->dm_current_enquiry() ;
      if (enq)   {                                             // Log_to(0, ": signal received ", *enq, " -> processed...") ;
         // processinng the Signal
         if (enq->_e == DM_enquiry_type::DM_stop) {  // Quit: set the future, clear signal and exit
            // stop Sockets Server
            // h->_stop.store(true, std::memory_order_release) ;  // no use with a blocking select()
            h->dm_stop_servers() ; std::this_thread::sleep_for(100ms) ;
            // back to Caller
            sig->set_value(DM_enquiry{}), clear_signal(h) ;
            break ;
         } else if (enq->_e == DM_enquiry_type::DM_show_local)   {
            dm_db_print() ;
            sig->set_value(*enq), clear_signal(h) ;
            continue ;
         }

         DM_enquiry   we = *enq ;                                        // copy of the current ...
         if (h->is_localDM(we))   {
            dm_localDB(we) ;
            if (we._r != -1)   sig->set_value(we) ;                      // signal out the Result (future)
            else               sig->set_exception() ;
            clear_signal(h) ;
         } else {
                                                                         // Log_to(0, ": processing a request ", we,
                                                                         //           " to remote location...") ;
            // forward the enquiry to the next node
            if (!(h->dm_udp_client(we)))   {  // error on UDP forward
               Log_to(0, "\n> error occured on udp sendto()") ;
               sig->set_exception(), clear_signal(h) ;
            }
         }
      }
      while (h->_comm.load(std::memory_order_acquire)) std::this_thread::yield()  ; // spin until the signal clears = fulfilled
   }
   Log_to(0, "\n> the Service exits...") ;
} // dm_task()

void* dm_launch(unsigned int tsize, unsigned int node_index,
                const char* ip_addr,
                const char* udp_port, const char* tcp_port,
                const char* udp_port_next)
{
   Log_to(0, "\n: launching the Service (node: ", node_index, ") with: IP (", ip_addr,
             ") and ports: UDP (", udp_port, ") UDP_next (", udp_port_next,
             ") TCP (", tcp_port, ")") ;

   // const void*  pn = ip_addr ;
   static DM_service_ dm_serv{tsize, node_index,
                              get_ip_integer_equivalent(static_cast<const uint8_t *>(static_cast<const void*>(ip_addr))),
                              (unsigned int)atoi(tcp_port)
                             } ;

   std::cout << "\n: DM_signal is always-lock-free: " << dm_serv._comm.is_always_lock_free ;
   std::cout << "\n: DM_signal is        lock-free: " << dm_serv._comm.is_lock_free() ;

   // setting master sockets: ??? in the Constructor
   dm_serv._tcp_ms = tcp_socket_listen(ip_addr, tcp_port, DM_service_::_NCONN_TO_LISTEN) ;
   if (!ISVALID_SOCKET(dm_serv._tcp_ms))    return nullptr ;
   dm_serv._udp_ms = udp_socket_local(ip_addr, udp_port, DM_service_::_NCONN_TO_LISTEN, &(dm_serv._this_node)) ;
   if (!ISVALID_SOCKET(dm_serv._udp_ms))    return nullptr ;

   dm_serv._udp_ps = udp_socket_remote(ip_addr, udp_port_next, &(dm_serv._next_node)) ;
   if (!ISVALID_SOCKET(dm_serv._udp_ps))  return nullptr ;
   // resource cleaning in case of an error: in the Destructor of dm_serv

   /* to signal STOP through the flag: no use with a blocking select()
   dm_serv._stop.store(false, std::memory_order_release) ;  // to stop Sockets Server */

   std::thread   th(dm_task, &dm_serv) ; th.detach() ;
   std::thread   th_sock(dm_node_srv, &dm_serv) ; th_sock.detach() ;

   return &dm_serv ;
} // dm_launch()


std::future<DM_enquiry> send_signal(void* handle, void* s)   // set internal pointer to the Signal
{                                                            // return the future associated
   auto   fut = ((DM_signal *)s)->get_future() ;
                                                               // Log_to(0, "> setting a signal") ;
   DM_signal*    wp = nullptr ;
   DM_service_*  h = static_cast<DM_service_ *>(handle) ;
   try {  // ie set exception* to a promise
      if (!(h->_comm.compare_exchange_strong(wp, (DM_signal *)s))) {  // not changed, ie was NOT nullptr: set exception
         throw std::runtime_error("> DM_signal overload atempt...") ;
      }
   } catch (...) { ((DM_signal *)s)->set_exception() ; }

   return fut ;
}

void clear_signal(void* handle)
{
   static_cast<DM_service_ *>(handle)->clear_signal() ;
}

// eof dm_task.cpp
