/*
Copyright (C) 2011 Adapteva, Inc.
Contributed by Oleg Raikhman <support@adapteva.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program, see the file COPYING. If not, see
<http://www.gnu.org/licenses/>.
*/

static char *igen_ver = "$Rev: 1480 $";

#include <scv.h>

extern void _scv_pop_constraint();

#include "misc.h"
#include "common.h"
unsigned int NREGS = 0;
#define MAX_SUB_ROUTINE_SIZE 256
#define PROGRAM_START 0x100
#define MAX_NUM_INTER 3

unsigned int MAX_NUNBER_SUBROUTINE = 1;
unsigned int MAXLOOP_IT=1;

bool isGenFP=true;
bool isFPonly=false;
bool isGenDMA=true;
bool isGenInter=true;
bool isHwLoopOn=false;
bool isAutoDMAOn=false;
bool isIntegerModeOn=false;
bool random_stop_resume_on=false;
bool isBkptOn=false;
bool isClockGateOff=false;

unsigned int chip_id = 0x88;
//#define MAX_LOAD_STORE_RANGE ((1<<11) - 64)




#define TIMER_MODE_MASK_SET   0xffffff1f
#define TIMER_MODE_MASK_CLEAR 0xffffff0f


#include "mem_manager.h"
#include "regs.h"
#include "subroutine.h"
#include "cond_type.h"

#include "intsruct_if.h"
#include "data_processing_instr.h"
#include "reg_move_regular.h"
#include "load_store_instr.h"
#include "branch_instr.h"
#include "fp_macro_instr.h"
#include "config_reg.h"
#include "swi.h"
#include "dma.h"

enum EInstrType { BRANCHING, LOAD_STORE , DATA_P, REG_MOVE,  SWI , FP_MEGA};
template<>
class scv_extensions<EInstrType> : public scv_enum_base<EInstrType> {
public:

	SCV_ENUM_CTOR(EInstrType) {
		SCV_ENUM(BRANCHING);
		SCV_ENUM(LOAD_STORE);
		SCV_ENUM(DATA_P);
		SCV_ENUM(REG_MOVE);
		SCV_ENUM(SWI);
		SCV_ENUM(FP_MEGA);
	}
};


struct TProgram {
public:

	TRegManager *regM;
	TMemManager *memManager;
	TConfigRegSetI *fConfigRegI;

	unsigned numSubRoutine;


public:
	vector<TSubRoutine *> subRoutineDoneList;
	vector<TSubRoutine *> subRoutinePendingList;
public:

	scv_smart_ptr<EInstrType> InstrSel;
	scv_bag<EInstrType> InstrDist;


public:
	TProgram() {
		numSubRoutine = 1;

		regM = new TRegManager();

		memManager = new TMemManager();

		fConfigRegI= new TConfigRegSetI("setconfig");
		fConfigRegI->BindRegsManager(regM);

		if(isFPonly) {

			InstrSel->keep_only(FP_MEGA);
		} else {


			InstrDist.add(BRANCHING,300);
			InstrDist.add(LOAD_STORE,850);/// This is not real number -- due to big loss in LS generation attempts
			InstrDist.add(DATA_P,300);
			InstrDist.add(REG_MOVE,25);
			InstrDist.add(SWI,7);

			InstrSel->set_mode(InstrDist);

			//the mega FP instruction generation breaks the generation flow, exclude it
			InstrSel->keep_out(FP_MEGA);
		}

		if(!isGenInter) {
			InstrSel->keep_out(SWI);
		}
		//TODO
		//no SWI !!!!!!!!!!!
		InstrSel->keep_out(SWI);

	}
	//TODO
	~TProgram() {

	}

