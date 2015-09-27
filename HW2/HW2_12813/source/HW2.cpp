#include "pin.H"
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <map>
#include <climits>

UINT64 insCount = 0;        //number of dynamically executed instructions
UINT64 fast_forward_count = 0;

// PART B
BOOL flag_B;
UINT32 count_BTB = 0;
ADDRINT srcAddr_1;
ADDRINT srcAddr_2;

UINT32 count_miss_1 = 0;
UINT32 count_misprediction_1 = 0;
ADDRINT tarAddr_1;

UINT32 count_miss_2 = 0;
UINT32 count_misprediction_2 = 0;
ADDRINT tarAddr_2;

// PART A
BOOL flag_A;
UINT32 count = 0;
UINT32 count_forw = 0;
UINT32 count_back = 0;

UINT32 corrCount_static = 0;
UINT32 corrCount_static_forw = 0;
UINT32 corrCount_static_back = 0;
BOOL prediction_taken_static;
long unsigned int prediction_static;

UINT32 corrCount_bimodal = 0;
UINT32 corrCount_bimodal_forw = 0;
UINT32 corrCount_bimodal_back = 0;
BOOL prediction_taken_bimodal;
long unsigned int prediction_bimodal;
UINT32 bimodal_index;

UINT32 corrCount_SAg = 0;
UINT32 corrCount_SAg_forw = 0;
UINT32 corrCount_SAg_back = 0;
BOOL prediction_taken_SAg;
long unsigned int prediction_SAg;
UINT32 SAg_index;

UINT32 corrCount_GAg = 0;
UINT32 corrCount_GAg_forw = 0;
UINT32 corrCount_GAg_back = 0;
BOOL prediction_taken_GAg;
long unsigned int prediction_GAg;

UINT32 corrCount_gshare = 0;
UINT32 corrCount_gshare_forw = 0;
UINT32 corrCount_gshare_back = 0;
BOOL prediction_taken_gshare;
long unsigned int prediction_gshare;
UINT32 gshare_index;

UINT32 corrCount_hybrid2 = 0;
UINT32 corrCount_hybrid2_forw = 0;
UINT32 corrCount_hybrid2_back = 0;
BOOL prediction_taken_hybrid2;
long unsigned int prediction_hybrid2;

UINT32 corrCount_hybrid31 = 0;
UINT32 corrCount_hybrid31_forw = 0;
UINT32 corrCount_hybrid31_back = 0;
BOOL prediction_taken_hybrid31;
long unsigned int prediction_hybrid31;

UINT32 corrCount_hybrid32 = 0;
UINT32 corrCount_hybrid32_forw = 0;
UINT32 corrCount_hybrid32_back = 0;
BOOL prediction_taken_hybrid32;
BOOL prediction_taken_hybrid321;
BOOL prediction_taken_hybrid322;
BOOL prediction_taken_hybrid323;
long unsigned int prediction_hybrid32;
long unsigned int prediction_hybrid322;
long unsigned int prediction_hybrid323;


std::ostream * out = &cerr;

class LRUClock{
private:
    int n;  // number of counters;
    int pntr;
    bool * clk;
public:
    LRUClock(int num){
        n = num;
        pntr = 0;
        clk = (bool*)calloc(n,sizeof(bool)); 
    }

    void setUpdateClock(int idx){
        clk[idx] = 1;
    }

    void unsetUpdateClock(int idx){
        clk[idx] = 0;
    }

    int replacementIndex(){
        while(clk[pntr]){
            clk[pntr] = 0;
            pntr = (pntr+1)%n;
        }
        clk[pntr] = 1;
        return pntr;
    }
};

