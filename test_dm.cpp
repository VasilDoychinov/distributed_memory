// main.cpp: Distributed memory simulation
//      for details:   see Readme.txt
//
//
//

#include <iostream>
#include <thread>
#include <string>

#include "Logger_decl.hpp"
#include "Logger_helpers.hpp"


#include "src/dm_task.h"
#include "src/dm_communication.h"

// helpers
int get_int() ;


int main(int argc, char** argv)
{
   if (argc < 5)   { std::cout << "\nusage: _____ IP_address UDP_own UDP_next TCP_own\n" ; return 0 ; }

   std::ios_base::sync_with_stdio(false) ;
   Logger_type& wLog = logger_get() ;
   // wLog.log_args(0, "> logger with ", wLog.size(), " pre-defined method(s)") ;
   Log_to(0, "\n> # of hardware threads: ", std::thread::hardware_concurrency(),
             "\n> main() thread: ", std::this_thread::get_id(), '\n') ;

   std::string ip_addr = argv[1] ;
   std::string udp_port = argv[2] ;
   std::string udp_port_next = argv[3] ;
   std::string tcp_port = argv[4] ;

   auto handle = dm_launch(DM_TASK_NUMBER_OF_NODES, ip_addr.c_str(), udp_port.c_str(), tcp_port.c_str(), udp_port_next.c_str()) ;
   if (!handle)   { Log_to(0, "\n> unsuccessful launch. Exiting...\n") ; return 0 ; }

   int   key, data, et ;
   try { // if an Exception would be propagated through promise<>.set_exception()
   while (true) {
      if (dm_busy(handle)) {
         Log_to(0, "\n> the Service is busy...") ;
         std::this_thread::sleep_for(100ms) ;
         continue ;
      }

      Log_to(0, "\n\n: enter enquiry(1 - update; 2 - read; 3 - remove; otherwise - QUIT): ") ;
      DM_enquiry_type   etype = static_cast<DM_enquiry_type>(get_int()) ;
      if (etype != DM_enquiry_type::DM_stop)   {
         Log_to(0, ": enter key (< 0 to quit): ") ; key = get_int() ;
         Log_to(0, ": enter data(< 0 to quit): ") ; data = get_int() ;
      }

      DM_enquiry   enq{etype, key, data} ;
                                                                         // Log_to(0, ": sending: ", enq) ;
      DM_signal    esig{&enq} ;

      auto   result = send_signal(handle, &esig) ; // result.get() ;   wait for the result
      Log_to(0, "\n: EXPECTING received result(etype, key, et+k+d): ", result.get()) ;   // BLOCKING call on get()
                                                                                         // in Service: clear_signal() ; ???
      if (etype == DM_enquiry_type::DM_stop)   break ;
   }
   } catch (...) {
     Log_to(0, "\n\n> Exception has been thrown...") ;
   }

   Log_to(0, LOG_SOURCE_LOCATION, LOG_TIME_LAPSE(Log_start()), '\n',
             std::string{"\n> That's it...\n"}) ;

   return 0 ;
}

int get_int()
{
   // clear stdin
   // char lbuff[100] ; // fgets(lbuff, sizeof(lbuff), stdin), fflush(stdin) ;
   // get the choice
   // fgets(lbuff, sizeof(lbuff), stdin), fflush(stdin) ;
   // return std::atoi(lbuff) ;
   fflush(stdin) ;
   int   inp ; std::cin >> inp ;
   return inp ;
}

// eof main.cpp
