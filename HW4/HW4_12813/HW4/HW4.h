#define  NbSimul 1 /* Number of Simulations for each cache policies*/

#ifdef _WIN32
#define random rand
#endif

#include <stdio.h>
#include <malloc.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
  

typedef struct CACHEINT{
  
  int           Hit;
  int           Miss;
  int           Size;    /* Size = number of lines of the cache */
  int           Logbank; /* log base 2 of the bank size */
  int           Number_of_Bank; 
  int           Size_Bank;/* size in line of a bank */
  int	        DATE;    /* date of the last access to the cache */
  int           ACCESS;  /* number of access to the cache */
  
  unsigned int	*Cache;  /* array that contains the memory adress of the block contained in the cache line indexed */  
  int	*Cache_DATE; /* this array contains the date of the last access to the line indexed */ 
  int	*Cache_Prio; /* this variable is used to establish the priority between the blocks for some replacement policies */
  void (*Allocate) (int , struct CACHEINT*, int);
  int (*Is_In_Cache) (int , struct CACHEINT*, int);
}CACHEINT;
  

typedef struct {
  CACHEINT	cacheL2;
  CACHEINT	cachedata;
  CACHEINT	cacheinst;  
}CACHE;

#define FreeAddr -1

#define mem_inst 2 
#define mem_write 1
#define mem_read 0


int	CacheSize; /* size in Bytes of the cache */
int	LOGLINE;/* log base 2 of line size */
void (*Mapfct) (int ,CACHEINT*, int);/* pointer to the mapping function */
char ResFile[] = "HW4.out";/* pointer to the result file */
unsigned int	array_ADDR[16384];/* input-buffer that contains the address of the memory-access */  
unsigned int	MEMTYPE[16384];/* input-buffer that contains the type of the memory-access */
int	Index = 0; /* Index is used by the input-buffers array_ADDR and MEMTYPE */
int	nbref = 0;/* number of access to the cache */
int	nbinst = 0;/* number of access to the instruction cache */
int	lastinst;  /* lastinst contains the adress of the last used instruction */
int	lastdata; /* lastdata contains the adress of the last accessed data */
int *	AdBank;	/* array that contains the address of mapping in each bank of the Skewed Associative caches */  
int	DEUP[16] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768 };
int    	SCRAMBLE[32768][16]; 

CACHE	DM[NbSimul],
	TWOSKEWpseudoLRU[NbSimul],
	FOURASSOCLRU[NbSimul], 
	FOURASSOCRAND[NbSimul]; 

void AllocateDM( int addr, CACHEINT *cache, int memtype )
{
  int  X,T;
  T = (*cache).Size_Bank;
  X = (addr & (T-1));
#ifdef SIML2
  EXCLUDE= (*cache).Cache[X];
#endif

  (*cache).Cache[X] = addr;
}

int Is_In_CacheDM(int addr, CACHEINT *cache, int memtype)
{
  int		X,T;
  T = (*cache).Size_Bank;
  X = (addr & (T-1));
  
  if ((*cache).Cache[X] == (unsigned)addr) {
    return (1);
  };
  return (0);
}