class BTBuffer{
private:
    ADDRINT ** tag;
    ADDRINT ** target;
    bool ** valid;
    LRUClock * clk;
public:
    BTBuffer(){
        int i;

        tag = (ADDRINT **)calloc(4,sizeof(ADDRINT*));    
        for(i=0;i<4;i++) tag[i] = (ADDRINT *)calloc(128,sizeof(ADDRINT));    
        target = (ADDRINT **)calloc(4,sizeof(ADDRINT*));    
        for(i=0;i<4;i++) target[i] = (ADDRINT *)calloc(128,sizeof(ADDRINT));    
        valid = (bool **)calloc(4,sizeof(bool*));    
        for(i=0;i<4;i++) valid[i] = (bool *)calloc(128,sizeof(bool)); 

        clk = new LRUClock(4);   
    }

    ADDRINT chkHitMiss(ADDRINT insAddr){
        int idx = insAddr%128;
        for(int i=0;i<4;i++){
            if((tag[i][idx] == insAddr/128) && valid[i][idx]){
                clk->setUpdateClock(i);            
                return target[i][idx];
            }
        }
        return (ADDRINT)-1;
    }

    void addMissNTaken(ADDRINT insAddr, ADDRINT tarAddr){
        int idx = insAddr%128;
        int set = clk->replacementIndex();

        tag[set][idx] = insAddr/128;
        target[set][idx] = tarAddr;
        valid[set][idx] = 1;
    }

    void removeHitNNotTaken(ADDRINT insAddr){
        int idx = insAddr%128;
        for(int i=0;i<4;i++){
            if((tag[i][idx] == insAddr/128) && valid[i][idx]) {
                valid[i][idx] = 0;
                clk->unsetUpdateClock(i);
                break;
            }
        }
    }
};

class NBitRegisterFile{
private:
    int x,y;
    bool ** reg;
public:
    NBitRegisterFile(int x, int y){
        this->x = x;
        this->y = y;
        int i;
        reg = (bool**)calloc(x,sizeof(bool*));
        for(i=0;i<x;i++) reg[i] = (bool*)calloc(y,sizeof(bool));
    }

    void incrementReg(int whichX){
        bool * whichReg = reg[whichX];
        int i;
        for(i=0;i<y;i++){
            if(!whichReg[i]){
                whichReg[i]=1;
                while(i--) whichReg[i]=0;
                break;
            }
        }
    }

    void decrementReg(int whichX){
        bool * whichReg = reg[whichX];
        int i;
        for(i=0;i<y;i++){
            if(whichReg[i]){
                whichReg[i]=0;
                while(i--) whichReg[i]=1;
                break;
            }
        }
    }

    bool getPrediction(int whichX){
        return reg[whichX][y-1];
    }
};

class NBitShiftRegisterFile{
private:
    int x,y;
    int *reg;
public:
    NBitShiftRegisterFile(int x, int y){
        this->x = x;
        this->y = y;
        reg = (int*)calloc(x,sizeof(int));
    }
    
    void addHistory(int whichX, bool hisBit){
        reg[whichX] = (reg[whichX]<<1)%(int)pow(2,y) + hisBit;
    }

    int getPHTIndex(int whichX){
        return reg[whichX];
    }
};

NBitRegisterFile bimodal(512,2);

NBitShiftRegisterFile SAgBHT(1024,9);
NBitRegisterFile SAgPHT(512,2);

NBitShiftRegisterFile GAgBHT(1,9);
NBitRegisterFile GAgPHT(512,3);

NBitShiftRegisterFile gshareBHT(1,9);
NBitRegisterFile gsharePHT(512,3);

NBitShiftRegisterFile hybrid2BHT(1,9);
NBitRegisterFile hybrid2PHT(512,2);

NBitShiftRegisterFile hybrid321BHT(1,9);
NBitRegisterFile hybrid321PHT(512,2);

NBitShiftRegisterFile hybrid322BHT(1,9);
NBitRegisterFile hybrid322PHT(512,2);

NBitShiftRegisterFile hybrid323BHT(1,9);
NBitRegisterFile hybrid323PHT(512,2);


KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE,  "pintool",
    "o", "", "specify file name for MyPinTool output");