	void FillSubroutine (TSubRoutine *subRoutine, ::std::ostream& s= ::std::cout) {

		unsigned numCurrBr=0;

		bool isSubroutineHWloop=false;



		s << ".section L_" << hex << subRoutine->fStartAddress << dec << ",\"a\",@progbits;" << endl;
		//s << ".size L_" << hex << subRoutine->fStartAddress << dec << ", 0x256"
		s<< ".global "<< ".L" <<   hex << subRoutine->fStartAddress << dec << ";"<< endl;
		s<< ".L" <<   hex << subRoutine->fStartAddress << dec << ":"<< endl;
		unsigned Snumber= subRoutineDoneList.size() ;
		cerr << " sub routine =====N%= " <<Snumber<< " from "<< MAX_NUNBER_SUBROUTINE << endl;


		scv_smart_ptr<unsigned> HWloop_length;
		HWloop_length->keep_only(8,16);//HW loops limitation >= 8
		HWloop_length->next();

		unsigned section_offset=8;//used to initialize loops in start(HW) or in end(SW) of section
		//loops
		if(isHwLoopOn && subRoutineDoneList.size()+1==MAX_NUNBER_SUBROUTINE && /*should be 8 boundary alignment,linker bug*/ (subRoutine->fStartAddress%8)==0) {
			InstrSel->keep_out(BRANCHING); InstrSel->keep_out(SWI); // due to generation problem //TODO
			isSubroutineHWloop=true;

			s << "MOV   R"<<regM->GetLoopCountRegisterInd()<<",%low("<< "L_" << hex << subRoutine->fStartAddress  << dec << "_hw_loop_end" <<")" <<endl;
			s << "MOVT  R"<<regM->GetLoopCountRegisterInd()<<",%high("<< "L_" << hex << subRoutine->fStartAddress  << dec << "_hw_loop_end" <<")"	<<endl;
			s << " MOVTS LE,R"<<regM->GetLoopCountRegisterInd()<<"               ;set loop-end"	<<endl;

			s << "MOV   R"<<regM->GetLoopCountRegisterInd()<<",%low("<< "L_" << hex << subRoutine->fStartAddress  << dec << "_hw_loop_start" <<") ;"<<endl;
			s << "MOV   R"<<regM->GetLoopCountRegisterInd()<<",%high("<< "L_" << hex << subRoutine->fStartAddress  << dec << "_hw_loop_start" <<") ;"<<endl;
			s << " MOVTS LS,R"<<regM->GetLoopCountRegisterInd()<<"               ;set loop-start"<<endl;

			s << "MOV   R"<<regM->GetLoopCountRegisterInd()<<",%low(0x" << hex <<1+ MAXLOOP_IT   << ");"<< dec << endl;
			s << " MOVTS LC,R"<<regM->GetLoopCountRegisterInd()<<"               ;set loop-counter"<<endl;
			s << ".balignw 8,0x01a2 ; have 8 bytes boundary alignment and fill gap by nops" << endl;
			s << "L_" << hex << subRoutine->fStartAddress  << dec << "_hw_loop_start" << ":" << endl;



			scv_smart_ptr<unsigned> loopBodyLength;//generate body from 1 to max can fit to 256 bytes section
			assert((section_offset+24) < (subRoutine->sSize-32));
			loopBodyLength->keep_only(section_offset+24/*max*/,subRoutine->sSize-32/*min*/);
			loopBodyLength->next();
			section_offset=loopBodyLength.read();//loop init HW - loop int SW
		}


		//always invalidate all regs
		regM->InvalidateAllRegs();

		unsigned int iSize = 4;
		//intruction counter for jump to middle of subroutine
		unsigned int i_counter=0;

		bool hw_loop_done=false;

		cerr << "subRoutine->sSize-section_offset " << dec << subRoutine->sSize <<  " " << section_offset << endl;
		for ( unsigned i = 0 ; i < subRoutine->sSize-section_offset/*to be on the safety side,*/; i=i+iSize) {//400

			if(hw_loop_done) break;

			//randomize the instr selector

			if(numSubRoutine>=MAX_NUNBER_SUBROUTINE) {
				InstrSel->keep_out(BRANCHING); InstrSel->keep_out(SWI); // due to generation problem //TODO
				cerr << "MAX_NUNBER_SUBROUTINE reached: stop gen jump" << endl;
			}


			if(memManager->IsNoFetchMoreMemory() ) {
				// can't jump
				s << "/*************************no more instruction memory*/" << endl;
				break;
				//exit(9);
			}

			do {
				InstrSel->next();
			} while ( (InstrSel.read() == BRANCHING)  && (numCurrBr>=2)  ); //no more 2 jump in subroutine

			bool isChangeOfFlow = false;

			TInstr_If *newInstr=0;


			switch(InstrSel.read()) {
			case DATA_P:
				cerr << "InstrSel DATA_P" << endl;
				newInstr=new TDPIntr("dp");
				break;
			case REG_MOVE:
				cerr << "InstrSel REG_MOVE" << endl;
				newInstr=new TMoveRegRegular("reg_move");
				break;

			case LOAD_STORE:
				cerr << "InstrSel LOAD_STORE" << endl;
				newInstr=new TLSInstr("load_store");
				break;

			case BRANCHING:
				cerr << "InstrSel BRANCHING" << endl;
				newInstr=new TBranchInstr("branch");
				isChangeOfFlow = true;
				break;


			case SWI:
				cerr << "InstrSel SWI" << endl;
				newInstr=new TSWI("swi");
				break;


			case FP_MEGA:
				assert(isFPonly);
				cerr << "InstrSel TFPMegaInstr" << endl;
				newInstr=new TFPMegaInstr("fp_mega");
				break;

			};


			if( newInstr ) {


				newInstr->BindRegsManager(regM);
				newInstr->BindMemManager(memManager);

				newInstr->SetCPC(subRoutine->fStartAddress+i);//assuming the all 32 bits instruction
				newInstr->GenerateMe();

				//update PC offset in subroutine
				iSize = newInstr->GetInstructionSize();
				if( iSize == 0) {
					//TODO patch for load store gen problem
					cerr << "NO instruction generated " << endl;

				} else {
					s << "L_X_" << Snumber << "_" << i_counter << ":\t";


					if(isSubroutineHWloop && i_counter+1==HWloop_length.read()) {


						s << "/*second last in the HW loop set*/" << endl;
						s << ".balignw 8,0x01a2 ; have 8 bytes boundary alignment and fill gap by nops" << endl;


					}

					if(isSubroutineHWloop && i_counter==HWloop_length.read()) {

						hw_loop_done=true;
						s << "/*last in the HW loop set  --- filled hw loop */" << endl;
						s << "L_" << hex << subRoutine->fStartAddress  << dec << "_hw_loop_end" << ":" << endl;

					}

					i_counter++;
				}
				if(isSubroutineHWloop ) {
					newInstr->Force32bitsOpcodeMode();
				} else {
					newInstr->RestoreRegularOpcodeMode();
				}
				newInstr->PrintMe(s);

				if(hw_loop_done) {
					s << "//HW loop done " << endl;
					//s << ".balignw 8,0x03a2 ; have 8 bytes boundary alignment and fill gap by snops" << endl;
				}


				if(iSize  && isChangeOfFlow) {
					numCurrBr+=1;
					numSubRoutine+= 1;

					TSubRoutine *subRoutine = new TSubRoutine();
					subRoutine->fStartAddress = newInstr->GetNextPC();

					cerr << "(" <<  numSubRoutine << " )call to subroutine for address " << hex << subRoutine->fStartAddress << dec <<endl;
					//add to pending list
					subRoutinePendingList.push_back(subRoutine);

				}
				//
				delete newInstr;


			} else {
				assert(0);//never get here -- the instruction should be generated
			}
		}

		//done list
		subRoutineDoneList.push_back(subRoutine);


		if(!isFPonly)  {
			//generate loop back for HW ot SW
			regM->SetReg(regM->GetLoopCountRegisterInd(),0);//will cause to add loop counter  to save/restore list
			if(!isSubroutineHWloop) {
				s << "/*loop back by SW*/" << endl;
				s <<dec  << "SUB R" << regM->GetLoopCountRegisterInd() <<  ",R" << regM->GetLoopCountRegisterInd()<< ",#1;" << endl;
				s <<dec  << "BNE ";
				s<< ".L" <<   hex << subRoutine->fStartAddress << dec << ";"<< endl;
				s <<dec  << "MOV R" <<  regM->GetLoopCountRegisterInd() <<  ",%low(0x" << hex <<1+ MAXLOOP_IT   << ");" << dec << endl;
			} else {
				if(!hw_loop_done) {
					s << "/*loop back by HW*/" << endl;

					assert(i_counter > 7);

					s << "/*second last in the HW loop <NOP>*/" << "//--- " << dec <<  i_counter << endl;
					s << ".balignw 8,0x01a2 ; have 8 bytes boundary alignment and fill gap by nops" << endl;
					s << "nop.l" << endl;


					s << "L_" << hex << subRoutine->fStartAddress  << dec << "_hw_loop_end" << ":" << endl;
					s << "nop.l" <<endl;


					//s << ".balignw 8,0x03a2 ; have 8 bytes boundary alignment and fill gap by snops" << endl;

				}
			}
		}

	}

