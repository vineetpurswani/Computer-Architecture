#include "pin.H"
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include "HW4.h"

std::ostream * out = &cerr;
unsigned insCount, fast_forward_count;

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE,  "pintool",
    "o", "", "specify file name for MyPinTool output");

KNOB<UINT64> KnobFastAmount(KNOB_MODE_WRITEONCE,  "pintool",
    "f", "0", "specify fast-forward amount");

VOID InsCount()  {
    insCount++;
}

ADDRINT Terminate(void){
    return (insCount >= fast_forward_count + 1000000000);
}

void MyExitRoutine() {
    exit(0);
}

ADDRINT FastForward(void) {
    return (insCount >= fast_forward_count && insCount);
}

void ReadCache(ADDRINT readAddr){
    *out << "0\t" << readAddr << endl;
    memory_access(0, readAddr);
}
void WriteCache(ADDRINT writeAddr){
    *out << "1\t" << writeAddr << endl;
    memory_access(1, writeAddr);
}
void InsCache(ADDRINT insAddr){
    *out << "2\t" << insAddr << endl;
    memory_access(2, insAddr);
}

void Instruction(INS ins, void *v) {
    INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR) Terminate, IARG_END);
    INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR) MyExitRoutine, IARG_END);

    if (INS_IsMemoryRead(ins)){
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR) FastForward, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR) ReadCache, IARG_MEMORYREAD_EA, IARG_END);
    }
    if (INS_HasMemoryRead2(ins)){
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR) FastForward, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR) ReadCache, IARG_MEMORYREAD2_EA, IARG_END);
    }
    if(INS_IsMemoryWrite(ins)){
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR) FastForward, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR) WriteCache, IARG_MEMORYWRITE_EA, IARG_END);
    }

    INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR) FastForward, IARG_END);
    INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR) InsCache, IARG_INST_PTR, IARG_END);

    INS_InsertCall(ins, IPOINT_BEFORE, InsCount, IARG_END);
}


VOID Fini(INT32 code, VOID *v)
{
    SIM();   // useful for the end of the simulation file  
    DUMPRES();
    // PrintStats();
}

int main(int argc, char *argv[])
{
    PIN_Init(argc,argv);
    
    string fileName = KnobOutputFile.Value();
    fast_forward_count = KnobFastAmount * 1000000000;

    if (!fileName.empty()) { out = new std::ofstream(fileName.c_str());}

    INITCACHE();

    INS_AddInstrumentFunction(Instruction, 0);

    PIN_AddFiniFunction(Fini, 0);
    
    cerr <<  "===============================================" << endl;
    cerr <<  "This application is instrumented by HW4" << endl;
    if (!KnobOutputFile.Value().empty()) 
    {
        cerr << "See file " << KnobOutputFile.Value() << " for analysis results" << endl;
    }
    cerr <<  "===============================================" << endl;

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}