KNOB<UINT64> KnobFastAmount(KNOB_MODE_WRITEONCE,  "pintool",
    "f", "0", "specify fast-forward amount");

BTBuffer *BTB_1 = new BTBuffer();
BTBuffer *BTB_2 = new BTBuffer();

VOID InsCount()  {
    insCount++;
}

ADDRINT Terminate(void)
{
    return (insCount >= fast_forward_count + 1000000000);
}

ADDRINT FastForward(void) {
    return (insCount >= fast_forward_count && insCount);
}

void PrintStats(){
    *out <<  "===============================================" << endl;
    *out <<  "HW2 analysis results- " << endl;
    *out <<  "===============================================" << endl;
    *out <<  "PART A Results: " << endl;
    *out <<  "===============================================" << endl;
    *out <<  "Count " << count << endl;
    *out <<  "corrCount_static "<< corrCount_static << endl;
    *out <<  "\tfraction of misprediction: " << (double)(count - corrCount_static)/count << endl;
    *out <<  "\tfraction of forward misprediction: " << (double)(count_forw - corrCount_static_forw)/count_forw << endl;
    *out <<  "\tfraction of backward misprediction: " << (double)(count_back - corrCount_static_back)/count_back << endl;
    *out <<  "corrCount_bimodal "<< corrCount_bimodal << endl;
    *out <<  "\tfraction of misprediction: " << (double)(count - corrCount_bimodal)/count << endl;
    *out <<  "\tfraction of forward misprediction: " << (double)(count_forw - corrCount_bimodal_forw)/count_forw << endl;
    *out <<  "\tfraction of backward misprediction: " << (double)(count_back - corrCount_bimodal_back)/count_back << endl;
    *out <<  "corrCount_SAg "<< corrCount_SAg << endl;
    *out <<  "\tfraction of misprediction: " << (double)(count - corrCount_SAg)/count << endl;
    *out <<  "\tfraction of forward misprediction: " << (double)(count_forw - corrCount_SAg_forw)/count_forw << endl;
    *out <<  "\tfraction of backward misprediction: " << (double)(count_back - corrCount_SAg_back)/count_back << endl;
    *out <<  "corrCount_GAg "<< corrCount_GAg << endl;
    *out <<  "\tfraction of misprediction: " << (double)(count - corrCount_GAg)/count << endl;
    *out <<  "\tfraction of forward misprediction: " << (double)(count_forw - corrCount_GAg_forw)/count_forw << endl;
    *out <<  "\tfraction of backward misprediction: " << (double)(count_back - corrCount_GAg_back)/count_back << endl;
    *out <<  "corrCount_gshare "<< corrCount_gshare << endl;
    *out <<  "\tfraction of misprediction: " << (double)(count - corrCount_gshare)/count << endl;
    *out <<  "\tfraction of forward misprediction: " << (double)(count_forw - corrCount_gshare_forw)/count_forw << endl;
    *out <<  "\tfraction of backward misprediction: " << (double)(count_back - corrCount_gshare_back)/count_back << endl;
    *out <<  "corrCount_hybrid2 "<< corrCount_hybrid2 << endl;
    *out <<  "\tfraction of misprediction: " << (double)(count - corrCount_hybrid2)/count << endl;
    *out <<  "\tfraction of forward misprediction: " << (double)(count_forw - corrCount_hybrid2_forw)/count_forw << endl;
    *out <<  "\tfraction of backward misprediction: " << (double)(count_back - corrCount_hybrid2_back)/count_back << endl;
    *out <<  "corrCount_hybrid31 "<< corrCount_hybrid31 << endl;
    *out <<  "\tfraction of misprediction: " << (double)(count - corrCount_hybrid31)/count << endl;
    *out <<  "\tfraction of forward misprediction: " << (double)(count_forw - corrCount_hybrid31_forw)/count_forw << endl;
    *out <<  "\tfraction of backward misprediction: " << (double)(count_back - corrCount_hybrid31_back)/count_back << endl;
    *out <<  "corrCount_hybrid32 "<< corrCount_hybrid32 << endl;
    *out <<  "\tfraction of misprediction: " << (double)(count - corrCount_hybrid32)/count << endl;
    *out <<  "\tfraction of forward misprediction: " << (double)(count_forw - corrCount_hybrid32_forw)/count_forw << endl;
    *out <<  "\tfraction of backward misprediction: " << (double)(count_back - corrCount_hybrid32_back)/count_back << endl;
    *out <<  "===============================================" << endl;
    *out <<  "PART B Results: " << endl;
    *out <<  "===============================================" << endl;
    *out <<  "Total Count: " << count_BTB << endl;
    *out <<  "\tMiss Count percentage BTB 1: " << (double)(count_miss_1)/count_BTB << endl;
    *out <<  "\tMisprediction Count percentage BTB 1: " << (double)(count_misprediction_1)/count_BTB << endl;
    *out <<  "\tMiss Count percentage BTB 2: " << (double)(count_miss_2)/count_BTB << endl;
    *out <<  "\tMisprediction Count percentage BTB 2: " << (double)(count_misprediction_2)/count_BTB << endl;
    *out <<  "===============================================" << endl;
}

