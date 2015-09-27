/* Copyright Andre Seznec, Jerome Hedouin, INRIA-IRISA (1997) */

#include "cacheLibrary.h"
extern void (*Mapfct) ();


void AllocateSKEWNRU(int, CACHEINT*, int);
int Is_In_CacheSKEWNRU(int, CACHEINT*, int);


void AllocateSKEWNRU( int addr, CACHEINT *cache, int memtype )
{
  int           Old[8], set;
  int		i, Start;
  int		pseudoDATE;
  
  Start = random() & ((*cache).Number_of_Bank - 1);
  /*the random number*/	
  
  Mapfct( addr, cache, memtype );
  
  set = -1;
  /* (*cache).DATE records the number of touched "old" cache lines*/
  
  /* Computing the minimum "date" for young */ 
  pseudoDATE = (*cache).DATE & (0xffffffff - (((*cache).Size / 2) - 1));
  
  /*chosing among old cache lines*/
  for(i=0; i<(*cache).Number_of_Bank; i++){
    Old[i] = (((*cache).Cache_DATE[AdBank[i]] < pseudoDATE));
  }
  for(i=Start; i<(*cache).Number_of_Bank; i++){
    if (Old[i]) set=i;
  }
  for(i=0; i<Start; i++){
    if (Old[i]) set=i;
  }
  
  /*chosing among young cache lines*/
  if (set== -1) {
    set =Start;
  }
  
  
  for (i = 0; i < (*cache).Number_of_Bank; i++){
    if ((*cache).Cache[AdBank[i]] == FreeAddr) {
      set = i;
    }
  }
  (*cache).Cache_DATE[AdBank[set]] = (*cache).DATE;
  (*cache).DATE++;
#ifdef SIML2
  EXCLUDE= (*cache).Cache[AdBank[set]];
#endif
  (*cache).Cache[AdBank[set]] = addr;
  
}



int Is_In_CacheSKEWNRU( int addr, CACHEINT *cache, int memtype )
{
  int	i;
  
  Mapfct( addr, cache, memtype );
  
  for (i = 0; i < (*cache).Number_of_Bank; i++) {
    if ((*cache).Cache[AdBank[i]] == addr) {
      if ((*cache).Cache_DATE[AdBank[i]] < ((*cache).DATE & (0xffffffff - ((*cache).Size / 2 - 1)))){
	(*cache).DATE++;
      }
      (*cache).Cache_DATE[AdBank[i]] = (*cache).DATE;
      return (1);
    }
  };
  return (0);
  
}













