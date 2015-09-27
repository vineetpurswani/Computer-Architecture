CFLAGS = -O

all:  CACHESKEW  CACHESKEWL2

CACHESKEW :  mainess.c cacheLibrary.h display.c initcache.c Sim.c mappingfct1.c mappingfct2.c mappingfct3.c SetAsso.c SkewLRU.c SkewNRU.c SkewUNRU.c SkewENRU.c SkewRAND.c SkewUSEFUL.c SkewPseudoLRU.c
	gcc -ansi  -o $@ mainess.c display.c initcache.c Sim.c mappingfct1.c mappingfct2.c mappingfct3.c SetAsso.c SkewLRU.c SkewNRU.c SkewUNRU.c SkewENRU.c SkewRAND.c SkewUSEFUL.c SkewPseudoLRU.c


CACHESKEWL2 : mainess.c cacheLibrary.h display.c initcache.c Sim.c mappingfct1.c mappingfct2.c mappingfct3.c SetAsso.c SkewLRU.c SkewNRU.c SkewUNRU.c SkewENRU.c SkewRAND.c SkewUSEFUL.c SkewPseudoLRU.c
	gcc -ansi -DSIML2 -o $@ mainess.c display.c initcache.c Sim.c mappingfct1.c mappingfct2.c mappingfct3.c SetAsso.c SkewLRU.c SkewNRU.c SkewUNRU.c SkewENRU.c SkewRAND.c SkewUSEFUL.c SkewPseudoLRU.c


#CACHESKEWL2 assumes a L2 cache with inclusion property
# for the moment L1 cannot be configured