void MyExitRoutine() {
    PrintStats();
    exit(0);
}

void MyPredicatedAnalysis_PARTB(ADDRINT insAddr) {
    flag_B = 1;
    srcAddr_1 = insAddr;
    srcAddr_2 = insAddr^GAgBHT.getPHTIndex(0);
    tarAddr_1 = BTB_1->chkHitMiss(srcAddr_1);
    tarAddr_2 = BTB_2->chkHitMiss(srcAddr_2);
}

void MyPredicatedAnalysis_chk_PARTB(ADDRINT insAddr){
    if(flag_B){
        count_BTB++;
        if(tarAddr_1 == (ADDRINT)-1){
            count_miss_1++;
            BTB_1->addMissNTaken(srcAddr_1, insAddr);
        }
        else if(tarAddr_1 != insAddr){
            count_misprediction_1++;
            BTB_1->removeHitNNotTaken(srcAddr_1);
        }

        if(tarAddr_2 == (ADDRINT)-1){
            count_miss_2++;
            BTB_2->addMissNTaken(srcAddr_2, insAddr);
        }
        else if(tarAddr_2 != insAddr){
            count_misprediction_2++;
            BTB_2->removeHitNNotTaken(srcAddr_2);
        }
    }
    flag_B = 0;
}

void MyPredicatedAnalysis_PARTA(ADDRINT insAddr, ADDRINT fallInsAddr, ADDRINT brInsAddr) {
    flag_A = 1;

    // static predictor
    prediction_taken_static = (insAddr > brInsAddr);
    if(prediction_taken_static)
        prediction_static = brInsAddr;
    else prediction_static = fallInsAddr;

    // bimodal predictor
    bimodal_index = insAddr%512;
    prediction_taken_bimodal = bimodal.getPrediction(bimodal_index);
    if(prediction_taken_bimodal) prediction_bimodal = brInsAddr;
    else prediction_bimodal = fallInsAddr;

    //SAg predictor
    SAg_index = insAddr%1024;
    prediction_taken_SAg = SAgPHT.getPrediction(SAgBHT.getPHTIndex(SAg_index));
    if(prediction_taken_SAg) prediction_SAg = brInsAddr;
    else prediction_SAg = fallInsAddr;

    //GAg predictor
    prediction_taken_GAg = GAgPHT.getPrediction(GAgBHT.getPHTIndex(0));
    if(prediction_taken_GAg) prediction_GAg = brInsAddr;
    else prediction_GAg = fallInsAddr;

    //gshare predictor
    gshare_index = gshareBHT.getPHTIndex(0)^(insAddr%512);
    prediction_taken_gshare = gsharePHT.getPrediction(gshare_index);
    if(prediction_taken_gshare) prediction_gshare = brInsAddr;
    else prediction_gshare = fallInsAddr;

    //hybrid2 predictor
    prediction_taken_hybrid2 = hybrid2PHT.getPrediction(hybrid2BHT.getPHTIndex(0)) ? prediction_taken_GAg:prediction_taken_SAg;
    if(prediction_taken_hybrid2) prediction_hybrid2 = brInsAddr;
    else prediction_hybrid2 = fallInsAddr;

    //hybrid31 predictor
    prediction_taken_hybrid31 = ((prediction_taken_GAg+prediction_taken_SAg+prediction_taken_gshare)>1);
    if(prediction_taken_hybrid31) prediction_hybrid31 = brInsAddr;
    else prediction_hybrid31 = fallInsAddr;

    //hybrid31 predictor
    prediction_taken_hybrid321 = prediction_taken_hybrid2;
    prediction_taken_hybrid322 = hybrid322PHT.getPrediction(hybrid322BHT.getPHTIndex(0)) ? prediction_taken_gshare:prediction_taken_GAg;
    prediction_taken_hybrid323 = hybrid323PHT.getPrediction(hybrid323BHT.getPHTIndex(0)) ? prediction_taken_SAg:prediction_taken_gshare;
    prediction_taken_hybrid32 = ((prediction_taken_hybrid321+prediction_taken_hybrid322+prediction_taken_hybrid323)>1);
    if(prediction_taken_hybrid32) prediction_hybrid32 = brInsAddr;
    else prediction_hybrid32 = fallInsAddr;
    if(prediction_taken_hybrid322) prediction_hybrid322 = brInsAddr;
    else prediction_hybrid322 = fallInsAddr;
    if(prediction_taken_hybrid323) prediction_hybrid323 = brInsAddr;
    else prediction_hybrid323 = fallInsAddr;
}

