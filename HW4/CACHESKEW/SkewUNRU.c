/* Copyright Andre Seznec, Jerome Hedouin, INRIA-IRISA (1997) */

/* Useful + NRU */


#include "cacheLibrary.h"
extern void (*Mapfct) ();

void AllocateSKEWUNRU(int, CACHEINT*, int);
int Is_In_CacheSKEWUNRU(int, CACHEINT*, int);

void AllocateSKEWUNRU( int addr, CACHEINT *cache, int memtype )
{
  /*NRU + Useful */
  /* This replacement policy only works for 2-way caches*/  
  /* remark: Cache_Prio stands for the "Useful tag" */
  
  int             Young0, Young1, i, set;
  int             pseudoDATE;
  
  Mapfct( addr, cache, memtype );
  
  set = -1;
  pseudoDATE = (*cache).DATE & (0xffffffff - (((*cache).Size / 2) - 1));
  Young0 = ((*cache).Cache_DATE[AdBank[0]] >= pseudoDATE);
  Young1 = ((*cache).Cache_DATE[AdBank[1]] >= pseudoDATE);
  
  if (Young0 != Young1) {
    if (Young0){
      set = 1;
    }else{
      set = 0;
    }
  } else {
    if ((*cache).Cache_Prio[AdBank[0]] == (*cache).Cache_Prio[AdBank[1]]){
      set = (*cache).Cache_Prio[AdBank[0]];
    }else{
      set = random() & ((*cache).Number_of_Bank - 1);
    }
  }
  
  
  for (i = 0; i < (*cache).Number_of_Bank; i++){
    if ((*cache).Cache[AdBank[i]] == FreeAddr) {
      set = i;
    }
  }

  (*cache).Cache_Prio[AdBank[1]] = 1 - set;
  (*cache).Cache_Prio[AdBank[0]] = 1 - set;    
  (*cache).Cache_DATE[AdBank[set]] = (*cache).DATE;
  (*cache).DATE++;

#ifdef SIML2
      EXCLUDE= (*cache).Cache[AdBank[set]];
#endif
      (*cache).Cache[AdBank[set]] = addr;
      
}



int Is_In_CacheSKEWUNRU( int addr, CACHEINT *cache, int memtype )
{
  
  int	i;
  
  if ((*cache).Number_of_Bank != 2){
    printf("ERROR: The SkewUNRU replacement policy only works for two banks.");
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


