// dm_communication.h: communication between:
//      - main() and DM deamon
//      - nodes
//

#ifndef _DEFS_DM_COMMUNICATION_
#define _DEFS_DM_COMMUNICATION_

#include <future>
#include <atomic>
#include <utility>
#include <exception>

#include "../dm_task.h"


#include <iostream>

class DM_signal {     // move-only: move-from state holds _penq{nullptr}

   DM_enquiry*                 _penq {nullptr} ;  // access the enquiry
   std::promise<DM_enquiry>    _prom ;            // set the result in (when ready)

   public:
      DM_signal(DM_enquiry* enq) : _penq{enq}, _prom{} {}
      // ~DM_signal() { _penq = nullptr ; }

      DM_signal(DM_signal&& s) : _penq{s._penq}, _prom{std::move(s._prom)} {}
      DM_signal& operator= (DM_signal&& s) = delete ;

      DM_enquiry* get_enquiry() const& { return _penq ; }
      // DM_enquiry* clear_enquiry() & { DM_enquiry* we = _penq ; _penq = nullptr ; return we ; }

      std::future<DM_enquiry> get_future() { return _prom.get_future() ; }
      void   set_exception() { _prom.set_exception(std::current_exception()) ; }
      void   set_value(const DM_enquiry& enq) { _prom.set_value(enq) ; }
      void   set_value(DM_enquiry&& enq) { _prom.set_value(std::move(enq)) ; }

      friend void dm_task(void* handle) ;   // the deamon

      friend std::ostream& operator<< (std::ostream& os, const DM_signal* s) {
         DM_enquiry   e = *(s->_penq) ;
         return os << '{' << 'x' << ',' << e._k << ',' << e._d << '}' ;
      }
};

_DEFS_DM_COMMUNICATION_
#endif

// eof dm_communication.h
