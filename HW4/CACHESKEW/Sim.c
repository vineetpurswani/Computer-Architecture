/* Copyright Andre Seznec, Jerome Hedouin, INRIA-IRISA (1997) */


#include "cacheLibrary.h"

void SIM(void);
void Simul(CACHE *, int, int );
void memory_access( int, int );
void enforceinclusion( CACHE*);



void SIM(void)
{
  int i,K;
  
  for (i = 0; i < NbSimul; i++) {
    
    for (K = 0; K < Index; K++)
      Simul(&DM[i], ADDR[K], MEMTYPE[K]);
    
    for (K = 0; K < Index; K++)
      Simul(&TWOASSOCLRU[i], ADDR[K], MEMTYPE[K]);
    
    for (K = 0; K < Index; K++)
      Simul(&FOURASSOCLRU[i], ADDR[K], MEMTYPE[K]);
    
    for (K = 0; K < Index; K++)
      Simul(&EIGHTASSOCLRU[i], ADDR[K], MEMTYPE[K]);
    
    for (K = 0; K < Index; K++)
      Simul(&SIXTEENASSOCLRU[i], ADDR[K], MEMTYPE[K]);
    
    for (K = 0; K < Index; K++)
      Simul(&TWOASSOCRAND[i], ADDR[K], MEMTYPE[K]);
    
    for (K = 0; K < Index; K++)
      Simul(&FOURASSOCRAND[i], ADDR[K], MEMTYPE[K]);
    
    for (K = 0; K < Index; K++)
      Simul(&EIGHTASSOCRAND[i], ADDR[K], MEMTYPE[K]);
    
    for (K = 0; K < Index; K++)
      Simul(&SIXTEENASSOCRAND[i], ADDR[K], MEMTYPE[K]);
    
    for (K = 0; K < Index; K++)
      Simul(&TWOSKEWLRU[i], ADDR[K], MEMTYPE[K]);
    
    for (K = 0; K < Index; K++)
      Simul(&TWOSKEWRAND[i], ADDR[K], MEMTYPE[K]);
    
    for (K = 0; K < Index; K++)
      Simul(&TWOSKEWpseudoLRU[i], ADDR[K], MEMTYPE[K]);
    
    for (K = 0; K < Index; K++)
      Simul(&TWOSKEWNRU[i], ADDR[K], MEMTYPE[K]);
    
    for (K = 0; K < Index; K++)
      Simul(&TWOSKEWUNRU[i], ADDR[K], MEMTYPE[K]);
    
    for (K = 0; K < Index; K++)
      Simul(&TWOSKEWENRU[i], ADDR[K], MEMTYPE[K]);
    
    for (K = 0; K < Index; K++)
      Simul(&TWOSKEWUseful[i], ADDR[K], MEMTYPE[K]);
    
    for (K = 0; K < Index; K++)
      Simul(&FOURSKEWLRU[i], ADDR[K], MEMTYPE[K]);
    
    for (K = 0; K < Index; K++)
      Simul(&FOURSKEWRAND[i], ADDR[K], MEMTYPE[K]);
    
    for (K = 0; K < Index; K++)
      Simul(&FOURSKEWENRU[i], ADDR[K], MEMTYPE[K]);
    
  }
  Index = 0;
}



/****** the following part of this file is not supposed to be modified ******/


void Simul(CACHE *cache, int add, int memtype)
{
  
  CACHEINT *	cacheint;


  if (memtype == mem_inst) {
    cacheint = &(cache->cacheinst);
  } else {
    cacheint = &(cache->cachedata);
  }

  (*cacheint).ACCESS++;
  if ((*cacheint).Is_In_Cache(add, cacheint, memtype)) {
    cacheint->Hit++;
  } else {
    cacheint->Miss++;
    ((*cacheint).Allocate(add, cacheint, memtype));
#ifdef SIML2
    cacheint = &(cache->cacheL2);
    (*cacheint).ACCESS++;
    add = add >> (LOGLINE - 4);
    if ((*cacheint).Is_In_Cache(add, cacheint, memtype)) {
      cacheint->Hit++;
    } else {
      cacheint->Miss++;
      ((*cacheint).Allocate(add, cacheint, memtype));
      enforceinclusion(cache);
    }
#endif
  }
}



#ifdef SIML2
void enforceinclusion(CACHE* cache)
{
  /* this procedure assumes that a 8K direct-mapped split L1 cache with
   16 byte cache line is used   */
  int             i;
  EXCLUDE = EXCLUDE << LOGLINE - 4;
  for (i = 0; i < (1 << LOGLINE - 4) - 1; i++) {
    if ((*cache).cacheinst.Cache[EXCLUDE & 511] == EXCLUDE)
      (*cache).cacheinst.Cache[EXCLUDE & 511] = -1;
    if ((*cache).cachedata.Cache[EXCLUDE & 511] == EXCLUDE)
      (*cache).cachedata.Cache[EXCLUDE & 511] = -1;
    EXCLUDE++;
  }
}
#endif



/* memory_access is called for each access*/
void memory_access(int memtype, int addr)

{

#ifdef SIML2
  addr = addr >> 4;
  /* line size on L1 cache*/
#else
  addr = addr >> LOGLINE;
#endif
  /*get the line number*/
  nbref++;
  if (nbref & 0xffff == 0){
    printf(" (%d) ", nbref);
  }
  if (memtype == mem_inst){
    nbinst++;
    if (lastinst == addr) return;
    /* if the instruction block is the same as the last one
       then do not simulate it*/
    lastinst = addr;
  } else {
    if (lastdata == addr) return;
    /* if the data block is the same as the last one
       then do not simulate it*/
    lastdata = addr;
  }
  
  MEMTYPE[Index] = memtype;
  ADDR[Index] = addr;
  /*Buffered before simulation: in order to preserve locality during cache simulation*/
  Index++;
  if (Index != 16384)
    return;
  SIM();
  /*let us run the simulations*/
}







