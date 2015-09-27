/* Copyright Andre Seznec, Jerome Hedouin, INRIA-IRISA (1997) */



#include "cacheLibrary.h"
 
void Mapfct1(int, CACHEINT*, int);
void initscramble(void);
int scramb(int, int);
int scramble(int X,int N);
int BITR(int, int);


/*Caution: only 4 banks are foreseen here, these  mapping functions can be easily extended to more banks*/
void Mapfct1( int addr, CACHEINT *cache, int  memtype )
{
        
  int  LOGBank, T, X, X1, X2, X3, X4;


  LOGBank = (*cache).Logbank;
  T = (*cache).Size_Bank;
  X1 = addr & (T - 1);
  X1=scramble(X1,LOGBank);
  X = BITR(LOGBank, (addr >> LOGBank) & (T - 1));
  X2 = ((X1 * 2) & (T - 1)) + ((2 * X1) >> LOGBank);
  X3 = ((X2 * 2) & (T - 1)) + ((2 * X2) >> LOGBank);
  X4 = ((X3 * 2) & (T - 1)) + ((2 * X3) >> LOGBank);
  AdBank[0] = X ^ X1;
  AdBank[1] = (X ^ X2) +     (*cache).Size_Bank;
  AdBank[2] = (X ^ X3) + 2 * (*cache).Size_Bank;
  AdBank[3] = (X ^ X4) + 3 * (*cache).Size_Bank;
  
}  



/* functions for computing functions */

void initMappingfunction(void)
{
  int i,j;
  
  for (i=0;i<16;i++){
    for (j=0;j<DEUP[i];j++){
      SCRAMBLE[j][i]=scramb(j,i);
    }
  }
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






