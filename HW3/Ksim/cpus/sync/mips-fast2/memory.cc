#include "memory.h"

Memory::Memory (Mipc *mc)
{
   _mc = mc;
}

Memory::~Memory (void) {}

void
Memory::MainLoop (void)
{
   Bool ins;
   Bool isSyscall;
   Bool isIllegalOp;
   Bool memControl;
   Bool writeREG;
   Bool writeFREG;
   Bool loWPort;
   Bool hiWPort;
   Bool decodedDST;
   Bool opResultLo;
   Bool opResultHi;

   while (1) {
      AWAIT_P_PHI0;	// @posedge
      if (_mc->_execValid) {
         // memControl = _mc->_memControl;
      ins         = _mc->_pipeReg_em[0];
      isSyscall   = _mc->_pipeReg_em[1];
      isIllegalOp = _mc->_pipeReg_em[2];
      memControl  = _mc->_pipeReg_em[3];  
      writeREG    = _mc->_pipeReg_em[4];
      writeFREG   = _mc->_pipeReg_em[5];
      loWPort     = _mc->_pipeReg_em[6];
      hiWPort     = _mc->_pipeReg_em[7];
      decodedDST  = _mc->_pipeReg_em[8];
      opResultLo  = _mc->_pipeReg_em[9];
      opResultHi  = _mc->_pipeReg_em[10];



         AWAIT_P_PHI1;       // @negedge
         if (memControl) {
            _mc->_memOp (_mc);
#ifdef MIPC_DEBUG
            fprintf(_mc->_debugLog, "<%llu> Accessing memory at address %#x for ins %#x\n", SIM_TIME, _mc->_MAR, ins);
#endif
         }
         else {
#ifdef MIPC_DEBUG
            fprintf(_mc->_debugLog, "<%llu> Memory has nothing to do for ins %#x\n", SIM_TIME, ins);
#endif
         }
         // _mc->_execValid = FALSE;
         _mc->_memValid = TRUE;
         // _mc->_insDone = TRUE;

      _mc->_pipeReg_mw[0] = ins;
      _mc->_pipeReg_mw[1] = isSyscall;
      _mc->_pipeReg_mw[2] = isIllegalOp;
      _mc->_pipeReg_mw[3] = writeREG;
      _mc->_pipeReg_mw[4] = writeFREG;
      _mc->_pipeReg_mw[5] = loWPort;
      _mc->_pipeReg_mw[6] = hiWPort;
      _mc->_pipeReg_mw[7] = decodedDST;
      _mc->_pipeReg_mw[8] = opResultLo;
      _mc->_pipeReg_mw[9] = opResultHi;
      
      }
      else {
         PAUSE(1);
      }
   }
}