void MyPredicatedAnalysis_chk_PARTA(ADDRINT insAddr){
    if(flag_A == 1){
        count++;
        if(prediction_taken_static) count_back++;
        else count_forw++;

        //Static
        if(prediction_static == insAddr){
            corrCount_static++;
            if(prediction_taken_static) corrCount_static_back++;
            else corrCount_static_forw++;
        }

        //Bimodal
        if(prediction_bimodal == insAddr){
            corrCount_bimodal++;
            if(prediction_taken_static) corrCount_bimodal_back++;
            else corrCount_bimodal_forw++;
        
            if(prediction_taken_bimodal) bimodal.incrementReg(bimodal_index);
            else bimodal.decrementReg(bimodal_index);
        }
        else{
            if(prediction_taken_bimodal) bimodal.decrementReg(bimodal_index);
            else bimodal.incrementReg(bimodal_index);
        }

        //SAg
        if(prediction_SAg == insAddr){
            corrCount_SAg++;
            if(prediction_taken_static) corrCount_SAg_back++;
            else corrCount_SAg_forw++;
        
            if(prediction_taken_SAg) 
                SAgPHT.incrementReg(SAgBHT.getPHTIndex(SAg_index)), SAgBHT.addHistory(SAg_index, 1);
            else
                SAgPHT.decrementReg(SAgBHT.getPHTIndex(SAg_index)), SAgBHT.addHistory(SAg_index, 0);   
        }
        else{
            if(prediction_taken_SAg) 
                SAgPHT.decrementReg(SAgBHT.getPHTIndex(SAg_index)), SAgBHT.addHistory(SAg_index, 0);
            else 
                SAgPHT.incrementReg(SAgBHT.getPHTIndex(SAg_index)), SAgBHT.addHistory(SAg_index, 1);   
        }

        //GAg
        if(prediction_GAg == insAddr){
            corrCount_GAg++;
            if(prediction_taken_static) corrCount_GAg_back++;
            else corrCount_GAg_forw++;
        
            if(prediction_taken_GAg) 
                GAgPHT.incrementReg(GAgBHT.getPHTIndex(0)), GAgBHT.addHistory(0, 1);
            else 
                GAgPHT.decrementReg(GAgBHT.getPHTIndex(0)), GAgBHT.addHistory(0, 0); 
        }
        else{
            if(prediction_taken_GAg) 
                GAgPHT.decrementReg(GAgBHT.getPHTIndex(0)), GAgBHT.addHistory(0, 0);
            else 
                GAgPHT.incrementReg(GAgBHT.getPHTIndex(0)), GAgBHT.addHistory(0, 1); 
        }

        //gshare
        if(prediction_gshare == insAddr){
            corrCount_gshare++;
            if(prediction_taken_static) corrCount_gshare_back++;
            else corrCount_gshare_forw++;
        
            if(prediction_taken_gshare) 
                gsharePHT.incrementReg(gshare_index), gshareBHT.addHistory(0, 1);
            else 
                gsharePHT.decrementReg(gshare_index), gshareBHT.addHistory(0, 0); 
        }
        else{
            if(prediction_taken_gshare) 
                gsharePHT.decrementReg(gshare_index), gshareBHT.addHistory(0, 0);
            else 
                gsharePHT.incrementReg(gshare_index), gshareBHT.addHistory(0, 1); 
        }

        //hybrid2
        if(prediction_hybrid2 == insAddr){
            corrCount_hybrid2++;
            if(prediction_taken_static) corrCount_hybrid2_back++;
            else corrCount_hybrid2_forw++;
        
            if(prediction_taken_SAg == prediction_taken_GAg);
            else if(prediction_taken_hybrid2 == prediction_taken_GAg) 
                hybrid2PHT.incrementReg(hybrid2BHT.getPHTIndex(0)), hybrid2BHT.addHistory(0, 1);
            else if(prediction_taken_hybrid2 == prediction_taken_SAg)
                hybrid2PHT.decrementReg(hybrid2BHT.getPHTIndex(0)), hybrid2BHT.addHistory(0, 0); 
        }
        else{
            if(prediction_taken_SAg == prediction_taken_GAg);
            else if(prediction_taken_hybrid2 == prediction_taken_GAg) 
                hybrid2PHT.decrementReg(hybrid2BHT.getPHTIndex(0)), hybrid2BHT.addHistory(0, 0);
            else if(prediction_taken_hybrid2 == prediction_taken_SAg)
                hybrid2PHT.incrementReg(hybrid2BHT.getPHTIndex(0)), hybrid2BHT.addHistory(0, 1);
        }

        //hybrid322
        if(prediction_taken_hybrid322 == insAddr){
            if(prediction_taken_GAg == prediction_taken_gshare);
            else if(prediction_taken_hybrid322 == prediction_taken_gshare) 
                hybrid322PHT.incrementReg(hybrid322BHT.getPHTIndex(0)), hybrid322BHT.addHistory(0, 1);
            else if(prediction_taken_hybrid322 == prediction_taken_GAg)
                hybrid322PHT.decrementReg(hybrid322BHT.getPHTIndex(0)), hybrid322BHT.addHistory(0, 0); 
        }
        else{
            if(prediction_taken_GAg == prediction_taken_gshare);
            else if(prediction_taken_hybrid322 == prediction_taken_gshare) 
                hybrid322PHT.decrementReg(hybrid322BHT.getPHTIndex(0)), hybrid322BHT.addHistory(0, 0);
            else if(prediction_taken_hybrid322 == prediction_taken_GAg)
                hybrid322PHT.incrementReg(hybrid322BHT.getPHTIndex(0)), hybrid322BHT.addHistory(0, 1);
        }

        //hybrid323
        if(prediction_taken_hybrid323 == insAddr){
            if(prediction_taken_gshare == prediction_taken_SAg);
            else if(prediction_taken_hybrid323 == prediction_taken_SAg) 
                hybrid323PHT.incrementReg(hybrid323BHT.getPHTIndex(0)), hybrid323BHT.addHistory(0, 1);
            else if(prediction_taken_hybrid323 == prediction_taken_gshare)
                hybrid323PHT.decrementReg(hybrid323BHT.getPHTIndex(0)), hybrid323BHT.addHistory(0, 0); 
        }
        else{
            if(prediction_taken_gshare == prediction_taken_SAg);
            else if(prediction_taken_hybrid323 == prediction_taken_SAg) 
                hybrid323PHT.decrementReg(hybrid323BHT.getPHTIndex(0)), hybrid323BHT.addHistory(0, 0);
            else if(prediction_taken_hybrid323 == prediction_taken_gshare)
                hybrid323PHT.incrementReg(hybrid323BHT.getPHTIndex(0)), hybrid323BHT.addHistory(0, 1); 
        }

        //hybrid31
        if(prediction_hybrid31 == insAddr){
            corrCount_hybrid31++;
            if(prediction_taken_static) corrCount_hybrid31_back++;
            else corrCount_hybrid31_forw++;
        }

        //hybrid32
        if(prediction_hybrid32 == insAddr){
            corrCount_hybrid32++;
            if(prediction_taken_static) corrCount_hybrid32_back++;
            else corrCount_hybrid32_forw++;
        }
    }
    flag_A = 0;
}

