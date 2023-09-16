// dm_localDB.cpp: see dm_localDB.h
//

#include "dm_localDB.h"

bool dm_localDB(DM_enquiry& enq)  // simulation
{
    bool   fl = false ;
   switch (enq._e)   {
      case DM_enquiry_type::DM_update: fl = dm_db_update(enq) ; break ;
      case DM_enquiry_type::DM_remove: fl = dm_db_delete(enq) ; break ;
      case DM_enquiry_type::DM_retrieve: fl = dm_db_retrieve(enq) ; break ;
      default: enq._e = DM_enquiry_type::DM_stop ;
   }
   return fl ;
}

std::map<int, int> simulateDB ;

void  dm_db_initiate(unsigned int nn, unsigned int nind)
{
   for (unsigned int i = 0 ; i < 5 ; ++i)   simulateDB[nind + i * nn] = i + nind ;
}

// @return: result, @v: the old value, if any ; otherwise - hold the new
bool dm_db_update(DM_enquiry& e)  // int k, int& v)
{
   int   nv = e._d ;               // v ;
   if (simulateDB.count(e._k) != 0)   e._d = simulateDB[e._k] ;
   simulateDB[e._k] = nv ;
   e._r = 1 ;
   return true ;
}

// @return: result, @v: the retrieved value, if any ; otherwise - undefined
bool dm_db_retrieve(DM_enquiry& e)  // int k, int& v)
{
   if (simulateDB.count(e._k) != 0)   e._d = simulateDB[e._k], e._r =  1 ;
   else                               e._d = e._r = 0 ; /// not found
   return e._r == 1 ;
}

// @return: result, @v: the old value, if any ; otherwise - undefined
bool dm_db_delete(DM_enquiry& e)    // int k, int& v)
{
   if (simulateDB.count(e._k) != 0)  e._d = simulateDB[e._k], e._r = 1 ;
   else                              e._r = 0 ;

   return simulateDB.erase(e._k) > 0 ;
}


void dm_db_print()
{
   std::string   pr_str{} ;

   for (auto it = simulateDB.begin() ; it != simulateDB.end() ; ++it) {
      if (it != simulateDB.begin() && std::distance(simulateDB.begin(), it) % 10 == 0)   pr_str += "\n:::" ;
      pr_str += " <" + std::to_string(it->first) + ", " + std::to_string(it->second) + '>' ;
   }
   Log_to(0, "> # of elements: ", simulateDB.size(), ": ", pr_str) ;
}


// eof dm_localDB.cpp
