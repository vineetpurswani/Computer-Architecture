/* Copyright Andre Seznec, Jerome Hedouin, INRIA-IRISA (1997) */

/* this file contains the mapping fonctions used for experiments described in the reference "Skewed associativity improves program performance and enhances predictability" */

#include "cacheLibrary.h"
 
void  Mapfct3(int, CACHEINT*, int);


void  Mapfct3( int addr, CACHEINT *cache, int  memtype )
{
  int    A1, A2;
  int    mask, Low, i;
  
  
  mask = (*cache).Size_Bank - 1;
  
  A1 = addr & mask;
  A2 = (addr >> (*cache).Logbank) & mask;
  for(i=0;i<(*cache).Number_of_Bank;i++){
    AdBank[i] = (A1 ^ A2) + (i * (*cache).Size_Bank);
    Low = A1 >> ((*cache).Logbank-1);
    A1 = ((2*A1) + Low) & mask;
  }

}