	void CreateProgram(::std::ostream& s= ::std::cout) {

		if(!isGenDMA) {
			s << "//no DMA in the test" << endl;
			s << ".set ISR_DMA0, L_DMA_GLOBAL0" << endl;
			s << ".set ISR_DMA1, L_DMA_GLOBAL1" << endl;
		}

		s << ".section gen_start,\"a\",@progbits     ;" << endl;
		s << ".align 2" << endl;
		s << ".gen_start:" << endl;

		//init DMA
		if( isGenDMA ) {
			s << "BL .L_DMA_INIT_CONFIG0" << endl;
			s << "BL .L_DMA_INIT_CONFIG1" << endl;
		}

		regM->Init();

		vector<sc_uint<32> > v;
		memManager->FillStartMemRanges(v);

		//generate index registers to make easy the load/store access
		scv_smart_ptr<bool> IsManyLSOp;
		scv_bag<bool> IsManyLSOpDist;
		IsManyLSOpDist.add(true,80);
		IsManyLSOpDist.add(false,20);
		IsManyLSOp->set_mode(IsManyLSOpDist);
		if(isFPonly) { //we need the pointer to save the L/S after FP instructions
			IsManyLSOp.write(true);
		} else {
			IsManyLSOp->next();
		}
		unsigned numberLsRegs = ::min((unsigned int)(v.size()),(unsigned int)( NREGS/4));
		for(unsigned k = 0 ; k < numberLsRegs ; k++) {
			if(IsManyLSOp.read()) regM->CreateSetMemPointerReg(v[k]);
			cerr << "REG_LS" << k << endl;
		}
		//make rds before set up

		scv_smart_ptr<bool> SetRdsBeforeAllSetUp;
		if(isGenDMA || isGenInter){
			//prevent first DMA or timer ISR before  RDS from reset ISR
			SetRdsBeforeAllSetUp.write(false);
		} else {
			SetRdsBeforeAllSetUp->next();
		}

		if( SetRdsBeforeAllSetUp.read() == true) {
			Rds(s);
		}



		regM->PrintMe(s);

		regM->PrintMe(cerr);



		// configuration register
		s << "// set config " << endl;
		fConfigRegI->GenerateMe();
		fConfigRegI->PrintMe(s);

		//make rds after set up
		if(SetRdsBeforeAllSetUp.read() == false) {
			Rds(s);
		}

		//start test
		s << "RANDOM_START:" << endl;
		sc_uint<32> val;//need to restore reg 0 to prev generated random value;
		if(regM->GetReg(fConfigRegI->GetTmpRegId(),val)) {
			regM->SetReg(fConfigRegI->GetTmpRegId(),val);
			regM->PrintReg(fConfigRegI->GetTmpRegId(),val,s);
		} else {
			assert(0);//should defined through Init
		}


		//s <<dec  << "MOV R"<< regM->GetISRContextSaveRegInd()  <<  ",%low(" << hex << "ISR_SAVE_RESTORE_SPACE" << ");" << dec << endl;
		//s <<dec  << "MOVT R" << regM->GetISRContextSaveRegInd()  << ",%high(" << hex << "ISR_SAVE_RESTORE_SPACE" << ");"<< dec << endl;

		//jump to code
		TBranchInstr *newInstr=new TBranchInstr("branch");
		newInstr->BindRegsManager(regM);
		newInstr->BindMemManager(memManager);
		newInstr->SetCPC(PROGRAM_START+(NREGS*4));
		newInstr->fIsCond->disable_randomization();
		newInstr->fIsCond.write(false);

		if(! memManager->IsNoFetchMoreMemory()  ) {
			newInstr->GenerateMe();
			newInstr->PrintMe(s);
		} else {

			s <<dec  << "MOV R0"  <<  ",%low(END_OF_TEST);" << dec << endl;
			s <<dec  << "MOVT R0"  << ",%high(END_OF_TEST);"<< dec << endl;
			s << "JR R0; no instruction memory has been generated the test will stops at idle immediately " <<  endl;
		}

		//enf of test procedure
		EndofTest(s);

		//add ISR signature space
		//add load / store context for ISR
		s << ".align 4;\nISR_SIGNATURE:\n"<< endl;
		for ( unsigned n = 0 ; n < MAX_NUM_INTER ; n ++ ) {
			s << "//the ISR should set the word if it is triggered" << endl;
			s << ".fill " << 1 << ", 4 , 0x0" << endl;
			s << "//the DV sets the value below and host will compare it to value above" << endl;
			s << ".fill " << 1 << ", 4 , " ;
			unsigned expValFromTimer=0;
			if(n==1) {expValFromTimer = fConfigRegI->GetExpectedForTimer0();}
			if(n==2) {expValFromTimer = fConfigRegI->GetExpectedForTimer1();}
			s <<  expValFromTimer;
			s << "// times the ISR should occur  ";
			s << endl;
		}

		//add load / store context for ISR
		s << ".align 4;\nISR_SAVE_RESTORE_SPACE:\n"<< endl;
		for ( unsigned n = 0 ; n < MAX_NUM_INTER ; n ++ ) {
			s << ".fill " << NREGS  << ", 4 , 0xefbeadde	+"<< n<< ";;\n" << endl;
		}

		s << "/*0 - tmp to load/store the configuration and others; 1 - load/store R0, 2 - IVT , 3 -reserved*/" <<endl;

		s << "DMA_INTERRUPT_BUFFER0:" << endl;
		s << ".word 0x0" << endl;
		s << ".word 0x0" << endl;
		s << ".word ISR_DMA0" << endl;
		s << ".word 0xcccccccc" << endl;

		s << "DMA_INTERRUPT_BUFFER1:" << endl;
		s << ".word 0x0" << endl;
		s << ".word 0x0" << endl;
		s << ".word ISR_DMA1" << endl;
		s << ".word 0xdddddddd" << endl;


		TSubRoutine *subRoutine = new TSubRoutine();
		if(! memManager->IsNoFetchMoreMemory()  ) {

			subRoutine->fStartAddress = newInstr->GetNextPC();
			subRoutinePendingList.push_back(subRoutine);
		}

		//process the pending list
		while (subRoutinePendingList.size() != 0 ) {

			//create subroutine

			subRoutine = *(subRoutinePendingList.begin());
			FillSubroutine(subRoutine,s);
			subRoutinePendingList.erase(subRoutinePendingList.begin());




			//tie to next subroutine
			if(  subRoutinePendingList.size() !=0 ) {

				s <<dec  << "MOV R0"  <<  ",%low(0x" << hex <<(*(subRoutinePendingList.begin()))->fStartAddress  << ");" << dec << endl;
				s <<dec  << "MOVT R0"  << ",%high(0x" << hex <<(*(subRoutinePendingList.begin()))->fStartAddress  << ");"<< dec << endl;


			} else {

				s <<dec  << "MOV R0"  <<  ",%low(END_OF_TEST);" << dec << endl;
				s <<dec  << "MOVT R0"  << ",%high(END_OF_TEST);"<< dec << endl;

			}

			s << "JR R0; /*end of subroutine */" <<  endl;


		}


		GenerateRandomISR(s);

		GenerateRegularDMA(s);


		memManager->PrintAssemlySections(s);

		FillIVTable(s);

		CreateLDF();



	}


