/*********************************************************************************************
 *
 * An implementation of a 32-bit MIPS execution engine.
 * The driver (in this case the instruction fetcher) is not included.
 *
 * Used primarily for teaching purpose in a senior undergraduate computer architecture course
 *
 * Mainak Chaudhuri
 * Department of Computer Science and Engineering
 * Indian Institute of Technology
 * Kanpur 208016
 * INDIA
 *
 *********************************************************************************************/

#include <math.h>
#include <assert.h>
#include <stdio.h>

// Define instruction encoding
typedef union {
  unsigned int data;
#if BYTE_ORDER == BIG_ENDIAN
  struct {
    unsigned int op:6;
    unsigned int rs:5;
    unsigned int rt:5;
    unsigned int imm:16;
  } imm;		// Immediate mode
  struct {
    unsigned int op:6;
    unsigned int tgt:26;
  } tgt;		// Direct jump addressing
  struct {
    unsigned int op:6;
    unsigned int rs:5;
    unsigned int rt:5;
    unsigned int rd:5;
    unsigned int sa:5;
    unsigned int func:6;
  } reg;		// Register-register instructions
  struct {
    unsigned int op:6;
    unsigned int fmt:5;
    unsigned int ft:5;
    unsigned int fs:5;
    unsigned int fd:5;
    unsigned int func:6;
  } freg;		// Floating-point instruction format
#else
  struct {
    unsigned int imm:16;
    unsigned int rt:5;
    unsigned int rs:5;
    unsigned int op:6;
  } imm;		// Immediate mode
  struct {
    unsigned int tgt:26;
    unsigned int op:6;
  } tgt;		// Direct jump addressing
  struct {
    unsigned int func:6;
    unsigned int sa:5;
    unsigned int rd:5;
    unsigned int rt:5;
    unsigned int rs:5;
    unsigned int op:6;
  } reg;		// Register-register instructions
  struct {
    unsigned int func:6;
    unsigned int fd:5;
    unsigned int fs:5;
    unsigned int ft:5;
    unsigned int fmt:5;
    unsigned int op:6;
  } freg;		// Floating-point instruction format
#endif
} MipsInsn;

// Define floating-point execution related macros
#if BYTE_ORDER == LITTLE_ENDIAN

#define FP_TWIDDLE 0

#else

#define FP_TWIDDLE 1

#endif

#define FP_LONG(idx)  fpr[(idx)>>1].l[FP_TWIDDLE^((idx)&1)]
#define FP_FLOAT(idx)  fpr[(idx)>>1].f[FP_TWIDDLE^((idx)&1)]
#define FP_DBL(idx)  fpr[(idx)>>1].d

#define s_isNaN(x) ((((FP_LONG(x) >> 23) & 0xff) == 0xff) && \
			((FP_LONG(x) & 0x7fffff) != 0))

#define d_isNaN(x) ((((FP_LONG((x)|1) >> 20) & 0x7ff) == 0x7ff) && \
		((FP_LONG(x) != 0 || ((FP_LONG((x)|1) & 0xfffff) != 0))) )


#define FMT_S 0x10
#define FMT_D 0x11
#define FMT_W 0x14
#define FMT_L 0x15

// Define a pair of floating-point registers as a union
typedef union {
   unsigned int l[2];
   float f[2];
   double d;
} FPreg;

// Define floating-point rounding modes
#if defined(LINUX) || defined(__linux__)
#include <fpu_control.h>
#include <fenv.h>
#define FP_RN  FE_TONEAREST
#define FP_RM  FE_DOWNWARD
#define FP_RP  FE_UPWARD
#define FP_RZ  FE_TOWARDZERO
#define SETRMODE(x) \
   { \
      fpu_control_t status; \
      _FPU_GETCW(status);   \
      status &= ~0xC00;     \
      status |= ((x) & 0x3) << 10; \
      _FPU_SETCW(status);  \
   }
