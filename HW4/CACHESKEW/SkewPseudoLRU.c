/* Copyright Andre Seznec, Jerome Hedouin, INRIA-IRISA (1997) */

/* single-bit replacement policy */
/* this replacement policy only works for two banks */
/* remark: Cache_Prio is the tag bit associated with each line */
 

#include "cacheLibrary.h"
extern void (*Mapfct) ();


void AllocateSKEWpseudoLRU(int, CACHEINT*, int);
int Is_In_CacheSKEWpseudoLRU(int, CACHEINT*, int);


void AllocateSKEWpseudoLRU(int addr, CACHEINT *cache, int memtype )
{
  
  int		i, set;
  
  Mapfct( addr, cache, memtype );
 
  set = (*cache).Cache_Prio[AdBank[0]];
  
  for (i = 0; i < (*cache).Number_of_Bank; i++){
    if ((*cache).Cache[AdBank[i]] == FreeAddr) {
      set = i;
    }
  }
#ifdef SIML2
  EXCLUDE= (*cache).Cache[AdBank[set]];
#endif
  (*cache).Cache[AdBank[set]] = addr;
  (*cache).Cache_Prio[AdBank[0]] = 1 - set;
  
}



int Is_In_CacheSKEWpseudoLRU(int addr, CACHEINT *cache, int memtype )
{
  int	i;
  
  if ((*cache).Number_of_Bank != 2){
    printf("ERROR: The SKEWpseudoLRU replacement policy only works for two banks.");
    exit(0);
  }

  Mapfct( addr, cache, memtype );
  
  for (i = 0; i < (*cache).Number_of_Bank; i++){
    if ((*cache).Cache[AdBank[i]] == addr) {
      (*cache).Cache_Prio[AdBank[0]] = 1 - i;
      return (1);
    }
  }
  return (0);
  
}
