	void CreateLDF() {
		//create linker description file

		ofstream ldf;
		ldf.open ("test.ldf");
		ldf << "SECTIONS {" << endl;


		ldf << "\toutput_IVT 0x0 :   {test.o(IVT)   }" << endl;
		//ldf << "\toutput_gen_start 0x40 :   {   }" << endl;

		for ( vector<TSubRoutine *>::iterator it = subRoutineDoneList.begin() ; it !=  subRoutineDoneList.end() ; it++ ) {


			TSubRoutine *sr = *it;
			sc_uint<32> section_start=sr->fStartAddress;

			ldf << "\toutput_L_"  << hex << section_start <<  " 0x" << section_start << " :   {test.o(L_"
					<<   section_start  <<  ")  }"<< dec  << endl;

		}
		memManager->PrintLinkerSections(ldf);




		ldf << "\t output_L_ISR_SAVE0 0x40   : { test.o(L_ISR_SAVE0)     } " << endl;
		ldf << "\t output_L_ISR_SAVE1        : { test.o(L_ISR_SAVE1)     } " << endl;

		ldf << "\t output_L_ISR_RESTORE0     : { test.o(L_ISR_RESTORE0)     } " << endl;
		ldf << "\t output_L_ISR_RESTORE1     : { test.o(L_ISR_RESTORE1)     } " << endl;

		ldf << "\t output_L_DMA_GLOBAL0      : { test.o(L_DMA_GLOBAL0)     } " << endl;
		ldf << "\t output_L_DMA_GLOBAL1      : { test.o(L_DMA_GLOBAL1)     } " << endl;


		ldf << "\t output_L_DMA_INIT_CONFIG0      : { test.o(L_DMA_INIT_CONFIG0)     } " << endl;
		ldf << "\t output_L_DMA_INIT_CONFIG1      : { test.o(L_DMA_INIT_CONFIG1)     } " << endl;

		ldf << "\toutput_main :   {test.o(gen_start)  test.o(*)	   }" << endl;
		ldf << "}"<<endl;


		ldf.close();
	}


