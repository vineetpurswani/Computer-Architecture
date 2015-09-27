/* Copyright Andre Seznec, Jerome Hedouin, INRIA-IRISA (1997) */

/*Enhanced NRU*/


#include "cacheLibrary.h"
extern void (*Mapfct) ();

void AllocateSKEWENRU(int, CACHEINT*, int);
int Is_In_CacheSKEWENRU(int, CACHEINT*, int);


void AllocateSKEWENRU(int addr, CACHEINT *cache, int  memtype )
{
  int           Age[8], set;
  int		i, Start;
  int		pseudoDATE, pseudoDATE1;
  
  Start = random() & ((*cache).Number_of_Bank - 1);
  /*the random number*/	
  
  Mapfct( addr, cache, memtype );
  set = -1;
  
  /* Computing the two minimum "dates" for young and very young*/ 
  /* (*cache).DATE records the number of touched "old" cache lines*/
  pseudoDATE = (*cache).DATE & (0xffffffff - (((*cache).Size / 2) - 1));
  /*pseudoDATE records the last reset of one tag bit column*/
  
  pseudoDATE1 = ((*cache).DATE +  ((*cache).Size / 4)) & (0xffffffff - ((*cache).Size / 2) - 1);
  /*pseudoDATE1 records the last reset of the second tag bit column*/
  
  /*chosing among "old" cache lines*/
  for(i=0; i<(*cache).Number_of_Bank; i++){
    Age[i] = (((*cache).Cache_DATE[AdBank[i]] < pseudoDATE) & (((*cache).Cache_DATE[AdBank[i]] + ((*cache).Size / 4)) < pseudoDATE1));
    /* are the two tag bits  reset*/
  }

  for(i=Start; i<(*cache).Number_of_Bank; i++){
    if (Age[i]) set=i;
  }
  for(i=0; i<Start; i++){
    if (Age[i]) set=i;
  }
  
  /*chosing among "young" cache lines*/
  if (set == -1){
    for(i=0; i<(*cache).Number_of_Bank; i++){
      Age[i] = (((*cache).Cache_DATE[AdBank[i]] < pseudoDATE) |  ((((*cache).Cache_DATE[AdBank[i]] + ((*cache).Size / 4))) < pseudoDATE1));
      /* is there one tag bit reset*/
    }
    for(i=Start; i<(*cache).Number_of_Bank; i++){
      if (Age[i]) set=i;
    }
    for(i=0; i<Start; i++){
      if (Age[i]) set=i;
    }
  }
  
  /* chosing  among  "very young" cache lines*/
  if (set== -1) { 
    set = Start;
  }
  
  for (i = 0; i < (*cache).Number_of_Bank; i++){
    if ((*cache).Cache[AdBank[i]] == FreeAddr) {
      set = i;
    }
  }
  (*cache).Cache_DATE[AdBank[set]] = (*cache).DATE;
  /*the new cache line is very young*/
  (*cache).DATE++;
#ifdef SIML2
  EXCLUDE= (*cache).Cache[AdBank[set]];
#endif
  (*cache).Cache[AdBank[set]] = addr;
  
}



int Is_In_CacheSKEWENRU( int addr, CACHEINT *cache, int memtype )
{
  
  int	i;
  
  Mapfct( addr, cache, memtype );
  
  for (i = 0; i < (*cache).Number_of_Bank; i++) {
    if ((*cache).Cache[AdBank[i]] == addr) {
      if ((*cache).Cache_DATE[AdBank[i]] < ((*cache).DATE & (0xffffffff - ((*cache).Size / 2 - 1)))){
	/*do we have touched an "old" cache line*/
	(*cache).DATE++;
      }
      (*cache).Cache_DATE[AdBank[i]] = (*cache).DATE;
      return (1);
    }
  }
  return (0);
}



