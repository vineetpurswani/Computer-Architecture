#include "executor.h"

Exe::Exe (Mipc *mc)
{
   _mc = mc;
}

Exe::~Exe (void) {}

void
Exe::MainLoop (void)
{
   unsigned int ins;
   Bool isSyscall, isIllegalOp, bd, btgt;
   Bool memControl, writeREG, writeFREG, loWPort, hiWPort, decodedDST;

   while (1) {
      AWAIT_P_PHI0;	// @posedge
      if (_mc->_decodeValid) {
         ins         = _mc->_pipeReg_de[0];
         isSyscall   = _mc->_pipeReg_de[1];
         isIllegalOp = _mc->_pipeReg_de[2];
         bd          = _mc->_pipeReg_de[3];
	 btgt	     = _mc->_pipeReg_de[4];
         memControl  = _mc->_pipeReg_de[5];
         writeREG    = _mc->_pipeReg_de[6];
         writeFREG   = _mc->_pipeReg_de[7];  
         loWPort     = _mc->_pipeReg_de[8];
         hiWPort     = _mc->_pipeReg_de[9];
         decodedDST  = _mc->_pipeReg_de[10];

         // Code added for pipelining
         if (!isSyscall && !isIllegalOp && bd){
            _mc->_opControl(_mc,ins);
#ifdef MIPC_DEBUG
            fprintf(_mc->_debugLog, "<%llu> Executed branch ins %#x with next ins at PC %#x\n", SIM_TIME, ins, btgt);
            fprintf(_mc->_debugLog, "<%llu> Checking whether the ins %#x branches: %d\n", SIM_TIME, ins, _mc->_btaken);
#endif

            if (_mc->_btaken)
            {
               _mc->_pc = btgt;
#ifdef MIPC_DEBUG
            fprintf(_mc->_debugLog, "<%llu> Branching to ins with PC %#x\n", SIM_TIME, _mc->_pc);
#endif
            }
         } 
         
         AWAIT_P_PHI1;	// @negedge
         if (!isSyscall && !isIllegalOp && !bd) {
            _mc->_opControl(_mc,ins);
#ifdef MIPC_DEBUG
            fprintf(_mc->_debugLog, "<%llu> Executed ins %#x\n", SIM_TIME, ins);
            //fprintf(_mc->_debugLog, "<%llu> Checking whether the ins %#x branches: %d\n", SIM_TIME, ins, _mc->_btaken);
#endif
            // _mc->_pc = _mc->_pc + 4;
         }
         else if (isSyscall) {
#ifdef MIPC_DEBUG
            fprintf(_mc->_debugLog, "<%llu> Deferring execution of syscall ins %#x\n", SIM_TIME, ins);
#endif
         }
         else if (isIllegalOp){
#ifdef MIPC_DEBUG
            fprintf(_mc->_debugLog, "<%llu> Illegal ins %#x in execution stage at PC %#x\n", SIM_TIME, ins, _mc->_pc);
#endif
         }
            

	 //_mc->_lastbd = bd;

         _mc->_pipeReg_em[0] = ins;
         _mc->_pipeReg_em[1] = isSyscall;
         _mc->_pipeReg_em[2] = isIllegalOp;
         _mc->_pipeReg_em[3] = memControl;
         _mc->_pipeReg_em[4] = writeREG;
         _mc->_pipeReg_em[5] = writeFREG;
         _mc->_pipeReg_em[6] = loWPort;
         _mc->_pipeReg_em[7] = hiWPort;
         _mc->_pipeReg_em[8] = decodedDST;
         _mc->_pipeReg_em[9] = _mc->_opResultLo;
         _mc->_pipeReg_em[10]= _mc->_opResultHi;
         
         // _mc->_decodeValid = FALSE;
         _mc->_execValid = TRUE;

         // if (!isIllegalOp && !isSyscall) {
         //    if (_mc->_lastbd && _mc->_btaken)
         //    {
         //       _mc->_pc = _mc->_btgt;
         //    }
         //    else
         //    {
         //       _mc->_pc = _mc->_pc + 4;
         //    }
         //    _mc->_lastbd = _mc->_bd;
         // }
      }
      else {
         PAUSE(1);
      }
   }
}
