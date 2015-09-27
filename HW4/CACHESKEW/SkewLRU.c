/* Copyright Andre Seznec, Jerome Hedouin, INRIA-IRISA (1997) */

/*
 * full LRU Replacement policy: cannot be implemented in hardware
 */


#include "cacheLibrary.h"
extern void (*Mapfct) ();

void AllocateSKEWLRU(int, CACHEINT*, int);
int Is_In_CacheSKEWLRU(int, CACHEINT*, int);

void AllocateSKEWLRU( int addr, CACHEINT *cache, int memtype )
{
  int	i, min, set;
  
  Mapfct( addr, cache, memtype );
  
  set = 0;
  min = (*cache).Cache_DATE[AdBank[0]];
  for (i = 1; i < (*cache).Number_of_Bank; i++){
    if ((*cache).Cache_DATE[AdBank[i]] < min) {
      set = i;
      min = (*cache).Cache_DATE[AdBank[i]];
    }
  }
  for (i = 0; i < (*cache).Number_of_Bank; i++){
    if ((*cache).Cache[AdBank[i]] == FreeAddr) {
      set = i;
    }
  }
#ifdef SIML2
  EXCLUDE= (*cache).Cache[AdBank[set]];
#endif
  (*cache).Cache[AdBank[set]]= addr;
  (*cache).Cache_DATE[AdBank[set]]= (*cache).ACCESS ;
  
}



int Is_In_CacheSKEWLRU( int addr, CACHEINT *cache, int memtype )
{
  int	i;

  Mapfct( addr, cache, memtype );
  
  for (i = 0; i < (*cache).Number_of_Bank; i++){
    if ((*cache).Cache[AdBank[i]] == addr) {
      (*cache).Cache_DATE[AdBank[i]] = (*cache).ACCESS;
      return (1);
    }
  }
  return (0);
  
}