	void GenerateRandomISR(::std::ostream& s= ::std::cout) {

		if(isGenInter) {

			//create the interrupts

			//TODO
			InstrSel->keep_out(BRANCHING);//don't support branching in ISR

			InstrSel->keep_out(SWI);

			for( unsigned k = 0 ; k < MAX_NUM_INTER ; k++ ) {

				if(k>0/*not SWI*/) {
					InstrSel->keep_out(LOAD_STORE);  // TODO
					cerr << " stop gen load store for non swi ISR" << endl;
				}

				s << "/* ISR ------- "<< k  << "*/" << endl;
				pair<sc_uint<32>,sc_uint<32> > genRange_isr, genRange_save,genRange_restore;

				bool isIsrStartAddresGenerated=false;
				if(  memManager->GetGenRangebyJump( genRange_isr, MAX_SUB_ROUTINE_SIZE,MAX_SUB_ROUTINE_SIZE )		) {

					TSubRoutine*subRoutine = new TSubRoutine();
					subRoutine->sSize=64;// make ISR service shorter than regular subroutine due to save/restore stuff
					subRoutine->fStartAddress = genRange_isr.first;
					FillSubroutine(subRoutine,s);

					isIsrStartAddresGenerated=true;

				} else {
					//always invalidate all regs
					regM->InvalidateAllRegs();
				}

				//take care of save/restore R0 used for jumping
				regM->SetReg(0,0);//will cause to add R0 to save/restore list
				regM->SetReg(1,0);//will cause to add R1 to save/restore list
				regM->SetReg(regM->GetLoopCountRegisterInd() ,(1+ MAXLOOP_IT));


				//jump to restore
				s <<dec  << "MOV R0"  <<  ",%low(.L_ISR_RESTORE" << k << ");" << dec << endl;
				s <<dec  << "MOVT R0"  << ",%high(.L_ISR_RESTORE" << k << ");"<< dec << endl;
				s << "JR R0; /*end of ISR restore" << k << " */" <<  endl;
				//s << "B L_" << hex << subRoutine->fStartAddress << ";prevent put ISR @ non alignment" << dec << endl;



				s << ".section L_ISR_RESTORE" << k<< ",\"a\",@progbits;" << endl;
				s << ".align 2" << endl;
				s << ".L_ISR_RESTORE" << k << ":" << endl;


				s << "// restore interrupt context ===== "<< k << endl;

				//restore reti if nested
				if(fConfigRegI->isNestedAllowed(k)) {
					//gie
					s << "gid " << endl;
					s << "ldr r1 ,[R" << regM->GetISRContextSaveRegInd() << "," << k*NREGS + (NREGS-1)<< "]" << endl;
					s << "movts  iret,r1" <<endl;
				}

				//restore the status
				s << "ldr r0 ,[R" << regM->GetISRContextSaveRegInd() << "," << k*NREGS + (NREGS-2)<< "]" << endl;
				s << "movts  status,r0" <<endl;

				regM->PrintInterruptRestoreContext(k,s);


				s << "RTI;/*end of "<< k << "ISR*/" << endl;

				s << ".section L_ISR_SAVE" << k<< ",\"a\",@progbits;" << endl;
				s << ".align 2;" << endl;
				s << ".L_ISR_SAVE" << k << ":" << endl;



				s << "// save interrupt context ===== "<< k<< endl;

				regM->PrintInterruptSaveContext(k,s);
				//save the status
				s << "movfs r0 , status" <<endl;
				s << "str r0 ,[R" << regM->GetISRContextSaveRegInd() << "," << k*NREGS + (NREGS-2)<< "]" << endl;

				if(fConfigRegI->isNestedAllowed(k)) {

					//save reti
					s << "movfs r1 , iret" <<endl;
					s << "str r1 ,[R" << regM->GetISRContextSaveRegInd() << "," << k*NREGS + (NREGS-1)<< "]" << endl;

					//gie
					s << "gie " << endl;
				}

				if(k>0) {
					s << "//build signature" << endl;
					s <<dec  << "MOV R0"  <<  ",%low(ISR_SIGNATURE+" << 8*k << " );" << dec << endl;
					s <<dec  << "MOVT R0"  << ",%high(ISR_SIGNATURE+"<<  8*k <<  ");"<< dec << endl;
					s << "ldr r1 ,[r0,+0x0]" << endl;
					s << "add r1,r1,1" << endl;
					s << "str r1 ,[r0,+0x0]" << endl;

					s << " sub r1, r1,  " ;

					unsigned expValFromTimer=0;
					if(k==1) {expValFromTimer = fConfigRegI->GetExpectedForTimer0();}
					if(k==2) {expValFromTimer = fConfigRegI->GetExpectedForTimer1();}
					s <<  expValFromTimer<< endl;

					s << "BEQ __CONT_ISR" << k << endl;
				}

				if(k==1) {
					//reprogram timer0
					fConfigRegI->ReprogramTimer(1,  fConfigRegI->GenerateTimer0Value() , s );
				}
				if(k==2) {
					//reprogram timer1
					fConfigRegI->ReprogramTimer(2,  fConfigRegI->GenerateTimer1Value() , s );
				}
				s << "__CONT_ISR" << k << ":"<< endl;

				//jump to ISR
				if(  isIsrStartAddresGenerated) {
					s <<dec  << "MOV R0"  <<  ",%low(.L" << hex <<  genRange_isr.first << ");" << dec << endl;
					s <<dec  << "MOVT R0"  << ",%high(.L" << hex << genRange_isr.first  << ");"<< dec << endl;
				} else {
					s <<dec  << "MOV R0"  <<  ",%low(" <<  ".L_ISR_RESTORE" << k<< ");" << dec << endl;
					s <<dec  << "MOVT R0"  << ",%high(" <<  ".L_ISR_RESTORE" << k << ");"<< dec << endl;
				}

				//set lopp counter before generation the sub routine
				s << "//set loop counter for ISR" << endl;
				s <<dec  << "MOV R" <<  regM->GetLoopCountRegisterInd() <<  ",%low(0x" << hex <<1+ MAXLOOP_IT   << ");" << dec << endl;

				s << "JR R0; /*end of save */" <<  endl;


				//				 {
				//					//TODO no more memory for interrupts
				//					cerr << ";No more memory for interrupts generation" << endl;
				//					s << ";No more memory for interrupts generation " << endl;
				//					s << ".section L_ISR_SAVE" << k<< ",\"a\",@progbits;" << endl;
				//					s << ".align 2;" << endl;
				//					s << ".L_ISR_SAVE" << k << ":" << endl;
				//					s << "RTI;/*end of "<< k << "ISR*/" << endl;
				//				}
			}
		} else {
			for( unsigned k = 0 ; k < MAX_NUM_INTER ; k++ ) {
				s << ".section L_ISR_SAVE" << k<< ",\"a\",@progbits;" << endl;
				s << ".align 2;" << endl;
				s << ".L_ISR_SAVE" << k << ":" << endl;
				s << "RTI;/*NO ISR */" << endl;
			}
		}
	}


