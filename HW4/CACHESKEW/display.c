/* Copyright Andre Seznec, Jerome Hedouin, INRIA-IRISA (1997) */

#include "cacheLibrary.h"
 
void DUMPRES(void);
void printCACHE( FILE* , CACHE* );

 
void DUMPRES(void)
{
  FILE           *fx;
  int             i, Taille;
    
  fx = fopen(ResFile, "a");

  fprintf(fx, "#instructions : %d  || #data : %d \n", nbinst, nbref - nbinst);

    fprintf(fx, "\tnbinst: number of instructions\n");

#ifdef SIML2
    fprintf(fx, "\tL2Miss: number of miss of the second level cache\n");
    fprintf(fx,"\tthe first level of cache is fixed to 8K, 16bytes line size with a Direct Map replacement policy.");

#else
    fprintf(fx, "\tdataMiss: number of miss of the data cache\n\tinstMiss:number of miss of the instruction cache\n");

#endif

  fprintf(fx, "\n\n");
  Taille = CacheSize;
  for (i = 0; i < 3; i++) {

    fprintf(fx, "\n# %d \n", Taille);

    fprintf(fx, "strategy\tNbBank\tnbinst\t");

#ifdef SIML2
    fprintf(fx, "L2Miss\t100*L2Miss/nbinst\n\n");
#else
    fprintf(fx, "dataMiss\tinstMiss\t100*nbMiss/nbinst\t\n\n");

#endif


    fprintf(fx, "DirectMap \t1\t");    printCACHE(fx, &DM[i]);

    fprintf(fx, "AssocLRU  \t2\t");    printCACHE(fx, &TWOASSOCLRU[i]);

    fprintf(fx, "AssocLRU  \t4\t");    printCACHE(fx, &FOURASSOCLRU[i]);

    fprintf(fx, "AssocLRU  \t8\t");    printCACHE(fx, &EIGHTASSOCLRU[i]);

    fprintf(fx, "AssocLRU  \t16\t");   printCACHE(fx, &SIXTEENASSOCLRU[i]);

    fprintf(fx, "AssocRAND \t2\t");   printCACHE(fx, &TWOASSOCRAND[i]);

    fprintf(fx, "AssocRAND \t4\t");   printCACHE(fx, &FOURASSOCRAND[i]);

    fprintf(fx, "AssocRAND \t8\t");   printCACHE(fx, &EIGHTASSOCRAND[i]);

    fprintf(fx, "AssocRAND \t16\t");  printCACHE(fx, &SIXTEENASSOCRAND[i]);

    fprintf(fx, "SkewLRU   \t2\t");   printCACHE(fx, &TWOSKEWLRU[i]);

    fprintf(fx, "SkewRAND  \t2\t");   printCACHE(fx, &TWOSKEWRAND[i]);

    fprintf(fx, "SkewPseudoLRU\t2\t");   printCACHE(fx, &TWOSKEWpseudoLRU[i]);

    fprintf(fx, "SkewNRU   \t2\t");   printCACHE(fx, &TWOSKEWNRU[i]);

    fprintf(fx, "SkewUNRU  \t2\t");   printCACHE(fx, &TWOSKEWUNRU[i]);

    fprintf(fx, "SkewENRU  \t2\t");   printCACHE(fx, &TWOSKEWENRU[i]);

    fprintf(fx, "SkewUseful\t2\t");   printCACHE(fx, &TWOSKEWUseful[i]);

    fprintf(fx, "SkewLRU   \t4\t");   printCACHE(fx, &FOURSKEWLRU[i]);

    fprintf(fx, "SkewRAND  \t4\t");   printCACHE(fx, &FOURSKEWRAND[i]);

    fprintf(fx, "SkewENRU  \t4\t");  printCACHE(fx, &FOURSKEWENRU[i]);


    Taille = Taille * 2;
    fprintf(fx, "\n");
    
  }
  
  fclose(fx);
  
}



void printCACHE(FILE *fx, CACHE *cache)
{
 fprintf(fx, "%d\t",nbinst);
 
#ifdef SIML2
    fprintf(fx, "%d\t%f",(*cache).cacheL2.Miss,100. * (float) ((*cache).cacheL2.Miss) / ((float) nbinst));

#else
  fprintf(fx, "%d\t\t%d\t\t%f\t", 
	  (*cache).cachedata.Miss, 
	  (*cache).cacheinst.Miss, 
	  100.* ((float) ((*cache).cachedata.Miss + (*cache).cacheinst.Miss)) / ((float) nbinst));

#endif
    fprintf(fx, "\n");
  
}