void AllocateSKEWpseudoLRU(int addr, CACHEINT *cache, int memtype )
{
  int		i, set;
  
  Mapfct( addr, cache, memtype );
 
  set = (*cache).Cache_Prio[AdBank[0]];
  for (i = 0; i < (*cache).Number_of_Bank; i++){
    if ((*cache).Cache[AdBank[i]] == (unsigned)FreeAddr) {
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
    if ((*cache).Cache[AdBank[i]] == (unsigned)addr) {
      (*cache).Cache_Prio[AdBank[0]] = 1 - i;
      return (1);
    }
  }
  return (0);
  
}



void AllocateRAND(int addr,CACHEINT *cache, int memtype)
{
  int             Choose_Bank;
  int             T, X;

  T = (*cache).Size_Bank;
  X = (addr & (T - 1)) * (*cache).Number_of_Bank;
  Choose_Bank = random() % (*cache).Number_of_Bank;
  
  X = X + Choose_Bank;
#ifdef SIML2
  EXCLUDE= (*cache).Cache[X];
#endif
  (*cache).Cache[X] = addr;
}



void AllocateLRU( int addr, CACHEINT *cache, int memtype )
{
  int             min;
  int             Choose_Bank;
  int             T, X, i;
  
  T = (*cache).Size_Bank;
  X = (addr & (T - 1)) * (*cache).Number_of_Bank;
  Choose_Bank = 0;
  min = (*cache).Cache_DATE[X];
  for (i = 1; i < (*cache).Number_of_Bank; i++){
    if ((*cache).Cache_DATE[X + i] < min) {
      Choose_Bank = i;
      min = (*cache).Cache_DATE[X + i];
    }
  }
  for (i = 0; i < (*cache).Number_of_Bank; i++){
    if ((*cache).Cache[X + i] == (unsigned)-1) {
      Choose_Bank = i;
    }
  }
  X = X + Choose_Bank;
  
#ifdef SIML2
  EXCLUDE= (*cache).Cache[X];
#endif
  (*cache).Cache[X] = addr;
  (*cache).Cache_DATE[X] = (*cache).ACCESS;
  
}



int Is_In_CacheASSOC( int addr, CACHEINT *cache, int memtype )
{
  /* set-associative */
  
  int           i, X, T;
  int		inter;
  
  
  T = (*cache).Size_Bank;
  X = (addr & (T - 1)) * (*cache).Number_of_Bank;
  
  if ((*cache).Cache[X] == (unsigned)addr) return(1);
  
  for (i = 1; i < (*cache).Number_of_Bank; i++){
    if ((*cache).Cache[X + i] == (unsigned)addr) {
      (*cache).Cache_DATE[X + i] = (*cache).Cache_DATE[X];
      (*cache).Cache_DATE[X]= (*cache).ACCESS;
      inter = (*cache).Cache[X];
      (*cache).Cache[X]= (*cache).Cache[X + i];
      (*cache).Cache[X + i]= inter;
      /* in order to improve simulations rate most recenly used data is swapped with the first accessed data */
      
      return (1);
    }
  }
  return (0);
}


int scramb(int x, int y){
  int inter1,inter2,i;
  int X;
  
  X= x;
  inter1 =0;
  
  for (i=0; i< y; i +=2){
    inter1+=DEUP[i>>1] * (X & 0x01); 
    X =X >>2;
  }
  inter2 =0;
  X= x>>1;
  for (i=1; i< y; i +=2){
    inter2+=DEUP[i>>1] * (X & 0x01); 
    X =X >>2;
  }
  
  inter1 = inter2 + (inter1 << (y/2));
  if (inter1>=DEUP[y]) printf("problem x %d y %d inter1 %d\n",x,y,inter1);
  return(inter1);
  
}

void initMappingfunction(void)
{
  int i,j;
  
  for (i=0;i<16;i++){
    for (j=0;j<DEUP[i];j++){
      SCRAMBLE[j][i]=scramb(j,i);
    }
  }
}


int scramble(int X,int N)
{
  if((X < 32768)&&(N < 16)){
    return (SCRAMBLE[X][N]);
  }else{
    printf("\nERROR: bad access to the matrix SCRAMBLE\n");
    exit(0);
  }
}

int BITR(int A, int B)
{
  int   i, X;
  B= scramble(B,A);
  X=0;
  for (i = 1; i <= A; i++) {
    X = X + (DEUP[A - i] * (B & 0x01));
    B = B >> 1;
  }
  return (X);

}

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

char* allocation(int size)
{
  char           *pt;
  pt = (char *) malloc(sizeof(int) * size);
  if (!pt) {
    fprintf(stderr, "Not enough Memory\n");
    exit(1);
  }
  return (pt);
}

void INITCACHEINT(CACHEINT *cache, int NbLines, int NbBank, void (*Allo) (int a, CACHEINT *b, int c), int (*Is_In) (int a, CACHEINT *b, int c))
{
  int             i;
  int             t;
  
  (*cache).Size = NbLines;
  (*cache).Number_of_Bank = NbBank;
  (*cache).Size_Bank = NbLines / NbBank;

  (*cache).Logbank = 0;
  t = 1;
  while (t < (*cache).Size_Bank) {
    (*cache).Logbank++;
    t = t * 2;
  }
  
  (*cache).Cache = (unsigned int *) allocation(NbLines + 27);
  (*cache).Cache_DATE = (int *) allocation(NbLines + 39);
  (*cache).Cache_Prio = (int *) allocation(2 * NbLines + 67);
  
  for (i = 0; i < NbLines; i++) {
    (*cache).Cache[i] = FreeAddr;
    (*cache).Cache_DATE[i] = -1;
    (*cache).Cache_Prio[i] = 0;
  }
  (*cache).Allocate = (void (*)(int, CACHEINT*, int)) Allo;
  (*cache).Is_In_Cache = (int (*)(int, CACHEINT*, int)) Is_In;
}

void initCACHE(CACHE *cache, int NbLines, int NbBank, void (*Allo) (int a, CACHEINT *b, int c), int (*Is_In) (int a, CACHEINT *b, int c))
{
#ifdef SIML2
	/* First level is fixed to 8K, 16 bytes line size */
  INITCACHEINT(&(*cache).cacheL2, NbLines, NbBank, Allo, Is_In);
  INITCACHEINT(&(*cache).cacheinst, 512, 1, AllocateDM, Is_In_CacheDM);
  INITCACHEINT(&(*cache).cachedata, 512, 1, AllocateDM, Is_In_CacheDM);
#else
  INITCACHEINT(&(*cache).cacheinst, NbLines, NbBank, Allo, Is_In);
  INITCACHEINT(&(*cache).cachedata, NbLines, NbBank, Allo, Is_In);

#endif
}

void INITCACHE()
{

  int        i;
  int		 NbLines;	/* size in line of the caches */
  int        LINE;		/* size in bytes of a cache line */
  CacheSize = 8192;	/* size in bytes of the caches */
  LOGLINE = 4;	/* log base 2 of line size */

  Mapfct = Mapfct3;

  LINE = 1;
  for (i = 0; i < LOGLINE; i++){
    LINE = 2 * LINE;
  }
  NbLines = CacheSize / LINE;


  initMappingfunction();
  AdBank = (int*)malloc(sizeof(int)*4);


  for (i = 0; i < NbSimul; i++) {
    initCACHE(&DM[i], NbLines, 1, AllocateDM, Is_In_CacheDM);
    initCACHE(&FOURASSOCLRU[i], NbLines, 4, AllocateLRU, Is_In_CacheASSOC);
    initCACHE(&FOURASSOCRAND[i], NbLines, 4, AllocateRAND, Is_In_CacheASSOC);
    initCACHE(&TWOSKEWpseudoLRU[i], NbLines, 2, AllocateSKEWpseudoLRU, Is_In_CacheSKEWpseudoLRU);
    
    NbLines = NbLines * 2;
  }
}

void Simul(CACHE *cache, int add, int memtype)
{
  CACHEINT *	cacheint;

  if (memtype == mem_inst) {
    cacheint = &(cache->cacheinst);
  } else {
    cacheint = &(cache->cachedata);
  }

  (*cacheint).ACCESS++;
  if ((*cacheint).Is_In_Cache(add, (CACHEINT *)cacheint, memtype)) {
    cacheint->Hit++;
  } else {
    cacheint->Miss++;
    ((*cacheint).Allocate(add, (CACHEINT *)cacheint, memtype));
  }
}

void SIM(void)
{
  int i,K;
  // printf("[Simulating] %d data accesses in the buffer.\n", Index);
  
  for (i = 0; i < NbSimul; i++) {
    
    for (K = 0; K < Index; K++)
      Simul(&DM[i], array_ADDR[K], MEMTYPE[K]);
    
    for (K = 0; K < Index; K++)
      Simul(&FOURASSOCLRU[i], array_ADDR[K], MEMTYPE[K]);
    
    for (K = 0; K < Index; K++)
      Simul(&FOURASSOCRAND[i], array_ADDR[K], MEMTYPE[K]);

    for (K = 0; K < Index; K++)
      Simul(&TWOSKEWpseudoLRU[i], array_ADDR[K], MEMTYPE[K]);    
  }
  Index = 0;
}

void memory_access(int memtype, int addr){

#ifdef SIML2
  addr = addr >> 4;
  /* line size on L1 cache*/
#else
  addr = addr >> LOGLINE;
#endif
  /*get the line number*/
  nbref++;
  if ((nbref & 0xffff) == 0){
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
  array_ADDR[Index] = addr;
  /*Buffered before simulation: in order to preserve locality during cache simulation*/
  Index++;
  if (Index != 16384)
    return;

  SIM();
  /*let us run the simulations*/
}

void printCACHE(FILE *fx, CACHE *cache){
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
 
void DUMPRES(void){
  FILE           *fx;
  int             i, Taille;
    
  fx = fopen(ResFile, "w");

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
  for (i = 0; i < NbSimul; i++) {

    fprintf(fx, "\n# %d \n", Taille);

    fprintf(fx, "strategy\tNbBank\tnbinst\t");

#ifdef SIML2
    fprintf(fx, "L2Miss\t100*L2Miss/nbinst\n\n");
#else
    fprintf(fx, "dataMiss\tinstMiss\t100*nbMiss/nbinst\t\n\n");

#endif


    fprintf(fx, "DirectMap \t1\t");    printCACHE(fx, &DM[i]);

    fprintf(fx, "AssocLRU  \t4\t");    printCACHE(fx, &FOURASSOCLRU[i]);

    fprintf(fx, "AssocRAND \t4\t");   printCACHE(fx, &FOURASSOCRAND[i]);

    fprintf(fx, "SkewPseudoLRU\t2\t");   printCACHE(fx, &TWOSKEWpseudoLRU[i]);

    Taille = Taille * 2;
    fprintf(fx, "\n");
    
  }
  
  fclose(fx);
  
}