	//save all regular register to ISR SAVE/RESTORE space
	void DumpAllREgularRegsToMem(::std::ostream& s= ::std::cout) {

		s << "//dump all regs to ISR S/R place" << endl;

		s << "mov r0,0" << endl;
		s << "mov r1,0" << endl;

		s << "MOV R" << dec << regM->GetISRContextSaveRegInd()<< ",%low(ISR_SAVE_RESTORE_SPACE)" << endl;
		s << "MOVT R"<< dec << regM->GetISRContextSaveRegInd() << ",%high(ISR_SAVE_RESTORE_SPACE)" << endl;

		// overwrite the ISR S/R space for timer 0 -- ISR%1
		regM->DumpAllRegsToMem(s,NREGS*4/*bytes*/);

		// overwrite the ISR S/R space for timer 1 -- ISR%2
		regM->DumpAllRegsToMem(s,2*NREGS*4/*bytes*/);

	}

	//save all special register to ISR SAVE/RESTORE space -TODO
	void DumpAllSpecialRegsToMem(::std::ostream& s= ::std::cout) {

		s << "//dump special to ISR signature place"<< endl;


		s << "movfs r0,imask" << endl;
		s << "movfs r1,config" << endl;


		s << "MOV R7" << ",%low(ISR_SIGNATURE)" << endl;
		s << "MOVT R7" << ",%high(ISR_SIGNATURE)" << endl;

		//TODO DMA regs and others

		s << "strd r0, [R7,0]" << endl;
		s << "strd r0, [R7,1]" << endl;
		s << "strd r0, [R7,2]" << endl;
	}


	void CheckAndRestoreOrigISRSignature(::std::ostream& s= ::std::cout) {


		for(unsigned _DMAnum = 0 ; _DMAnum < 2 ; _DMAnum++) {
			//restore the DMA_INTERRUPT_BUFFER ( loc 1 , 2 )
			s << "MOV R" << dec << regM->GetISRContextSaveRegInd()<< ",%low(DMA_INTERRUPT_BUFFER"<< _DMAnum<< ")" << endl;
			s << "MOVT R"<< dec << regM->GetISRContextSaveRegInd() << ",%high(DMA_INTERRUPT_BUFFER"<< _DMAnum<< ")" << endl;

			s   << "MOV R0" <<  ",0" << endl;
			s << "str R0,[r" << regM->GetISRContextSaveRegInd() << ",1];" <<endl;

			s  << "MOV R0" <<  ",%low(ISR_DMA"<< _DMAnum << ")" << endl;
			s  << "MOVT R0" <<  ",%high(ISR_DMA"<< _DMAnum << ")" << endl;
			s << "str R0,[r" << regM->GetISRContextSaveRegInd() << ",2];" <<endl;

		}

		s << "MOV R" << dec << regM->GetISRContextSaveRegInd()<< ",%low(ISR_SIGNATURE)" << endl;
		s << "MOVT R"<< dec << regM->GetISRContextSaveRegInd() << ",%high(ISR_SIGNATURE)" << endl;

		s << "ldrd r0, [R" << regM->GetISRContextSaveRegInd() << ",0]" << endl;
		s << "sub r0,r0,r1" << endl;
		s << "strd r0, [R"<< regM->GetISRContextSaveRegInd() <<",0]" << endl;


		s << "ldrd r0, [R"<< regM->GetISRContextSaveRegInd() <<",1]" << endl;
		s << "sub r0,r0,r1" << endl;
		s << "strd r0, [R"<< regM->GetISRContextSaveRegInd() <<",1]" << endl;

		s << "ldrd r0, [R"<< regM->GetISRContextSaveRegInd() <<",2]" << endl;
		s << "sub r0,r0,r1" << endl;
		s << "strd r0, [R"<< regM->GetISRContextSaveRegInd() <<",2]" << endl;
	}

