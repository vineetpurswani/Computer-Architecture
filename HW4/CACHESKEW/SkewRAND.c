/* Copyright Andre Seznec, Jerome Hedouin, INRIA-IRISA (1997) */

/* random replacement policy */

#include "cacheLibrary.h"
extern void (*Mapfct) ();

void AllocateSKEWRAND(int, CACHEINT*, int);
int Is_In_CacheSKEWRAND(int, CACHEINT*, int);



void AllocateSKEWRAND(int addr, CACHEINT *cache,int memtype )
{

  int	i, set;
  
  Mapfct( addr, cache, memtype );
  
  set = random() % (*cache).Number_of_Bank;

  for (i = 0; i < (*cache).Number_of_Bank; i++){
    if ((*cache).Cache[AdBank[i]] == FreeAddr) {
      set = i;
    }
  }

#ifdef SIML2
  EXCLUDE= (*cache).Cache[AdBank[set]];
#endif
  (*cache).Cache[AdBank[set]] = addr;
  
}



int Is_In_CacheSKEWRAND( int addr, CACHEINT *cache, int memtype )
{

  int	i;
  
  Mapfct( addr, cache, memtype );
  
  for (i = 0; i < (*cache).Number_of_Bank; i++){
    if ((*cache).Cache[AdBank[i]] == addr) {
      return (1);
    }
  }
  return (0);
}