#define fpsetmask(x) \
   { \
      fpu_control_t status; \
      _FPU_GETCW(status);   \
      status &= ~0x3f;      \
      status |= (~(x) & 0x3f); \
      _FPU_SETCW(status);   \
}
#else
#include <floatingpoint.h>
#define SETRMODE(x) fpsetround(rmode[x])
#endif

static int rmode[] = {
  FP_RN,
  FP_RM,
  FP_RP,
  FP_RZ
};

#define FLT_EPSILON 1.19209290E-07F
#define DBL_EPSILON 2.2204460492503131E-16

// Some useful macros

#define SIGN_EXTEND_BYTE(x)  do { x <<= 24; x >>= 24; } while (0)
#define SIGN_EXTEND_IMM(x)   do { x <<= 16; x >>= 16; } while (0)

#define BRANCH_SETUP    do { a1 = i.imm.imm;  a1 <<= 16; a1 >>= 14; (*bd) = 1; (*btgt) = (unsigned)((signed)pc+a1+4); } while(0)

#define JMP_SETUP do { (*btgt) = ((pc+4) & 0xf0000000) | ((i.tgt.tgt)<<2); (*bd) = 1; (*btaken) = 1; } while (0)

#define JMP2_SETUP do { (*btgt) = gpr[i.reg.rs]; (*bd) = 1; (*btaken) = 1; } while (0)

#define WRITEREG(reg,val)  do { gpr[reg] = (val); } while (0)

// Memory interface
// Implementation not shown

unsigned int BigEndianGetWord (unsigned long long addr)
{
   unsigned int data;

   // Read word from memory
   // Implementation not shown

   return data;
}

unsigned int BigEndianGetHalfWord (unsigned long long addr)
{
   unsigned int data;

   // Read half word from memory
   // Implementation not shown

   return data;
}

unsigned int BigEndianGetByte (unsigned long long addr)
{
   unsigned int data;

   // Read byte from memory
   // Implementation not shown

   return data;
}

void BigEndianSetWord (unsigned long long addr, unsigned int data)
{
   // Write word to memory
   // Implementation not shown
}

void BigEndianSetHalfWord (unsigned long long addr, unsigned int data)
{
   // Write half word to memory
   // Implementation not shown
}

void BigEndianSetByte (unsigned long long addr, unsigned int data)
{
   // Write byte to memory
   // Implementation not shown
}

/*------------------------------------------------------------------------
 *
 *  Instruction execution: pass processor state
 *
 *  The instruction fetcher calls this function after fetching each instruction
 *
 *  The program counter updates are not shown here. The fetcher does that
 *  after executing the current instruction.
 *
 *------------------------------------------------------------------------
 */

