// dm_task.h: definitions, function prototypes, etc.
//      - the implementation in dm_task.cpp
//

#ifndef _DEFS_DM_TASK_
#define _DEFS_DM_TASK_

#include <iostream>

#include <string>
#include <future>

#define DM_TASK_NUMBER_OF_NODES   6   // size of the topology


enum class DM_enquiry_type { DM_stop, DM_update, DM_retrieve, DM_remove, DM_show_local } ;


struct DM_enquiry {   // abstract the enquiry: just to make it simple
    DM_enquiry_type   _e {DM_enquiry_type::DM_stop} ;
    int   _k{0} ;
    int   _d{0} ;
    int   _r{0} ;   // result: -1: error, 0: not foud, 1: found

    DM_enquiry() : _e{}, _k{}, _d{}, _r{} {}
    DM_enquiry(DM_enquiry_type et, int k, int v, int r = 0) : _e{et}, _k{k}, _d{v}, _r{r} {}

    friend std::ostream& operator<< (std::ostream& os, const DM_enquiry& e)   {
        return os << "{type: " << (int)e._e << ", key: " << e._k << ", value: " << e._d << ", result: "
               << (e._r == 1 ? "found" : (e._r ? "error" : "not found")) << '}' ;
    }
};

void* dm_launch(unsigned int tsize, unsigned int node_index,
                const char* ip_addr,
                const char* udp_port, const char* tcp_port,
                const char* udp_port_next) ;

std::future<DM_enquiry> send_signal(void* handle, void* s) ;
void                    clear_signal(void* handle) ;
bool                    dm_busy(void* handle) ;

_DEFS_DM_TASK_
#endif

// eof dm_task.h
