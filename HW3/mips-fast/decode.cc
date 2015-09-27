#include "decode.h"

Decode::Decode (Mipc *mc)
{
   _mc = mc;
}

Decode::~Decode (void) {}

void
Decode::MainLoop (void)
{
   unsigned int ins;
   while (1) {
      AWAIT_P_PHI0;	// @posedge
      if (_mc->_insValid ) {
         ins = _mc->_pipeReg_fd[0];
	 _mc->_bd = 0;
	//_mc->printROB();

         AWAIT_P_PHI1;	// @negedge
         _mc->Dec(ins);
         if(_mc->_isSyscall) _mc->_pipeStalled = 1;
	
#ifdef MIPC_DEBUG
         fprintf(_mc->_debugLog, "<%llu> Decoded ins %#x btgt: %#x\n", SIM_TIME, ins, _mc->_btgt);
#endif

	// if(_mc->_bd){
	// 	printf("Register state on termination for ins %#x:\n\n", ins);
 //            _mc->dumpregs(ins);
 //            exit(0);
	// }	
            _mc->_pipeReg_de[0] = ins;
            _mc->_pipeReg_de[1] = _mc->_isSyscall;
            _mc->_pipeReg_de[2] = _mc->_isIllegalOp;
            _mc->_pipeReg_de[3] = _mc->_bd;
   	      _mc->_pipeReg_de[4] = _mc->_btgt;
            _mc->_pipeReg_de[5] = _mc->_memControl;
            _mc->_pipeReg_de[6] = _mc->_writeREG;
            _mc->_pipeReg_de[7] = _mc->_writeFREG;
            _mc->_pipeReg_de[8] = _mc->_loWPort;
            _mc->_pipeReg_de[9] = _mc->_hiWPort;
            _mc->_pipeReg_de[10] = _mc->_decodedDST;
       

         // _mc->_insValid = FALSE;
         _mc->_decodeValid = TRUE;
      }
      else {
         PAUSE(1);
      }
   }
}
