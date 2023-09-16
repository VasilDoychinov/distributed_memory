// dm_localDB.h:   to provide local memory functionality, checks, etc
//

#ifndef _DEFS_DM_LOCALDB_
#define _DEFS_DM_LOCALDB_

#include <thread>
#include <map>
#include <string>

#include "Logger_decl.hpp"
#include "Logger_helpers.hpp"

#include "dm_task.h"

// DM task: based on what's in handle._comm, perfom the operation
//                  return the result in the future associated
//    -- launched in dm_task()
bool dm_localDB(DM_enquiry& enq) ;

// @return: result, @v: the old value, if any ; otherwise - hold the new
bool dm_db_update(DM_enquiry& enq) ;
// @return: result, @v: the retrieved value, if any ; otherwise - undefined
bool dm_db_retrieve(DM_enquiry& enq) ;
// @return: result, @v: the old value, if any ; otherwise - undefined
bool dm_db_delete(DM_enquiry& enq) ;

void dm_db_initiate(unsigned int nn, unsigned int nind) ;
void dm_db_print() ;

_DEFS_DM_LOCALDB_
#endif
// eof dm_localDB.h
