/*Copyright Andre Seznec,Jerome Hedouin, INRIA-IRISA (1997)*/

#define _main_
#include <stdio.h>
#include "cacheLibrary.h"
 
main(int argc, char** argv)
{
  int	memtype;
  int	addr;
  

  INITCACHE(argc, &argv[0]);
	
  while (scanf("%d %x", &memtype, &addr)>=0) {
    memory_access(memtype, addr);
  };
  SIM();/* useful for the end of the simulation file */ 

  DUMPRES();
  
}

 