void Instruction(INS ins, void *v) {
    INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR) Terminate, IARG_END);
    INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR) MyExitRoutine, IARG_END);

    INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR) FastForward, IARG_END);
    INS_InsertThenPredicatedCall(ins, IPOINT_BEFORE, 
    (AFUNPTR) MyPredicatedAnalysis_chk_PARTA, IARG_INST_PTR, IARG_END);
    

    if (INS_Category(ins) == XED_CATEGORY_COND_BR) {
            if(INS_IsDirectBranchOrCall(ins)){
                INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR) FastForward, IARG_END);
                INS_InsertThenPredicatedCall(ins, IPOINT_BEFORE, 
                (AFUNPTR) MyPredicatedAnalysis_PARTA, IARG_INST_PTR, IARG_FALLTHROUGH_ADDR , IARG_BRANCH_TARGET_ADDR, IARG_END);
            }
    }

    INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR) FastForward, IARG_END);
    INS_InsertThenPredicatedCall(ins, IPOINT_BEFORE, 
    (AFUNPTR) MyPredicatedAnalysis_chk_PARTB, IARG_INST_PTR, IARG_END);
    
    if (INS_Category(ins) == XED_CATEGORY_CALL && !INS_IsDirectCall(ins)) {
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR) FastForward, IARG_END);
        INS_InsertThenPredicatedCall(ins, IPOINT_BEFORE, 
        (AFUNPTR) MyPredicatedAnalysis_PARTB, IARG_INST_PTR, IARG_END);
    }

    INS_InsertCall(ins, IPOINT_BEFORE, InsCount, IARG_END);
}


VOID Fini(INT32 code, VOID *v)
{
    PrintStats();
}

int main(int argc, char *argv[])
{
    PIN_Init(argc,argv);
    
    string fileName = KnobOutputFile.Value();
    fast_forward_count = KnobFastAmount * 1000000000;

    if (!fileName.empty()) { out = new std::ofstream(fileName.c_str());}

    INS_AddInstrumentFunction(Instruction, 0);

    PIN_AddFiniFunction(Fini, 0);
    
    cerr <<  "===============================================" << endl;
    cerr <<  "This application is instrumented by HW1" << endl;
    if (!KnobOutputFile.Value().empty()) 
    {
        cerr << "See file " << KnobOutputFile.Value() << " for analysis results" << endl;
    }
    cerr <<  "===============================================" << endl;

    // Start the program, never returns
    PIN_StartProgram();
    
    return 0;
}