	//call rti to exit fron sync ISR
	void Rds(::std::ostream& s= ::std::cout) {
		unsigned tmpRgInd = 0;

		s << "//prepare return from reset ISR (ILAT set)" << endl;

		s << "MOV R" << tmpRgInd<< ",%low(RDS)" << endl;

		s << "movts iret,r"<< tmpRgInd << ";" << endl;

		s << "RTI" <<endl;

		s << ".global RDS" << endl;
		s << "RDS:" << endl;

	}

	void GenerateRegularDMA(::std::ostream& s= ::std::cout) {


		for ( unsigned i = 0 ; i < 2 ; i++ ) {

			if(!isGenDMA) {
				s << ".section L_DMA_GLOBAL"<< i <<",\"a\",@progbits     ;" <<endl;
				s << ".global "<< ".L_DMA_GLOBAL" << i << ";" << endl;
				s << ".L_DMA_GLOBAL" << i << ":"<<  endl;
				s << "B .L_DMA_GLOBAL" << i << "//never get here"<< endl;

			} else {


				cerr << "Do DMA " << i << endl;

				TDmaEngine *fDMA = new TDmaEngine(i);

				//Generate DMA engine
				fDMA->BindMemManager(memManager);
				fDMA->BindRegsManager(regM);

				fDMA->GenerateDescrList( subRoutineDoneList);

				s << ".set .L_000000000 , 0;" << endl;
				fDMA->PrintMe(s);

				//produce the expected results for DMA
				ofstream dmaf;

				dmaf.open ("dma_out.txt",ios_base::app);//

				fDMA->SimulateDMA(dmaf);
				dmaf.close();

				delete fDMA;
			}
		}
	}


	void FillIVTable(::std::ostream& s= ::std::cout) {
		//fill ivt table
		s << ".section IVT,\"a\",@progbits     ; " <<endl;
		s << ".global program_start;" << endl;
		s << "program_start:" << endl;
		s << "B .gen_start //SYNC - "<<endl;

		s << ".balign 4" << endl;
		s << "SW_EXCEPTION_ISR_LABEL:" << endl;
		s << "b SOFT_ISR_LABEL//=sw-exception" << endl;


		s << ".balign 4" << endl;
		s << "MEMPROTECT_ISR_LABEL://aka page miss" << endl;
		s << "B FINILIZE_TEST // used as last ISR in the test, called by host through ILAT set from DV monitor" << endl;


		s << ".balign 4" << endl;
		s << "TIMERS_ISR_LABEL:" << endl;
		if(isGenInter) {
			s << "B .L_ISR_SAVE1  //timer-expired 0"<<endl;
		} else {
			s << "trap 5  //no timer ISR 0"<<endl;
		}
		s << ".balign 4" << endl;
		if(isGenInter) {
			s << "B .L_ISR_SAVE2  //timer-expired 2"<<endl;
		} else {
			s << "trap 5 //no timer ISR 0"<<endl;
		}

		s << ".balign 4" << endl;
		s << "MESSAGE_ISR_LABEL:" << endl;
		s << "rti " << endl;


		s << ".balign 4" << endl;
		s << "DMA_ISR_LABEL:" << endl;
		if( !isGenDMA ) {
			s << "trap 5"  <<endl;
			s << ".balign 4" << endl;
			s << "trap 5"  <<endl;
		} else {
			s << "B .L_DMA_GLOBAL0"  <<endl;
			s << ".balign 4" << endl;
			s << "B .L_DMA_GLOBAL1"  <<endl;
		}

		s << ".balign 4" << endl;
		s << "WAND_ISR_LABEL:" << endl;
		s << "rti " << endl;

		s << ".balign 4" << endl;
		s << "SOFT_ISR_LABEL:" << endl;
		if(isGenInter) {
			s << "B .L_ISR_SAVE0 //SWI 0x20" << endl;
		} else {
			s << "rti" << endl;
		}
	}

	void EndofTest(::std::ostream& s= ::std::cout) {
		s << "END_OF_TEST: "<< endl;

		fConfigRegI->FinilizeTimers(s);

		s << "IDLE_END:" << endl;


		//save all registers to memory to allow comparison
		DumpAllREgularRegsToMem(s);

		s << "MOVFS R0,ILAT;" << endl;

		s << "_b_idle:" << endl;
		s << "		idle;" << endl;

		s << "B IDLE_END" << endl;

		s << "FINILIZE_TEST://not executed in cgen simulator" << endl;
		s << "MOVFS R0,IPEND;" << endl;
		s << "MOVFS R0,ILAT;" << endl;
		s << "MOVFS R0,CTIMER0;" << endl;

		//Will be done in the end of test from Host
		//s << "mov r0,0" << endl;

		//s << "movts config,r0"<<endl;
		//s << "movts meshconfig,r0"<<endl;
		//		//dump special regs
		//		DumpAllSpecialRegsToMem(s);

		//restore ISR signature to prevent mem diff
		CheckAndRestoreOrigISRSignature(s);




		s <<dec  << "MOV R0"  <<  ",%low(IDLE_END);" << dec << endl;
		s <<dec  << "MOVT R0"  << ",%high(IDLE_END);"<< dec << endl;
		s << "MOVTS IRET,R0;" << endl;

		s << "		RTI;" << endl;

	}

};


