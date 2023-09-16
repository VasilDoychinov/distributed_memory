// main.cpp: Distributed memory simulation
//      for some details:   Readme.txt
//
//
//

#include <iostream>
#include <thread>
#include <string>

#include "Logger_decl.hpp"
#include "Logger_helpers.hpp"


#include "src/dm_task.h"
#include "src/communication/dm_communication.h"
#include "src/dm_localDB.h"

// helpers
int get_clamped_int(int n_min, int n_max) ; // the outside numbers are set to n_min
int get_int() ;


int main(int argc, char** argv)
{
   // if (argc < 6)   { std::cout << "\nusage: _____ Index IP_address UDP_own UDP_next TCP_own\n" ; return 0 ; }
    // for testing
   if (argc < 3)   { std::cout << "\nusage: _____ Index OfNumber [] IP_address UDP_own UDP_next TCP_own\n" ; return 0 ; }

   std::ios_base::sync_with_stdio(false) ;
   Logger_type& wLog = logger_get() ;
   // wLog.log_args(0, "> logger with ", wLog.size(), " pre-defined method(s)") ;
   Log_to(0, "\n> # of hardware threads: ", std::thread::hardware_concurrency(),
             "\n> main() thread: ", std::this_thread::get_id(), '\n') ;

   unsigned int   node_index = std::atoi(argv[1]) ;
   unsigned int   nnodes = std::atoi(argv[2]) ;

   std::string ip_addr = "127.0.0.1" ;   // argv[2] ;           TESTING: using ports 3000,3001 for node 0
   const int   port_base = 2000 ;
   int         w_tcp = port_base + 2 * node_index ;
   std::string udp_port = std::to_string(w_tcp + 1) ;           // argv[3] ;
   std::string tcp_port = std::to_string(w_tcp) ;               // argv[5] ;
   w_tcp  = (node_index < nnodes - 1) ?  w_tcp + 2 + 1 : port_base + 1 ; // close the ring
   std::string udp_port_next = std::to_string(w_tcp) ;          // argv[4] ;

   dm_db_initiate(nnodes, node_index);

   auto handle = dm_launch(nnodes, node_index,               // DM_TASK_NUMBER_OF_NODES, node_index,
                           ip_addr.c_str(), udp_port.c_str(), tcp_port.c_str(), udp_port_next.c_str()) ;
   if (!handle)   { Log_to(0, "\n> unsuccessful launch. Exiting...\n") ; return 0 ; }
   std::this_thread::sleep_for(20ms) ;


   int   key, data, et ;
   try { // if an Exception would be propagated through promise<>.set_exception()
   while (true) {
      if (dm_busy(handle)) {
         Log_to(0, "\n> the Service is busy...") ;
         std::this_thread::sleep_for(100ms) ;
         continue ;
      }

      Log_to(0, "\n\n(", node_index, " of ", nnodes, ")> choose(1 - update; 2 - read; 3 - remove; 4 - print Local; others - QUIT): ") ;
      DM_enquiry_type   etype = static_cast<DM_enquiry_type>(get_clamped_int(0, 4)) ;

      key = data = 0 ;
      if (etype != DM_enquiry_type::DM_stop && etype != DM_enquiry_type::DM_show_local)   {
         Log_to(0, ": enter key (< 0 to quit): ") ; key = get_int() ;
         Log_to(0, ": enter data(< 0 to quit): ") ; data = get_int() ;
      }

      DM_enquiry   enq{etype, key, data} ; // a buffer, its address will be moved around
                                                                         // Log_to(0, ": sending: ", enq) ;
      DM_signal    esig{&enq} ;            // a buffer: mind the blocking result.get()

      auto   result = send_signal(handle, &esig) ; // result.get() ;   wait for the result
      Log_to(0, "\n: RECEIVED: ", result.get()) ;   // BLOCKING call on get()

      if (etype == DM_enquiry_type::DM_stop)   break ;
   }
   } catch (std::exception& e) {
      Log_to(0, "\n\n> exception caught: ", e.what()) ;
   } catch (...)  {
      Log_to(0, "\n\n> Unknown Exception has been thrown...") ;
   }

   Log_to(0, LOG_SOURCE_LOCATION, LOG_TIME_LAPSE(Log_start()), '\n',
             std::string{"\n> That's it...\n"}) ;

   return 0 ;
}


int get_int() { fflush(stdin) ; int i ; std::cin >> i ; return i ; }

int get_clamped_int(int l, int h)
{
   // clear stdin
   // char lbuff[100] ; // fgets(lbuff, sizeof(lbuff), stdin), fflush(stdin) ;
   // get the choice
   // fgets(lbuff, sizeof(lbuff), stdin), fflush(stdin) ;
   // return std::atoi(lbuff) ;
   int   inp = get_int() ;
   if (inp < l || inp > h)   inp = l ;
   return inp ;
}

// eof main.cpp
