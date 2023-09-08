// dm_task.cpp: the Service to be run as a separate thread
//

#include "dm_task.h"
#include "_dm_task.h"

#include "Logger_decl.hpp"
#include "Logger_helpers.hpp"

#include "dm_communication.h"


DM_service_::DM_service_(unsigned int nn) : _nodes_number{nn}
{
   Log_to(0, "\n: DM_service() with ", nn, " nodes") ;
   socket_init() ;      // initiate SOCKETS frame: Windows mostly
}

DM_service_::~DM_service_()
{
   Log_to(0, ": closing TCP master socket: ", _tcp_ms), Log_to(0, ": closing UDP master socket: ", _udp_ms),
   Log_to(0, ": closing UDP peer socket: ", _udp_ps, '\n') ;
   if (_next_node)   freeaddrinfo(_next_node);   // release udp address of the successor
   CLOSE_SOCKET(_tcp_ms), CLOSE_SOCKET(_udp_ms);
   socket_clear() ;     // ...
}


//

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
                                                                         // Log_to(0, "\n> the Service is launched ...") ;
   // std::this_thread::sleep_for(10s) ;
   // Log_to(0, "\n> Returning...") ;
   // return ;

   while (true) {

      DM_signal*    sig = h->_comm ;      // not modifying
      DM_enquiry*   enq ;
      if (sig && (enq = sig->_penq))   {   Log_to(0, ": signal received ", *enq, " -> processed...") ;

         // processinng the Signal
         if (enq->_e == DM_enquiry_type::DM_stop) {  // Quit: set the future, clear signal and exit
             sig->set_value(DM_enquiry{}), clear_signal(h) ;
             break ;
         }

         DM_enquiry   we = *enq ; we._d += (int)we._e + we._k ;

         sig->set_value(we) ;  // signal for the Result (future)
         clear_signal(h) ;
      }
      std::this_thread::yield() ;
   }
   Log_to(0, "\n> the Service exits...") ;
}

void* dm_launch(unsigned int tsize, const char* ip_addr,
                const char* udp_port, const char* tcp_port,
                const char* udp_port_next)
{
   Log_to(0, "\n: launching the Service with: IP (", ip_addr,
             ") and ports: UDP (", udp_port, ") UDP_next (", udp_port_next,
             ") TCP (", tcp_port, ")") ;

   static DM_service_   dm_serv{tsize} ;
   std::cout << "\n: DM_signal is always-lock-free: " << dm_serv._comm.is_always_lock_free ;
   std::cout << "\n: DM_signal is        lock-free: " << dm_serv._comm.is_lock_free() ;

   // setting master sockets: ??? in the Constructor
   dm_serv._tcp_ms = tcp_socket_listen(ip_addr, tcp_port, DM_service_::_NCONN_TO_LISTEN) ;
   if (!ISVALID_SOCKET(dm_serv._tcp_ms))    return nullptr ;
   dm_serv._udp_ms = udp_socket_local(ip_addr, udp_port, DM_service_::_NCONN_TO_LISTEN, nullptr) ;
   if (!ISVALID_SOCKET(dm_serv._udp_ms))    return nullptr ;

   dm_serv._udp_ps = udp_socket_remote(ip_addr, udp_port_next, &(dm_serv._next_node)) ;
   if (!ISVALID_SOCKET(dm_serv._udp_ps))  return nullptr ;
   // resource cleaning in case of an error: in the Destructor of dm_serv

   std::thread   th(dm_task, &dm_serv) ; th.detach() ;

   return &dm_serv ;
} // dm_launch()


std::future<DM_enquiry> send_signal(void* handle, void* s)   // set internal pointer to the Signal
{                                                            // return the future associated
   auto   fut = ((DM_signal *)s)->get_future() ;
                                                             Log_to(0, "> setting a signal") ;
   DM_signal*    wp = nullptr ;
   DM_service_*  h = static_cast<DM_service_ *>(handle) ;
   try {
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