void Execute (unsigned int ins, unsigned int pc, unsigned int *gpr, unsigned int *btgt, int *bd, int *btaken, unsigned int *hi, unsigned int *lo, FPreg *fpr, int *coc1, unsigned int *fcr)
{
   // Arguemnt list:
   // ins: currently fetched instruction to be executed
   // pc: current program counter
   // gpr: general purpose integer register array
   // btgt: branch target of taken branch, used by fetcher to update program counter.
   // bd: 1 if the next instruction is a branch delay slot. Used by fetcher.
   // btaken: 1 if this instruction resolves into a taken branch. Used by fetcher to update program counter.
   // hi, lo: Hi, Lo registers
   // fpr: floating-point register array
   // coc1: floating-point condition code
   // fcr: floating-point control register

   MipsInsn i;
   signed int a1, a2;
   unsigned int ar1, ar2, s1, s2, r1, r2, t1, t2;
   unsigned long long addr;
   unsigned int val;

   i.data = ins;
   (*bd) = 0;
   (*btaken) = 0;
  
   switch (i.reg.op) {
   case 0:
      // SPECIAL
      switch (i.reg.func) {

      case 0x0f:                        // sync
         // Multiprocessor support
         // Not shown
         break;

      case 0x20:			// add
      case 0x21:			// addu
	 a1 = gpr[i.reg.rs];
	 a2 = gpr[i.reg.rt];
	 WRITEREG (i.reg.rd, (unsigned)(a1+a2)); // ignore overflow
	 break;

      case 0x24:			// and
	 WRITEREG (i.reg.rd, gpr[i.reg.rs] & gpr[i.reg.rt]);
	 break;

      case 0x27:			// nor
	 WRITEREG (i.reg.rd, ~(gpr[i.reg.rs]|gpr[i.reg.rt]));
	 break;

      case 0x25:			// or
	 WRITEREG (i.reg.rd, gpr[i.reg.rs]|gpr[i.reg.rt]);
	 break;

      case 0:			// sll
	 WRITEREG (i.reg.rd, gpr[i.reg.rt] << i.reg.sa);
	 break;

      case 4:			// sllv
	 WRITEREG (i.reg.rd, gpr[i.reg.rt] << (gpr[i.reg.rs]&0x1f));
	 break;

      case 0x2a:			// slt
	 a1 = gpr[i.reg.rs];
	 a2 = gpr[i.reg.rt];
	 if (a1 < a2) {
	    WRITEREG (i.reg.rd, 1);
	 }
	 else {
	    WRITEREG (i.reg.rd, 0);
	 }
	 break;

      case 0x2b:			// sltu
	 if (gpr[i.reg.rs] < gpr[i.reg.rt]) {
	    WRITEREG (i.reg.rd, 1);
	 }
	 else {
	    WRITEREG (i.reg.rd, 0);
	 }
	 break;

      case 0x3:			// sra
	 a2 = gpr[i.reg.rt];
	 a2 >>= i.reg.sa;
	 WRITEREG (i.reg.rd, a2);
	 break;

      case 0x7:			// srav
	 a2 = gpr[i.reg.rt];
	 a2 >>= (gpr[i.reg.rs] & 0x1f);
	 WRITEREG (i.reg.rd, a2);
	 break;

      case 0x2:			// srl
	 WRITEREG (i.reg.rd, gpr[i.reg.rt] >> i.reg.sa);
	 break;

      case 0x6:			// srlv
	 WRITEREG (i.reg.rd, gpr[i.reg.rt] >> (gpr[i.reg.rs] & 0x1f));
	 break;

      case 0x22:			// sub
      case 0x23:			// subu
	 // no overflow check
	 WRITEREG (i.reg.rd, gpr[i.reg.rs] - gpr[i.reg.rt]);
	 break;

      case 0x26:			// xor
	 WRITEREG (i.reg.rd, gpr[i.reg.rs] ^ gpr[i.reg.rt]);
	 break;

      case 0x1a:			// div
	 a1 = gpr[i.reg.rs];
	 a2 = gpr[i.reg.rt];

	 if (a2 != 0) {
	    (*hi) = (unsigned)(a1 % a2);
	    (*lo) = (unsigned)(a1 / a2);
	 }
	 else {
	    (*hi) = 0x7fffffff;
	    (*lo) = 0x7fffffff;
	 }
	 break;

      case 0x1b:			// divu
	 if (gpr[i.reg.rt] != 0) {
	    (*hi) = gpr[i.reg.rs] % gpr[i.reg.rt];
	    (*lo) = gpr[i.reg.rs] / gpr[i.reg.rt];
	 }
	 else {
	    (*hi) = 0xffffffff;
	    (*lo) = 0xffffffff;
	 }
	 break;

      case 0x10:			// mfhi
	 WRITEREG (i.reg.rd, (*hi));
	 break;

      case 0x12:			// mflo
	 WRITEREG (i.reg.rd, (*lo));
	 break;

      case 0x11:			// mthi
	 (*hi) = gpr[i.reg.rs];
	 break;

      case 0x13:			// mtlo
	 (*lo) = gpr[i.reg.rs];
	 break;

      case 0x18:			// mult
	 ar1 = gpr[i.reg.rs];
	 ar2 = gpr[i.reg.rt];
	 s1 = ar1 >> 31; if (s1) ar1 = 0x7fffffff & (~ar1 + 1);
	 s2 = ar2 >> 31; if (s2) ar2 = 0x7fffffff & (~ar2 + 1);

	 t1 = (ar1 & 0xffff) * (ar2 & 0xffff);
	 r1 = t1 & 0xffff;		// bottom 16 bits

	 // compute next set of 16 bits
	 t1 = (ar1 & 0xffff) * (ar2 >> 16) + (t1 >> 16);
	 t2 = (ar2 & 0xffff) * (ar1 >> 16);

	 r1 = r1 | (((t1+t2) & 0xffff) << 16); // bottom 32 bits
	 r2 = (ar1 >> 16) * (ar2 >> 16) + (t1 >> 16) + (t2 >> 16) +
	    (((t1 & 0xffff) + (t2 & 0xffff)) >> 16);

	 if (s1 ^ s2) {
	    r1 = ~r1;
	    r2 = ~r2;
	    r1++;
	    if (r1 == 0)
	       r2++;
	 }
	 (*hi) = r2;
	 (*lo) = r1;
	 break;

      case 0x19:			// multu
	 ar1 = gpr[i.reg.rs];
	 ar2 = gpr[i.reg.rt];

	 t1 = (ar1 & 0xffff) * (ar2 & 0xffff);
	 r1 = t1 & 0xffff;		// bottom 16 bits

	 // compute next set of 16 bits
	 t1 = (ar1 & 0xffff) * (ar2 >> 16) + (t1 >> 16);
	 t2 = (ar2 & 0xffff) * (ar1 >> 16);

	 r1 = r1 | (((t1+t2) & 0xffff) << 16); // bottom 32 bits
	 r2 = (ar1 >> 16) * (ar2 >> 16) + (t1 >> 16) + (t2 >> 16) +
	    (((t1 & 0xffff) + (t2 & 0xffff)) >> 16);
	
	 (*hi) = r2;
	 (*lo) = r1;
	 break;

      case 9:			// jalr
	 JMP2_SETUP;
	 WRITEREG (i.reg.rd, pc + 8);
	 break;

      case 8:			// jr
	 JMP2_SETUP;
	 break;

      case 0xd:			// await/break
	 break;

      case 0xc:			// syscall
         // Pass control to system call emulation layer
         // Implementation not shown
	 break;

      default:
	 printf("Unknown instruction at PC=%#x, Opcode=%u, Function=%u\n", pc, i.reg.op, i.reg.func);
         assert(0);
	 break;
      }
      break;

   case 8:			// addi
   case 9:			// addiu
      // ignore overflow: no exceptions
      a1 = gpr[i.imm.rs];
      a2 = i.imm.imm;
      SIGN_EXTEND_IMM(a2);
      WRITEREG (i.imm.rt, (unsigned)(a1+a2));
      break;

   case 0xc:			// andi
      WRITEREG (i.imm.rt, gpr[i.imm.rs] & i.imm.imm);
      break;

   case 0xf:			// lui
      WRITEREG (i.imm.rt, i.imm.imm << 16);
      break;

   case 0xd:			// ori
      WRITEREG (i.imm.rt, gpr[i.imm.rs] | i.imm.imm);
      break;

   case 0xa:			// slti
      a1 = gpr[i.imm.rs];
      a2 = i.imm.imm;
      SIGN_EXTEND_IMM(a2);
      if (a1 < a2) {
	 WRITEREG (i.imm.rt, 1);
      }
      else {
	 WRITEREG (i.imm.rt, 0);
      }
      break;

   case 0xb:			// sltiu
      a2 = i.imm.imm;
      SIGN_EXTEND_IMM(a2);
      if (gpr[i.imm.rs] < (unsigned)a2) {
	 WRITEREG (i.imm.rt, 1);
      }
      else {
	 WRITEREG (i.imm.rt, 0);
      }
      break;

   case 0xe:			// xori
      WRITEREG (i.imm.rt, gpr[i.imm.rs] ^ i.imm.imm);
      break;

   case 4:			// beq
      BRANCH_SETUP;
      (*btaken) = (gpr[i.imm.rs] == gpr[i.imm.rt]) ? 1 : 0;
      break;

   case 1:
      // REGIMM

      switch (i.reg.rt) {
      case 1:			// bgez
	 BRANCH_SETUP;
	 (*btaken) = !(gpr[i.reg.rs] >> 31);
	 break;

      case 0x11:			// bgezal
	 BRANCH_SETUP;
	 (*btaken) = !(gpr[i.reg.rs] >> 31);
	 WRITEREG (31, pc + 8);
	 break;

      case 0x10:			// bltzal
	 BRANCH_SETUP;
	 (*btaken) = (gpr[i.reg.rs] >> 31);
	 WRITEREG (31, pc + 8);
	 break;

      case 0x0:			// bltz
	 BRANCH_SETUP;
	 (*btaken) = (gpr[i.reg.rs] >> 31);
	 break;

      default:
	 printf ("Unknown instruction at PC=%#x\n", pc);
	 assert(0);
	 break;
      }
      break;

   case 7:			// bgtz
      BRANCH_SETUP;
      a1 = gpr[i.reg.rs];
      (*btaken) = (a1 > 0);
      break;

   case 6:			// blez
      BRANCH_SETUP;
      a1 = gpr[i.reg.rs];
      (*btaken) = (a1 <= 0);
      break;

   case 5:			// bne
      BRANCH_SETUP;
      (*btaken) = (gpr[i.reg.rs] != gpr[i.reg.rt]);
      break;

   case 2:			// j
      JMP_SETUP;
      break;

   case 3:			// jal
      JMP_SETUP;
      WRITEREG (31, pc + 8);
      break;

   case 0x20:			// lb  
      a1 = gpr[i.reg.rs];
      a2 = i.imm.imm;
      SIGN_EXTEND_IMM(a2);
      addr = (unsigned)(a1+a2);
      a1 = BigEndianGetByte(addr);	// Implementation not shown
      SIGN_EXTEND_BYTE(a1);
      WRITEREG (i.reg.rt, a1);
      break;

   case 0x24:			// lbu
      a1 = gpr[i.reg.rs];
      a2 = i.imm.imm;
      SIGN_EXTEND_IMM(a2);
      addr = (unsigned)(a1+a2);
      WRITEREG (i.reg.rt, BigEndianGetByte(addr));
      break;

   case 0x21:			// lh
      a1 = gpr[i.reg.rs];
      a2 = i.imm.imm;
      SIGN_EXTEND_IMM(a2);
      addr = (unsigned)(a1+a2);
      a1 = BigEndianGetHalfWord(addr); 
      SIGN_EXTEND_IMM(a1);
      WRITEREG (i.reg.rt, a1);
      break;

   case 0x25:			// lhu
      a1 = gpr[i.reg.rs];
      a2 = i.imm.imm;
      SIGN_EXTEND_IMM(a2);
      addr = (unsigned)(a1+a2);
      WRITEREG (i.reg.rt, BigEndianGetHalfWord (addr));
      break;

   case 0x22:			// lwl
      a1 = gpr[i.reg.rs];
      a2 = i.imm.imm;
      SIGN_EXTEND_IMM(a2);
      addr = (unsigned)(a1+a2);
      a1 = BigEndianGetWord (addr);
      s1 = (addr & 3) << 3;
      WRITEREG (i.reg.rt, (a1 << s1) | (gpr[i.reg.rt] & ~(~0UL << s1)));
      break;

   case 0x23:			// lw
      a1 = gpr[i.reg.rs];
      a2 = i.imm.imm;
      SIGN_EXTEND_IMM(a2);
      addr = (unsigned)(a1+a2);
      WRITEREG (i.reg.rt, BigEndianGetWord (addr));
      break;

   case 0x26:			// lwr
      a1 = gpr[i.reg.rs];
      a2 = i.imm.imm;
      SIGN_EXTEND_IMM(a2);
      addr = (unsigned)(a1+a2);
      ar1 = BigEndianGetWord (addr);
      s1 = (~addr & 3) << 3;
      WRITEREG (i.reg.rt, (ar1 >> s1) | (gpr[i.reg.rt] & ~(~(unsigned)0 >> s1)));
      break;

   case 0x30:                   // ll
      // Multiprocessor support
      // Implementation not shown
      break;

   case 0x31:			// lwc1
      a1 = gpr[i.reg.rs];
      a2 = i.imm.imm;
      SIGN_EXTEND_IMM(a2);
      addr = (unsigned)(a1+a2);
      FP_LONG(i.reg.rt) = BigEndianGetWord (addr);
      break;

   case 0x33:			//lwc3 for the purpose of prefetch
      // Implementation not shown
      break;

   case 0x38:                   //sc
      // Multiprocessor support
      // Implementation not shown
      break;

   case 0x39:			// swc1
      a1 = gpr[i.reg.rs];
      a2 = i.imm.imm;
      SIGN_EXTEND_IMM(a2);
      addr = (unsigned)(a1+a2);
      BigEndianSetWord (addr, FP_LONG(i.reg.rt));
      break;

   case 0x28:			// sb
      a1 = gpr[i.reg.rs];
      a2 = i.imm.imm;
      SIGN_EXTEND_IMM(a2);
      addr = (unsigned)(a1+a2);
      BigEndianSetByte (addr, gpr[i.reg.rt] & 0xff);
      break;

   case 0x29:			// sh  store half word
      a1 = gpr[i.reg.rs];
      a2 = i.imm.imm;
      SIGN_EXTEND_IMM(a2);
      addr = (unsigned)(a1+a2);
      BigEndianSetHalfWord (addr, gpr[i.reg.rt] & 0xffff);
      break;

   case 0x2a:			// swl
      a1 = gpr[i.reg.rs];
      a2 = i.imm.imm;
      SIGN_EXTEND_IMM(a2);
      addr = (unsigned)(a1+a2);
      ar1 = BigEndianGetWord (addr);
      s1 = (addr & 3) << 3;
      ar1 = (gpr[i.reg.rt] >> s1) | (ar1 & ~(~(unsigned)0 >> s1));
      BigEndianSetWord (addr, ar1);
      break;

   case 0x2b:			// sw
      a1 = gpr[i.reg.rs];
      a2 = i.imm.imm;
      SIGN_EXTEND_IMM(a2);
      addr = (unsigned)(a1+a2);
      BigEndianSetWord (addr, gpr[i.reg.rt]);
      break;

   case 0x2e:			// swr
      a1 = gpr[i.reg.rs];
      a2 = i.imm.imm;
      SIGN_EXTEND_IMM(a2);
      addr = (unsigned)(a1+a2);
      ar1 = BigEndianGetWord (addr);
      s1 = (~addr & 3) << 3;
      ar1 = (gpr[i.reg.rt] << s1) | (ar1 & ~(~0UL << s1));
      BigEndianSetWord (addr, ar1);
      break;

   case 0x11:			// floating-point
      switch (i.freg.fmt) {
      case 0x8:
	 // bc1[tf]
	 BRANCH_SETUP;
	 (*btaken) = (i.freg.ft == 1) ? (*coc1) : !(*coc1);
	 break;

      case 2:			// cfc1
	 if (i.freg.fs == 31) {
	    gpr[i.freg.ft] = (*fcr);	// delay slot of 1...
         }
	 else {
	    printf ("cfc1, fcr %d\n", i.freg.fs);
	 }
	 break;

      case 6:			// ctc1
	 if (i.freg.fs == 31) {
	    (*fcr) = gpr[i.freg.ft];
	    SETRMODE ((*fcr) & 3);
	 }
	 else {
	    printf ("ctc1 fcr=%d\n", i.freg.fs);
	 }
	 (*coc1) = ((*fcr) >> 23) & 1;
	 break;

      case 4:			// mtc1
	 FP_LONG(i.freg.fs) = gpr[i.freg.ft];
	 break;

      case 0:			// mfc1
	 gpr[i.freg.ft] = FP_LONG (i.freg.fs);
	 break;

      case FMT_S:
      case FMT_D:
      case FMT_W:
	 switch (i.freg.func) {
	 case 0: // add
	    if (i.freg.fmt == FMT_S) {
	       FP_FLOAT(i.freg.fd) = FP_FLOAT(i.freg.fs) + FP_FLOAT(i.freg.ft);
	    }
	    else {
	       FP_DBL(i.freg.fd) = FP_DBL(i.freg.fs) + FP_DBL(i.freg.ft);
	    }
	    break;

	 case 1:			// sub
	    if (i.freg.fmt == FMT_S) {
	       /* .s */
	       FP_FLOAT(i.freg.fd) = FP_FLOAT(i.freg.fs) - FP_FLOAT(i.freg.ft);
	    }
	    else {
	       /* .d */
	       FP_DBL(i.freg.fd) = FP_DBL(i.freg.fs) - FP_DBL(i.freg.ft);
	    }
	    break;
	 case 2:			// mul
	    if (i.freg.fmt == FMT_S) {
	       /* .s */
	       FP_FLOAT(i.freg.fd) = FP_FLOAT(i.freg.fs) * FP_FLOAT(i.freg.ft);
	    }
	    else {
	       /* .d */
	       FP_DBL(i.freg.fd) = FP_DBL(i.freg.fs) * FP_DBL(i.freg.ft);
	    }
	    break;
	 case 3:			// div
	    if (i.freg.fmt == FMT_S) {
	       /* .s */
	       if (FP_FLOAT(i.freg.ft) == 0) {
		  printf ("Division by zero...\n");
	       }
	       else {
		  FP_FLOAT(i.freg.fd) = FP_FLOAT(i.freg.fs)/FP_FLOAT(i.freg.ft);
	       }
	    }
	    else {
	       /* .d */
	       if (FP_DBL (i.freg.ft) == 0) {
		  printf ("Division by zero...\n");
	       }
	       else {
		  FP_DBL(i.freg.fd) = FP_DBL(i.freg.fs) / FP_DBL(i.freg.ft);
	       }
	    }
	    break;
	 case 5:			// abs
	    if (i.freg.fmt == FMT_S) {
	       /* .s */
	       FP_FLOAT(i.freg.fd) = (FP_FLOAT(i.freg.fs) >= 0.0) ? 
		  FP_FLOAT(i.freg.fs) : -FP_FLOAT(i.freg.fs);
	    }
	    else {
	       /* .d */
	       FP_DBL(i.freg.fd) = (FP_DBL(i.freg.fs) >= 0.0) ?
		  FP_DBL(i.freg.fs) : -FP_DBL(i.freg.fs);
	    }
	    break;
	 case 6:			// mov
	    if (i.freg.fmt == FMT_S) {
	       FP_FLOAT(i.freg.fd) = FP_FLOAT(i.freg.fs);
	    }
	    else {
	       FP_DBL(i.freg.fd) = FP_DBL(i.freg.fs);
	    }	 
	    break;
	 case 7:			// neg
	    if (i.freg.fmt == FMT_S) {
	       FP_FLOAT(i.freg.fd) = -FP_FLOAT(i.freg.fs);
	    }
	    else {
	       FP_DBL(i.freg.fd) = -FP_DBL(i.freg.fs);
	    }
	    break;
	 case 0x30:			// compare
	 case 0x31:			// compare
	 case 0x32:			// compare
	 case 0x33:			// compare
	 case 0x34:			// compare
	 case 0x35:			// compare
	 case 0x36:			// compare
	 case 0x37:			// compare
	 case 0x38:			// compare
	 case 0x39:			// compare
	 case 0x3a:			// compare
	 case 0x3b:			// compare
	 case 0x3c:			// compare
	 case 0x3d:			// compare
	 case 0x3e:			// compare
	 case 0x3f:			// compare
	 {
	    int less, equal, unordered;
	    int cond;

	    if (i.freg.fmt == FMT_S) {
	       /* single-precision */
	       if (s_isNaN (i.freg.fs) || s_isNaN (i.freg.ft)) {
		  less = 0;
		  unordered = 1;
		  equal = 0;
	       }
	       else {
		  less = ((FP_FLOAT(i.freg.fs) < FP_FLOAT(i.freg.ft)) ? 1 : 0);
		  equal = ((FP_FLOAT(i.freg.fs) == FP_FLOAT(i.freg.ft)) ? 1 : 0);
		  unordered = 0;
	       }
	    }
	    else {
	       /* double precision */
	       if (d_isNaN (i.freg.fs) || d_isNaN (i.freg.ft)) {
		  less = 0;
		  unordered = 1;
		  equal = 0;
	       }
	       else {
		  less = ((FP_DBL(i.freg.fs) < FP_DBL(i.freg.ft)) ? 1 : 0);
		  equal = ((FP_DBL(i.freg.fs) == FP_DBL(i.freg.ft)) ? 1 : 0);
		  unordered = 0;
	       }
	    }
	    cond = ((((i.freg.func & 0x04) && (less == 1)) ||
		     ((i.freg.func & 0x02) && (equal == 1)) || 
		     ((i.freg.func & 0x01) && (unordered == 1))) ? 1 : 0);
	    (*fcr) &= ~(1 << 23); // clear condition bit
	    (*fcr) |= (cond << 23);
	    (*coc1) = cond;
	 }
	 break;

	 case 0x21:			// cvt.d
	    if (i.freg.fmt == FMT_W) /* cvt.d.w */ {
	       FP_DBL (i.freg.fd) = (signed)FP_LONG(i.freg.fs);
	    }
	    else
	       FP_DBL (i.freg.fd) = FP_FLOAT(i.freg.fs);
	    break;
	 case 0x20:			// cvt.s
	    if (i.freg.fmt == FMT_W) /* cvt.s.w */
	       FP_FLOAT(i.freg.fd) = (signed)FP_LONG(i.freg.fs);
	    else
	       FP_FLOAT(i.freg.fd) = FP_DBL(i.freg.fs);
	    break;
	 case 0x24:			// cvt.w
	    if (i.freg.fmt == FMT_S) {
	       float val, l;

	       val = FP_FLOAT(i.freg.fs);

	       switch ((*fcr) & 0x3) {
	       case 0:
		  /* RN */
		  l = (val > 0.0 ? val + 0.5 : val - 0.5);
		  break;
	       case 1:
		  /* RZ */
		  l = val;
		  break;
	       case 2:
		  /* RP */
		  l = (val >= 0.0 ? (val + 1 - FLT_EPSILON) : val);
		  break;
	       case 3:
		  /* RM */
		  break;
	       }
	       FP_LONG(i.freg.fd) = (int)l;
	    }
	    else {
	       double val, l;

	       val = FP_DBL(i.freg.fs);

	       switch ((*fcr) & 0x3) {
	       case 0:
		  /* RN */
		  l = (val > 0.0 ? val + 0.5 : val - 0.5);
		  break;
	       case 1:
		  /* RZ */
		  l = val;
		  break;
	       case 2:
		  /* RP */
		  l = (val >= 0.0 ? val + (1-DBL_EPSILON) : val);
		  break;
	       case 3:
		  /* RM */
		  l = (val >= 0.0 ? val : val - (1-DBL_EPSILON));
		  break;
	       }
	       FP_LONG(i.freg.fd) = (int)l;
	    }
	    break;
	 default:
	    printf ("Unknown fp instruction at PC=%#x\n", pc);
            assert(0);
	    break;
	 }
	 break;
      default:
	 printf ("Unknown fp fmt at PC=%#x\n", pc);
         assert(0);
	 break;
      }
      break;
   default:
      printf ("Unknown instruction at PC=%#x, Opcode=%u\n", pc, i.reg.op);
      assert(0);
      break;
   }
}
