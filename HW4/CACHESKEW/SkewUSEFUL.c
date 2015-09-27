/* Copyright Andre Seznec, Jerome Hedouin, INRIA-IRISA (1997) */

/* Usefulness policy */
/* works only for 2-way caches */


#include "cacheLibrary.h"
extern void (*Mapfct) ();

void AllocateSKEWUseful(int, CACHEINT*, int);
int Is_In_CacheSKEWUseful(int, CACHEINT*, int);

void AllocateSKEWUseful( int addr, CACHEINT *cache, int memtype )
{
  
  int  i, set;
  
  Mapfct( addr, cache, memtype );
  set = -1;
  if ((*cache).Cache_Prio[AdBank[0]] == (*cache).Cache_Prio[AdBank[1]]){
    set = (*cache).Cache_Prio[AdBank[0]];
  }else{
    set = random() & ((*cache).Number_of_Bank - 1);
  }
  
  for (i = 0; i < (*cache).Number_of_Bank; i++){
    if ((*cache).Cache[AdBank[i]] == FreeAddr) {
      set = i;
    }
  }
  (*cache).Cache_Prio[AdBank[0]] = 1 - set;
  (*cache).Cache_Prio[AdBank[1]] = 1 - set;
#ifdef SIML2
  EXCLUDE= (*cache).Cache[AdBank[set]];
#endif
  (*cache).Cache[AdBank[set]] = addr;
      
}



int Is_In_CacheSKEWUseful( int addr, CACHEINT *cache, int memtype )
{
  
  int	i;
  
  if ((*cache).Number_of_Bank != 2){
    printf("ERROR: The SkewUseful replacement policy only works for two banks.");
    exit(0);
  }

  Mapfct( addr, cache, memtype );
  
  for (i = 0; i < (*cache).Number_of_Bank; i++) {
    if ((*cache).Cache[AdBank[i]] == addr) {
      (*cache).Cache_Prio[AdBank[0]] = 1 - i;
      (*cache).Cache_Prio[AdBank[1]] = 1 - i;
      if ((*cache).Cache_DATE[AdBank[i]] < ((*cache).DATE & (0xffffffff - ((*cache).Size / 2 - 1)))){
	(*cache).DATE++;
      }
      (*cache).Cache_DATE[AdBank[i]] = (*cache).DATE;
      return (1);
    }
  }
  return (0);
  
}