void usage() {
	cerr << "Usage " << " igen [ -no_dma] ( [ -no_fp ] | [-fp_only] )[ -mem <mem_ranges.txt> ] [ -seed <seed>] [ -nloops <nloops> ] [ -nsubr <nsubr>]"  << endl;
}

int sc_main (int argc, char** argv)
{

	unsigned long long seed=0;
	ifstream *memFile=0;

	for (unsigned int n = 1; n < argc; n++) {
		if(!strcmp(argv[n],"-seed")) {
			n+=1;
			if(n< argc) {
				seed = (std::atoll(argv[n]));
			} else {
				usage();
				return 3;
			}
			continue;
		}



		if(!strcmp(argv[n],"-chip_id")) {
			n+=1;
			if(n< argc) {
				chip_id = (std::atoll(argv[n]));
			} else {
				usage();
				return 3;
			}
			continue;
		}


		if(!strcmp(argv[n],"-nsubr")) {
			n+=1;
			if(n< argc) {
				MAX_NUNBER_SUBROUTINE = (std::atoll(argv[n]));

			} else {
				usage();
				return 3;
			}
			continue;
		}


		if(!strcmp(argv[n],"-nloops")) {
			n+=1;
			if(n< argc) {
				MAXLOOP_IT = (std::atoll(argv[n]));

			} else {
				usage();
				return 3;
			}
			continue;
		}

		if(!strcmp(argv[n],"-hw_loop_on")) {

			isHwLoopOn = true;
			continue;
		}

		if(!strcmp(argv[n],"-auto_dma_on")) {

			isAutoDMAOn = true;
			continue;
		}



		if(!strcmp(argv[n],"-bkpt_on")) {

			isBkptOn = true;
			continue;
		}
		if(!strcmp(argv[n],"-clock_gate_off")) {

			isClockGateOff = true;
			continue;
		}



		if(!strcmp(argv[n],"-integer_on")) {

			isIntegerModeOn = true;
			continue;
		}

		if(!strcmp(argv[n],"-no_fp")) {

			isGenFP = false;
			continue;
		}
		if(!strcmp(argv[n],"-fp_only")) {


			isFPonly = true;
			continue;
		}
		if(!strcmp(argv[n],"-no_dma")) {

			isGenDMA = false;
			continue;
		}
		if(!strcmp(argv[n],"-no_inter")) {

			isGenInter = false;
			continue;
		}

		if(!strcmp(argv[n],"-random_stop_resume_on")) {

			random_stop_resume_on = true;
			continue;
		}



		if(!strcmp(argv[n],"-mem")) {
			n+=1;
			if(n< argc) {
				memFile = new ifstream(argv[n]);
				if(!memFile->good()) {
					cerr << "Error: Can't access file " << argv[n] << endl;
					usage();
					return 4;
				} else {
					cerr << "setting memory range file to " << argv[n] << endl;
				}
			} else {
				usage();
				return 3;
			}
			continue;
		}
		//should not get here
		usage();
		return 55;

	}


	if(isFPonly && !isGenFP) {
		cerr << "ERROR: the -no_fp and -fp_only options are mutual exclusive" << endl;
		usage();
		exit(9);
	}

	scv_random::set_global_seed(seed);
	cerr << "Setting seed " << seed <<endl;


	scv_smart_ptr<unsigned> nREgToGenerate;
	scv_bag<unsigned> nREgToGenerateDist;
	nREgToGenerateDist.add(16);
	nREgToGenerateDist.add(32);
	nREgToGenerateDist.add(64);
	nREgToGenerate->set_mode(nREgToGenerateDist);
	nREgToGenerate->next();

	NREGS= nREgToGenerate.read();
	assert(NREGS == 16 || NREGS == 32 ||NREGS == 64 );


	MAX_NUNBER_SUBROUTINE = 1 + (rand() % MAX_NUNBER_SUBROUTINE);
	cerr << "MAXLOOP_IT "<< MAXLOOP_IT << endl;
	scv_smart_ptr<unsigned> mx_lop_gen; 	mx_lop_gen->keep_only(0,MAXLOOP_IT); mx_lop_gen->next();

	MAXLOOP_IT = mx_lop_gen.read();
	cerr << "MAXLOOP_IT "<< MAXLOOP_IT << endl;

	ofstream asmFile;
	asmFile.open ("test.s");
	asmFile << "/*-This assembly code has been generated by ATDSP Igen,\n";
	asmFile << "/* Igen: " << igen_ver << "*/" << endl;


	TProgram genProgram;

	if(memFile && memFile->good()) {
		cerr << "Reading mem ranges from file" << endl;
		genProgram.memManager->ReadInitialRangesList(*memFile);
		memFile->close();
	}
	cerr << "The available mem ranges are " << endl;
	genProgram.memManager->DumpInitialRangesList(cerr);

	//initialize memory ponters


	genProgram.CreateProgram(asmFile);

	asmFile.close();

	return 0;
}

