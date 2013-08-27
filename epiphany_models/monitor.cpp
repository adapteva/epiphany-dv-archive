/*
Copyright (C) 2011 Adapteva, Inc.
Contributed by Andreas Olofsson <andreas@adapteva.com>
Modified by Oleg Raikhman <support@adapteva.com>

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

#include <stdio.h>
#include "systemc.h"
#include "monitor.h"
#include <iostream>
#include <iomanip>

#include <map>

using namespace std;

#include "systemc.h"	// SystemC
#include "vmodel_utils.h"


#define REG_READ_VAL 0xc0ffe1f0



/********************************************************/
/* MONITOR FLOW:                                        */
/* -for now just monitor the cycle count                */
/********************************************************/
void Monitor::initialize(int t){
	timeout = t;
}
void Monitor::stop_test() {



	ofstream dutTranOutData("dut_trans_data_out.txt");

	assert(fExpectedDataFromDut);

	for( map<sc_uint<32> , sc_uint<8> >::iterator it =   fExpectedDataFromDut->begin(); it !=  fExpectedDataFromDut->end(); it++ ) {
		dutTranOutData<< setfill('0') << setw(8) << hex << long(it->first) << " " << long(it->second) << dec << endl;
	}

	dutTranOutData.close();

	sc_stop();
}

void Monitor::monitor_loop(void){

	int counter=0;

	while(1){
		if((counter%30000)==0){
			std::cout <<"MSG>>T=" << dec << counter  << " CYCLES, timeout " << timeout ;
			if(NCORES>2) {

				std::cout <<", NUMBER STIMULUS TO DRIVE from "<< hex << istim_north->GetBaseAddress() << dec <<  " is " << istim_north->GetStimulusPendingSize() ;
				cout << ", Number incoming transactions for last counted interval is " <<   istim_north->GetNumberTransactionRecordedAtHost() << std::endl;

				std::cout <<", NUMBER STIMULUS TO DRIVE from "<< hex << istim_south->GetBaseAddress() << dec <<  " is " << istim_south->GetStimulusPendingSize() ;
				cout << ", Number incoming transactions for last counted interval is " <<   istim_south->GetNumberTransactionRecordedAtHost() << std::endl;

				std::cout <<", NUMBER STIMULUS TO DRIVE from "<< hex << istim_east->GetBaseAddress() << dec <<  " is " << istim_east->GetStimulusPendingSize() ;
				cout << ", Number incoming transactions for last counted interval is " <<   istim_east->GetNumberTransactionRecordedAtHost() << std::endl;

				std::cout <<", NUMBER STIMULUS TO DRIVE from "<< hex << istim_west->GetBaseAddress() << dec <<  " is " << istim_west->GetStimulusPendingSize() ;
				cout << ", Number incoming transactions for last counted interval is " <<   istim_west->GetNumberTransactionRecordedAtHost() << std::endl;

			} else {
				std::cout <<", NUMBER STIMULUS TO DRIVE from "<< hex << istim_west->GetBaseAddress() << dec <<  " is " << istim_west->GetStimulusPendingSize() ;
				cout << ", Number incoming transactions for last counted interval is " <<   istim_west->GetNumberTransactionRecordedAtHost() << std::endl;
			}



			if(NCORES>2) {
				istim_north->ResetNumberTransactionRecordedAtHost();
				istim_south->ResetNumberTransactionRecordedAtHost();
				istim_east->ResetNumberTransactionRecordedAtHost();
				istim_west->ResetNumberTransactionRecordedAtHost();
			} else {
				istim_west->ResetNumberTransactionRecordedAtHost();
			}

		}

		if(counter > timeout){
			std::cout << "-------------------------------------------------" << endl;
			std::cout << "MSG>>TEST REACHED TIMEOUT LIMIT AFTER " << counter << " CYCLES, fetching some special registers" << endl;

			for(unsigned ncore = 0 ; ncore < NCORES;ncore++) {
				sc_uint<32> coreCtimer0 = GetDataByHost(ncore,CORE_CTIMER0,WORD_READ);
				sc_uint<32> coreCtimer1 = GetDataByHost(ncore,CORE_CTIMER1,WORD_READ);
				sc_uint<32> coreIpend = GetDataByHost(ncore,CORE_IPEND,WORD_READ);
				sc_uint<32> coreIlat = GetDataByHost(ncore,CORE_ILAT,WORD_READ);
				sc_uint<32> coreImask = GetDataByHost(ncore,CORE_IMASK,WORD_READ);
				sc_uint<32> dmaStatus0 = GetDataByHost(ncore,DMA0_STATUS,WORD_READ);
				sc_uint<32> dmaStatus1 = GetDataByHost(ncore,DMA1_STATUS,WORD_READ);
				sc_uint<32> coreSatus = GetDataByHost(ncore,CORE_STATUS,WORD_READ);

				cout <<  "MSG>>DV flow(TIMEOUT): Core("<< ncore <<") timers:" << hex << coreCtimer0 << " , " << coreCtimer1 << dec << endl;
				cout <<  "MSG>>DV flow(TIMEOUT): Core("<< ncore <<") coreSatus: "<< hex << coreSatus<< " ,ipend: "  << coreIpend << " ,ilat: " <<  coreIlat << " coreIlat & ~coreImask  " << (coreIlat & (~coreImask)) << dec << endl;
				cout <<  "MSG>>DV flow(TIMEOUT): Core("<< ncore <<") dmastatus 0,1: " << hex <<  dmaStatus0 << " ,  " << dmaStatus1 << dec << endl;

			}

			wait(10);

			stop_test();
		}
		counter++;
		wait();
	}
}




sc_uint<32> Monitor::GetDataByHost(unsigned ncore, sc_uint<32> addrFromHost, unsigned dataMode ) {

	assert(dataMode == BYTE_READ || dataMode == SHORT_READ || dataMode == WORD_READ);

	sc_uint<32> res=0;

	wait(1);

	sc_uint<32> spiAddrBack;

	if(NCORES>2 && spi_mode) {
		///// schedule  read and check response
		spiAddrBack=(fSpiDriver->GetBaseAddress() + ( (SPI_BACK_CORE_ROW_OFFSET) <<  26)   |  ( (SPI_BACK_CORE_COL_OFFSET) <<  20)  + SPI_RETURN_ADDR);
		StimulusEntry spiReadTran = StimulusEntry(spiAddrBack  , 0, (CHIP_BASE+CORE_NUM_2_ADDR(ncore)+addrFromHost)  , SPI_READ_CTRL_MODE, dataMode);
		fSpiDriver->AddStimulus(spiReadTran);
		//cout << "-------------------- Drive spi READ-----" << hex <<  spiReadTran << dec << "-------------------" << endl;
	} else {
		spiAddrBack=(istim_west->GetBaseAddress() + CORE_NUM_2_ADDR(ncore) +addrFromHost);
		istim_west->AddStimulus(StimulusEntry(spiAddrBack  , 0, (CHIP_BASE+CORE_NUM_2_ADDR(ncore)+addrFromHost)  , REGULAR_CTRL_MODE, dataMode));
	}


	unsigned maxCyclesToWaitX100=300;
	while(true) {
		wait(100);
		if( (fExpectedDataFromDut->find(spiAddrBack) != fExpectedDataFromDut->end())) {
			break;
		}
		if(maxCyclesToWaitX100 == 0 ) {
			cout << "SPI/HOST error: not getting read back transaction for more than 30000 cycles" << endl;
			wait(10);
			sc_stop();
		} else {
			maxCyclesToWaitX100--;
		}
	}
	unsigned dByteSize=4;
	if(dataMode == BYTE_READ ) dByteSize=1;
	if(dataMode == SHORT_READ ) dByteSize=2;

	for(unsigned i = 0 ; i < dByteSize ; i++ ) {
		assert(fExpectedDataFromDut->find(spiAddrBack+i) != fExpectedDataFromDut->end());
		res(7+i*8,i*8) =  (* fExpectedDataFromDut)[spiAddrBack+i];
		fExpectedDataFromDut->erase(fExpectedDataFromDut->find(spiAddrBack+i));
	}

	return res;
}

sc_uint<32> Monitor::SeDataByHost(unsigned ncore, sc_uint<32> adddrFromHost, sc_uint<32> data, unsigned dataMode) {

	assert(dataMode == BYTE_WRITE || dataMode == SHORT_WRITE || dataMode == WORD_WRITE);

	sc_uint<32> res=0;

	wait(1);
	sc_uint<32> spiAddrBack;

	if(NCORES>2 && spi_mode) {
		///// schedule  write and check response
		spiAddrBack=(fSpiDriver->GetBaseAddress() + ( (SPI_BACK_CORE_ROW_OFFSET) <<  26)   |  ( (SPI_BACK_CORE_COL_OFFSET) <<  20)  + SPI_RETURN_ADDR);
		StimulusEntry spiWriteEntry=StimulusEntry(spiAddrBack  , data, (CHIP_BASE+CORE_NUM_2_ADDR(ncore)+adddrFromHost)  , REGULAR_CTRL_MODE, dataMode);
		fSpiDriver->AddStimulus(spiWriteEntry);
		//cout << "-------------------- Drive spi WRITE-----" << hex <<  spiWriteEntry << dec << "-------------------" << endl;
	} else {
		istim_west->AddStimulus(StimulusEntry(0  , data, (CHIP_BASE+CORE_NUM_2_ADDR(ncore)+adddrFromHost)  , REGULAR_CTRL_MODE, dataMode));
	}



	if(NCORES>2 && spi_mode) {
		unsigned maxCyclesToWaitX100=300;

		while(true) {
			wait(100);
			if( (fExpectedDataFromDut->find(spiAddrBack) != fExpectedDataFromDut->end())) {
				break;
			}
			if(maxCyclesToWaitX100 == 0 ) {
				cout << "SPI/HOST error: not getting write back transaction for more than 30000 cycles" << endl;
				wait(10);
				sc_stop();
			} else {
				maxCyclesToWaitX100--;
			}
		}
		res =  (* fExpectedDataFromDut)[spiAddrBack];
		fExpectedDataFromDut->erase(fExpectedDataFromDut->find(spiAddrBack));
	}

	return res;
}



void Monitor::StdioHandler() {

	/* Enum declaration for trap instruction dispatch code.  see sim/epiphany/epiphany-desc.*/
	enum TRAP_CODES {
		TRAP_WRITE=0,
				TRAP_READ=1,
				TRAP_OPEN=2,
				TRAP_EXIT=3,
				TRAP_PASS=4,
				TRAP_FAIL=5,
				TRAP_CLOSE=6,
				TRAP_OTHER=7
	}  ;

	extern void HandleStopSignal(int );
	while(1) {
		wait(5000);

		for(unsigned ncore = 0 ; ncore < NCORES;ncore++) {
			sc_uint<32> coreDebug = GetDataByHost(ncore,CORE_DEBUG,WORD_READ);
			if(coreDebug[0]!=DEBUG_RESUME) {

				sc_uint<32> corePc = GetDataByHost(ncore,CORE_PC,WORD_READ);
				sc_uint<32> prevPc = corePc-2 ;//address of bkpt
				//cout << "MSG>>DV flow: DEBUG reg (" << hex<< coreDebug << "), Core halted in the (m)bkpt/trap instruction, PC " <<  (prevPc) <<endl;


				sc_uint<32> trapOpcode;

				if(IS_ADDRESS_INTERNAL(prevPc) || IS_ADDRESS_IN_CHIP_RANGE(prevPc)) {

					if( IS_ADDRESS_IN_CHIP_RANGE(prevPc)){
						cerr <<  "\nERROR: TRAP stopped at non valid memory address " << hex << prevPc << dec << endl;
					}

					if(IS_ADDRESS_INTERNAL(prevPc)) {
						trapOpcode = GetDataByHost(ncore,(prevPc),SHORT_READ);
					}

				} else {
					if(fExpectedDataFromDut->find(prevPc) == fExpectedDataFromDut->end() ||
							fExpectedDataFromDut->find(prevPc+1) == fExpectedDataFromDut->end() 	) {
						cerr <<  "\nERROR: TRAP stopped at non existing memory address " << hex << prevPc << dec << endl;

						trapOpcode=M_BKPT_INSTR_OPCODE;

						wait(10);
						sc_stop();
					} else {
						trapOpcode(7,0) = (*fExpectedDataFromDut)[prevPc];
						trapOpcode(15,8) = (*fExpectedDataFromDut)[prevPc+1];
					}
				}

				bool stoppedAtTrap = (trapOpcode(9,0) == TRAP_INSTR_OPCODE);
				char *buf;
				unsigned addrInMem ;
				if(stoppedAtTrap) {
					sc_uint<6> trapNumber = trapOpcode(15,10);
					sc_uint<32> r0 , r1 , r2;
					switch(trapNumber) {
					case TRAP_WRITE :

						r0 = GetDataByHost(ncore,CORE_R0,WORD_READ);//chan
						r1 = GetDataByHost(ncore,CORE_R1,WORD_READ);///addr
						r2 = GetDataByHost(ncore,CORE_R2,WORD_READ);//len

						//cerr << " ----- Trap WRITE Ignoring ...  LEN "<< hex << r2  << endl;

						buf = (char*)malloc(r2+1) ;
						for(unsigned i = 0 ; i < r2; i++) {
							addrInMem = r1 + i;
							if( IS_ADDRESS_IN_CHIP_RANGE(addrInMem)){
								buf[i]  = (char)GetDataByHost(ncore,addrInMem,BYTE_READ);
							} else {
								if(fExpectedDataFromDut->find(addrInMem) == fExpectedDataFromDut->end() ) {
									buf[i] = '\0';
								} else {
									buf[i] =  (char)((*fExpectedDataFromDut)[addrInMem]);
								}
							}
						}
						cerr << "__EPIPHANY___STDIO____ ";
						cerr.write(buf,r2);
						SeDataByHost( ncore,  CORE_R0, r2,WORD_WRITE);

						wait(100);

						free(buf);


//								sim_read (sd, PARM1, buf, PARM2); /* read PARM2 bytes of cpu memory at PARM1 into buf */
//								result = sim_io_write (sd, PARM0, buf, PARM2);
//								zfree (buf);

						break;

					case TRAP_READ:

						cerr << " Trap READ Ignoring ...  " << endl;

						break;
					case TRAP_OPEN:
						cerr << " Trap OPEN Ignoring ...  " << endl;
						break;
					case TRAP_EXIT:

						cerr << "(GDB SERVER/BATCH MODE): Exit called " << endl;
						HandleStopSignal(1);

						break;
					case TRAP_PASS:
						cerr << " Trap 4 PASS " << endl;
						break;
					case TRAP_FAIL:
						cerr << " Trap 5 FAIL " << endl;
						break;
					case TRAP_CLOSE:
						SeDataByHost( ncore,  CORE_R0, 0,WORD_WRITE);
						//cerr << " Trap CLOSE Ignoring ...  " << endl;
						break;
					default:
						break;
					}

					//resume
					//cout << "MSG>>DV flow: opcode " << hex<<  trapOpcode <<" (from TRAP) DEBUG cmd scheduled for core " <<dec << ncore << endl;
					SeDataByHost( ncore,  CORE_DEBUGCMD, DEBUG_RESUME,WORD_WRITE);

				} else {
					cerr <<  "\nERROR: TRAP opcode " << hex <<trapOpcode << " doesn't  match (9,0) " << TRAP_INSTR_OPCODE << dec << endl;
					wait(10);
					sc_stop();
				}


				wait(100);//hope the core will be resumed
			}

		}

	}
}


void Monitor::CheckCoreStatus() {

	unsigned numTimesResumed=0;

	wait(100);

	assert(fExpectedDataFromDut);

	//wait until the end of load
	while(istim_north->GetStimulusPendingSize()) {
		wait(100);
	}
	while(istim_south->GetStimulusPendingSize()) {
		wait(100);
	}
	while(istim_east->GetStimulusPendingSize()) {
		wait(100);
	}
	while(istim_west->GetStimulusPendingSize()) {
		wait(100);
	}

	//
	wait(10);

	bool *coreDone = new bool [NCORES];
	for(unsigned j = 0 ; j < NCORES;j++) {
		coreDone[j]=false;
	}


	extern bool random_stop_resume_on;

	//suspend and resume the cores
	if(random_stop_resume_on) {

		unsigned max_NumStopAndResumes= rand() %  12;

		for(unsigned int nStopAndResumes = 0 ; nStopAndResumes < max_NumStopAndResumes; nStopAndResumes++) {
			wait(10);
			for(unsigned ncore = 0 ; ncore < 1;ncore++) {

				unsigned int do_wait_debug_suspend = rand() % 50;
				wait(do_wait_debug_suspend+1);


				cout << "MSG>>DV flow: Suspend DEBUG cmd scheduled for core " << ncore << " Iteration ==N%==================  "<<  nStopAndResumes<< endl;
				SeDataByHost( ncore,  CORE_DEBUGCMD, DEBUG_HALT,WORD_WRITE);

				//wait for halt state
				while(true) {

					wait(10);
					//fLoaderHostDriver->AddStimulus(StimulusEntry( (fLoaderHostDriver->GetBaseAddress() + CORE_NUM_2_ADDR(ncore) +CORE_DEBUG) , REGULAR_CTRL_MODE, (CHIP_BASE+CORE_NUM_2_ADDR(ncore)+CORE_DEBUG)  , 0, WORD_READ, 0/*TODO delay check debug*/));
					//fExpectedDataFromDut->erase(fExpectedDataFromDut->find((fLoaderHostDriver->GetBaseAddress()+ CORE_NUM_2_ADDR(ncore) + CORE_DEBUG)));
					sc_uint<32> valueOfDebugReg = GetDataByHost(ncore,CORE_DEBUG,WORD_READ);

					bool ret= ((valueOfDebugReg[0] == DEBUG_HALT) && (valueOfDebugReg[1] == OUT_TRAN_FALSE) );
					if(ret) {
						cout << "MSG>>DV  flow: Core[" << ncore <<  "] halted "<< endl;


						//read ILAT
						//sc_uint<32> coreIlat = GetDataByHost(ncore,CORE_ILAT,WORD_READ);
						sc_uint<32> dmaStatus0 = GetDataByHost(ncore,DMA0_STATUS,WORD_READ);
						sc_uint<32> dmaStatus1 = GetDataByHost(ncore,DMA1_STATUS,WORD_READ);
						sc_uint<32> coreIpend = GetDataByHost(ncore,CORE_IPEND,WORD_READ);

						cout << "MSG>>DV  flow: Core[" << ncore <<  "] IPEND "<< hex<< coreIpend;
						cout << " DMA0_IDLE "<< dmaStatus0<<	" DMA1_IDLE "<< hex<< dmaStatus1 << dec << endl;

						//							if( (coreIpend >1) &&  (dmaStatus0(3,0) == 5 || dmaStatus1(3,0) == 5 )) {
						//								cout << "GOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOD" << endl;
						//								sc_stop();
						//							}

						break;
					} else {
						cout << "MSG>>DV  flow: Core[ " << ncore <<  "] wait for HALT,  valueOfDebugReg " << hex <<valueOfDebugReg << dec<< endl;
						wait(100);//TODO
					}


				}

				unsigned int do_wait_debug_resume = rand() % 50;
				wait(do_wait_debug_resume+1);


				//coreSetDebug = StimulusEntry( (fLoaderHostDriver->GetBaseAddress()+ CORE_NUM_2_ADDR(ncore) + CORE_DEBUGCMD) , DEBUG_RESUME, (CHIP_BASE+ CORE_NUM_2_ADDR(ncore) + CORE_DEBUGCMD )  , MODIFY_READ_MODE, WORD_READ, 0/*TODO delay after resume*/);
				//fLoaderHostDriver->AddStimulus(coreSetDebug);


				cout << "MSG>>DV flow: Resume DEBUG cmd scheduled for core " << ncore << endl;
				SeDataByHost( ncore,  CORE_DEBUGCMD, DEBUG_RESUME,WORD_WRITE);


				//wait for write response (HALT)


				//wait for resume state
				while(true) {

					wait(10);//TODO
					//fLoaderHostDriver->AddStimulus(StimulusEntry( (fLoaderHostDriver->GetBaseAddress() + CORE_NUM_2_ADDR(ncore) +CORE_DEBUG) , REGULAR_CTRL_MODE, (CHIP_BASE+CORE_NUM_2_ADDR(ncore)+CORE_DEBUG)  , 0, WORD_READ, 0/*TODO delay check debug*/));
					sc_uint<32> valueOfDebugReg = GetDataByHost(ncore,CORE_DEBUG,WORD_READ);

					bool ret= ((valueOfDebugReg[0] == DEBUG_RESUME)  );
					if(ret) {
						cout << "MSG>>DV  flow: Core[" << ncore <<  "] resumed "<< endl;
						break;
					} else {
						cout << "MSG>>DV  flow: Core[ " << ncore <<  "] wait for RESUMED,  valueOfDebugReg " << hex <<valueOfDebugReg << dec<< endl;

					}

				}

				wait(10);

			}//for(unsigned ncore = 0 ; ncore < 1;ncore++)

		}//unsigned int nStopAndResumes = 0 ; nStopAndResumes < max_NumStopAndResumes; nStopAndResumes+
	}//random_stop_resume_on

	cout << "MSG>>DV flow: All test stimulus are driven ... waiting for idle for all  " << NCORES << endl;


	bool chipDone=false;

	while(!chipDone) {// looking for chip done


		wait(3000);

		for(unsigned ncore = 0 ; ncore < NCORES;ncore++) {


			wait(500);

			if(coreDone[ncore] == false) {

				cout << "MSG>>DV flow: added statuses read for ncore " << ncore << endl;
				sc_uint<32> coreCtimer0 = GetDataByHost(ncore,CORE_CTIMER0,WORD_READ);
				sc_uint<32> coreCtimer1 = GetDataByHost(ncore,CORE_CTIMER1,WORD_READ);
				sc_uint<32> coreIpend = GetDataByHost(ncore,CORE_IPEND,WORD_READ);
				sc_uint<32> coreIlat = GetDataByHost(ncore,CORE_ILAT,WORD_READ);
				sc_uint<32> coreImask = GetDataByHost(ncore,CORE_IMASK,WORD_READ);
				sc_uint<32> dmaStatus0 = GetDataByHost(ncore,DMA0_STATUS,WORD_READ);
				sc_uint<32> dmaStatus1 = GetDataByHost(ncore,DMA1_STATUS,WORD_READ);
				sc_uint<32> coreSatus = GetDataByHost(ncore,CORE_STATUS,WORD_READ);


				wait(100);
				/// schedule status read and check response

				cout <<  "MSG>>DV flow: Core("<< ncore <<") timers:" << hex << coreCtimer0 << " , " << coreCtimer1 << dec << endl;
				cout <<  "MSG>>DV flow: Core("<< ncore <<") coreSatus: "<< hex << coreSatus<< " ,ipend: "  << coreIpend << " ,ilat: " <<  coreIlat << " coreIlat & ~coreImask  " << (coreIlat & (~coreImask)) << dec << endl;
				cout <<  "MSG>>DV flow: Core("<< ncore <<") dmastatus 0,1: " << hex <<  dmaStatus0 << " ,  " << dmaStatus1 << dec << endl;

				bool CoreDoneState = ((coreSatus[0] == 0)
						&& ( (coreIlat & (~coreImask)) ==0)//ISR is masked
						&& coreCtimer0==0 && coreCtimer1==0
						&&  coreIpend==0
						&& (dmaStatus0(3,0) ==DMA_IDLE )
						&& (dmaStatus1(3,0) ==DMA_IDLE )
				);

				if( (dmaStatus0(3,0) ==DMA_ERROR )
						|| (dmaStatus1(3,0) ==DMA_ERROR )) {
					cout << "ERROR : DMA error reported for core  " << ncore << endl;
					fExpectedDataFromDut->clear();
					wait(10);
					sc_stop();
				}

				if(CoreDoneState)  {
					cout << "MSG>>DV flow: Core (" << ncore << ") STOPPED AT IDLE    status[0], (ilat& ~imask) , timers, ipend ==0, dma, going to FINILIZE stage ( page miss ISR ILAT set) " << endl;


					SeDataByHost( ncore,  CORE_ILATST, PAGE_MISS_ILAT_BIT,WORD_WRITE);

					//FINILIZE STAGE
					while(true) {
						wait(100);


						//check status again
						sc_uint<32> coreSatus = GetDataByHost(ncore,CORE_STATUS,WORD_READ);

						if(coreSatus[0] ==0) {
							wait(1);
							break;
						} else {
							cout << "MSG>>DV flow: Core (" << ncore << ")  , for PAGE MISS done "<< coreSatus << endl;
						}

						cout << "MSG>>DV flow: Core (" << ncore << ")  ,Finalize the core , status[0]= "<< coreSatus[0] << " ,waiting for 0"<< endl;
					}
					coreDone[ncore]=true;
					cout << "MSG>>DV flow: Core (" << ncore << ") DONE !!!" << endl;
				} else {
					sc_uint<32> coreDebug = GetDataByHost(ncore,CORE_DEBUG,WORD_READ);
					// core not in idle ... check if halted in bkpt
					if(coreDebug[0]!=DEBUG_RESUME) {

						sc_uint<32> corePc = GetDataByHost(ncore,CORE_PC,WORD_READ);
						sc_uint<32> prevPc = corePc-2 ;//address of bkpt
						cout << "MSG>>DV flow: DEBUG reg (" << hex<< coreDebug << "), Core halted in the (m)bkpt/trap instruction, PC " <<  (corePc);
						if( coreDebug[4] == 0 /*not stopped by other*/  /* && corePc > 0x100*/ /*not in IVT table */) {


							sc_uint<32> bkptOpcode;

							if(IS_ADDRESS_INTERNAL(prevPc) || IS_ADDRESS_IN_CHIP_RANGE(prevPc)) {

								if( IS_ADDRESS_IN_CHIP_RANGE(prevPc)){
									//bkpt in other core .. ignore checking
									bkptOpcode=M_BKPT_INSTR_OPCODE;
								}

								if(IS_ADDRESS_INTERNAL(prevPc)) {
									bkptOpcode = GetDataByHost(ncore,(prevPc),SHORT_READ);
								}

							} else {
								if(fExpectedDataFromDut->find(prevPc) == fExpectedDataFromDut->end() ||
										fExpectedDataFromDut->find(prevPc+1) == fExpectedDataFromDut->end() 	) {
									cerr <<  "\nERROR: (M)BKPT stopped at non existing memory address " << hex << prevPc << dec << endl;

									bkptOpcode=M_BKPT_INSTR_OPCODE;

									wait(10);
									sc_stop();
								} else {
									bkptOpcode(7,0) = (*fExpectedDataFromDut)[prevPc];
									bkptOpcode(15,8) = (*fExpectedDataFromDut)[prevPc+1];
								}
							}

							cout << " , opcode " << bkptOpcode;
							if(bkptOpcode != M_BKPT_INSTR_OPCODE /*&& bkptOpcode != TRAP_INSTR_OPCODE*/ && bkptOpcode != DV_BKPT_INSTR_OPCODE) {
								cerr <<  "\nERROR: opcode should match the " << hex << DV_BKPT_INSTR_OPCODE << "/" << M_BKPT_INSTR_OPCODE << dec << endl;
								wait(10);
								sc_stop();
							}

						}
						cout << " @core "<< ncore<< dec << endl;

						cout << "MSG>>DV flow: Resume "<< numTimesResumed++ <<" (from BKPT) DEBUG cmd scheduled for core " << ncore << endl;
						SeDataByHost( ncore,  CORE_DEBUGCMD, DEBUG_RESUME,WORD_WRITE);

						wait(100);//hope the core will be resumed

					} else {
						//stop due to NOT bkpt
					}
					wait(500);
				}// Core not Done

			}//if(coreDone[ncore] == false)
		}//go all cores

		chipDone=coreDone[0];
		for(unsigned i = 0 ; i < NCORES;i++) {
			chipDone = chipDone && coreDone[i];
		}

	}//chip done

	assert(chipDone);
	cout << "MSG>>DV flow:-----------All Cores programs Done  " << endl;

	//We have the chip finished the test. while can be hosts with pending stimulus,
	while(true) {
		wait(1000);
		bool host_done = isHostDriversDone();
		if(host_done) {
			wait(2000);
			//check again
			host_done = isHostDriversDone();
			if(host_done) {
				break;
			} else {
				cout << "MSG>>DV flow WARNING:-----------Chip Done && There are pending transactions from hosts (2nd iteration) !!!!!!!!!!!!!!!!" << endl;
			}

		} else {
			cout << "MSG>>DV flow:-----------Chip Done && There are pending transactions from hosts " << endl;
		}
	}

	wait(100);
	cout << "MSG>>DV flow:-----------Chip and Hosts Done && !testDone-----------------DispathSignOff-----------------" << endl;
	//TODO SPI

	if(NCORES>2) {
		istim_north->DispathSignOff();
		istim_south->DispathSignOff();
		istim_east->DispathSignOff();
		istim_west->DispathSignOff();
	} else {
		istim_west->DispathSignOff();
	}



	//We have the signoff scheduled wait all read come back
	while(true) {
		wait(1000);
		bool host_done = isHostDriversDone();
		if(host_done) {
			cout << "MSG>>DV flow:----------- setting clock gating OFF " << endl;
			break;
		}
	}
	wait(1000);

	//turn off the clock gating
	for(unsigned ncore = 0 ; ncore < NCORES;ncore++) {
		wait(100);

		if(NCORES>2) {

			{
				StimulusEntry coreCfgWrite = StimulusEntry( (istim_north->GetBaseAddress()+ CORE_NUM_2_ADDR(ncore) + CORE_CONFIG) , /*reset core config*/ 0, (CHIP_BASE+ CORE_NUM_2_ADDR(ncore) + CORE_CONFIG )  , 0, WORD_WRITE);
				StimulusEntry memCfgWrite = StimulusEntry( (istim_north->GetBaseAddress()+ CORE_NUM_2_ADDR(ncore) + MESH_CONFIG) , /*reset mem config*/ 0, (CHIP_BASE+ CORE_NUM_2_ADDR(ncore) + MESH_CONFIG )  , 0, WORD_WRITE);
				coreCfgWrite.delay=200;
				memCfgWrite.delay=200;
				istim_north->AddStimulus(coreCfgWrite);
				istim_north->AddStimulus(memCfgWrite);
			}


			{
			StimulusEntry coreCfgWrite = StimulusEntry( (istim_south->GetBaseAddress()+ CORE_NUM_2_ADDR(ncore) + CORE_CONFIG) , /*reset core config*/ 0, (CHIP_BASE+ CORE_NUM_2_ADDR(ncore) + CORE_CONFIG )  , 0, WORD_WRITE);
			StimulusEntry memCfgWrite = StimulusEntry( (istim_south->GetBaseAddress()+ CORE_NUM_2_ADDR(ncore) + MESH_CONFIG) , /*reset mem config*/ 0, (CHIP_BASE+ CORE_NUM_2_ADDR(ncore) + MESH_CONFIG )  , 0, WORD_WRITE);
			coreCfgWrite.delay=200;
			memCfgWrite.delay=200;
			istim_south->AddStimulus(coreCfgWrite);
			istim_south->AddStimulus(memCfgWrite);
			}
			{
				StimulusEntry coreCfgWrite = StimulusEntry( (istim_east->GetBaseAddress()+ CORE_NUM_2_ADDR(ncore) + CORE_CONFIG) , /*reset core config*/ 0, (CHIP_BASE+ CORE_NUM_2_ADDR(ncore) + CORE_CONFIG )  , 0, WORD_WRITE);
				StimulusEntry memCfgWrite = StimulusEntry( (istim_east->GetBaseAddress()+ CORE_NUM_2_ADDR(ncore) + MESH_CONFIG) , /*reset mem config*/ 0, (CHIP_BASE+ CORE_NUM_2_ADDR(ncore) + MESH_CONFIG )  , 0, WORD_WRITE);
				coreCfgWrite.delay=200;
				memCfgWrite.delay=200;
				istim_east->AddStimulus(coreCfgWrite);
				istim_east->AddStimulus(memCfgWrite);
			}
			{
				StimulusEntry coreCfgWrite = StimulusEntry( (istim_west->GetBaseAddress()+ CORE_NUM_2_ADDR(ncore) + CORE_CONFIG) , /*reset core config*/ 0, (CHIP_BASE+ CORE_NUM_2_ADDR(ncore) + CORE_CONFIG )  , 0, WORD_WRITE);
				StimulusEntry memCfgWrite = StimulusEntry( (istim_west->GetBaseAddress()+ CORE_NUM_2_ADDR(ncore) + MESH_CONFIG) , /*reset mem config*/ 0, (CHIP_BASE+ CORE_NUM_2_ADDR(ncore) + MESH_CONFIG )  , 0, WORD_WRITE);
				coreCfgWrite.delay=200;
				memCfgWrite.delay=200;
				istim_west->AddStimulus(coreCfgWrite);
				istim_west->AddStimulus(memCfgWrite);
			}
		} else {
			StimulusEntry coreCfgWrite = StimulusEntry( (istim_west->GetBaseAddress()+ CORE_NUM_2_ADDR(ncore) + CORE_CONFIG) , /*reset core config*/ 0, (CHIP_BASE+ CORE_NUM_2_ADDR(ncore) + CORE_CONFIG )  , 0, WORD_WRITE);
			StimulusEntry memCfgWrite = StimulusEntry( (istim_west->GetBaseAddress()+ CORE_NUM_2_ADDR(ncore) + MESH_CONFIG) , /*reset mem config*/ 0, (CHIP_BASE+ CORE_NUM_2_ADDR(ncore) + MESH_CONFIG )  , 0, WORD_WRITE);
			coreCfgWrite.delay=200;
			memCfgWrite.delay=200;
			istim_west->AddStimulus(coreCfgWrite);
			istim_west->AddStimulus(memCfgWrite);
		}


		wait(100);
	}

	wait(1000);
	cout << "MSG>>DV flow:----------- append the R0 read at end of the test for core0,1... " << endl;
	for(unsigned i = 0 ; i < NCORES;i++) {
		if(NCORES>2) {
			istim_north->AddStimulus(StimulusEntry( (istim_north->GetBaseAddress()+ CORE_NUM_2_ADDR(i) + CORE_R0 + 4*i) , REG_READ_VAL, (CHIP_BASE+ CORE_NUM_2_ADDR(i) + CORE_R0 + 4*i)  , 0, WORD_WRITE,/*delay*/ 100));
			istim_south->AddStimulus(StimulusEntry( (istim_south->GetBaseAddress()+ CORE_NUM_2_ADDR(i) + CORE_R16 + 4*i) , REG_READ_VAL, (CHIP_BASE+ CORE_NUM_2_ADDR(i) + CORE_R16 + 4*i)  , 0, WORD_WRITE,/*delay*/ 100));
			istim_east->AddStimulus(StimulusEntry( (istim_east->GetBaseAddress()+ CORE_NUM_2_ADDR(i) + CORE_R32 + 4*i) , REG_READ_VAL, (CHIP_BASE+ CORE_NUM_2_ADDR(i) + CORE_R32 + 4*i)  , 0, WORD_WRITE,/*delay*/ 100));
			istim_west->AddStimulus(StimulusEntry( (istim_west->GetBaseAddress()+ CORE_NUM_2_ADDR(i) + CORE_R48 + 4*i) , REG_READ_VAL, (CHIP_BASE+ CORE_NUM_2_ADDR(i) + CORE_R48 + 4*i)  , 0, WORD_WRITE,/*delay*/ 100));
		} else {
			istim_west->AddStimulus(StimulusEntry( (istim_west->GetBaseAddress()+ CORE_NUM_2_ADDR(i) + CORE_R48 + 4*i) , REG_READ_VAL, (CHIP_BASE+ CORE_NUM_2_ADDR(i) + CORE_R48 + 4*i)  , 0, WORD_WRITE,/*delay*/ 100));
		}
	}
	wait(1000);

	for(unsigned i = 0 ; i < NCORES;i++) {
		if(NCORES>2) {
			istim_north->AddStimulus(StimulusEntry( (istim_north->GetBaseAddress()+ CORE_NUM_2_ADDR(i) + CORE_R0 + 4*i) , 0x0, (CHIP_BASE+ CORE_NUM_2_ADDR(i) + CORE_R0 + 4*i)  , 0, WORD_READ));
			istim_south->AddStimulus(StimulusEntry( (istim_south->GetBaseAddress()+ CORE_NUM_2_ADDR(i) + CORE_R16 + 4*i) , 0x0, (CHIP_BASE+ CORE_NUM_2_ADDR(i) + CORE_R16 + 4*i)  , 0, WORD_READ));
			istim_east->AddStimulus(StimulusEntry( (istim_east->GetBaseAddress()+ CORE_NUM_2_ADDR(i) + CORE_R32 + 4*i) , 0x0, (CHIP_BASE+ CORE_NUM_2_ADDR(i) + CORE_R32 + 4*i)  , 0, WORD_READ));
			istim_west->AddStimulus(StimulusEntry( (istim_west->GetBaseAddress()+ CORE_NUM_2_ADDR(i) + CORE_R48 + 4*i) , 0x0, (CHIP_BASE+ CORE_NUM_2_ADDR(i) + CORE_R48 + 4*i)  , 0, WORD_READ));
		} else {
			istim_west->AddStimulus(StimulusEntry( (istim_west->GetBaseAddress()+ CORE_NUM_2_ADDR(i) + CORE_R48 + 4*i) , 0x0, (CHIP_BASE+ CORE_NUM_2_ADDR(i) + CORE_R48 + 4*i)  , 0, WORD_READ));

		}
	}

	wait(1000);


	//waiting last all signoff transactions driven


	//RO get back to host
	for(unsigned ncore = 0 ; ncore < NCORES;ncore++) {

		wait(10);

		bool isCoreIdEndofTest = false;

		while(isCoreIdEndofTest == false) {

			wait(500);

			isCoreIdEndofTest = false;

			if(NCORES>2) {
				bool isCoreIdEndofTestistim_north =  ( fExpectedDataFromDut->find((istim_north->GetBaseAddress()+ CORE_NUM_2_ADDR(ncore) + CORE_R0 + 4*ncore)) != fExpectedDataFromDut->end());
				bool isCoreIdEndofTestistim_south =  ( fExpectedDataFromDut->find((istim_south->GetBaseAddress()+ CORE_NUM_2_ADDR(ncore) + CORE_R16 + 4*ncore)) != fExpectedDataFromDut->end());
				bool isCoreIdEndofTestistim_east =  ( fExpectedDataFromDut->find((istim_east->GetBaseAddress()+ CORE_NUM_2_ADDR(ncore) + CORE_R32 + 4*ncore)) != fExpectedDataFromDut->end());
				bool isCoreIdEndofTestistim_west =  ( fExpectedDataFromDut->find((istim_west->GetBaseAddress()+ CORE_NUM_2_ADDR(ncore) + CORE_R48 + 4*ncore)) != fExpectedDataFromDut->end());

				isCoreIdEndofTest = isCoreIdEndofTestistim_north && isCoreIdEndofTestistim_south && isCoreIdEndofTestistim_east && isCoreIdEndofTestistim_west;
			} else {
				bool isCoreIdEndofTestistim_west =  ( fExpectedDataFromDut->find((istim_west->GetBaseAddress()+ CORE_NUM_2_ADDR(ncore) + CORE_R48 + 4*ncore)) != fExpectedDataFromDut->end());
				isCoreIdEndofTest = isCoreIdEndofTestistim_west;
			}

			if(isCoreIdEndofTest) {
				//check returned data
				sc_uint<32> coreRegDataOut_istim_north,coreRegDataOut_istim_south,coreRegDataOut_istim_east,coreRegDataOut_istim_west;
				for( unsigned k = 0 ;k <4 ; k++  ) {

					if(NCORES>2) {
						coreRegDataOut_istim_north( ((k+1)*8-1),(k*8) )= fExpectedDataFromDut->operator [](istim_north->GetBaseAddress()+ CORE_NUM_2_ADDR(ncore) + CORE_R0 + 4*ncore +k);
						coreRegDataOut_istim_south( ((k+1)*8-1),(k*8) )= fExpectedDataFromDut->operator [](istim_south->GetBaseAddress()+ CORE_NUM_2_ADDR(ncore) + CORE_R16 + 4*ncore +k);
						coreRegDataOut_istim_east( ((k+1)*8-1),(k*8) )= fExpectedDataFromDut->operator [](istim_east->GetBaseAddress()+ CORE_NUM_2_ADDR(ncore) + CORE_R32 + 4*ncore +k);
						coreRegDataOut_istim_west( ((k+1)*8-1),(k*8) )= fExpectedDataFromDut->operator [](istim_west->GetBaseAddress()+ CORE_NUM_2_ADDR(ncore) + CORE_R48 + 4*ncore +k);
					} else {
						coreRegDataOut_istim_west( ((k+1)*8-1),(k*8) )= fExpectedDataFromDut->operator [](istim_west->GetBaseAddress()+ CORE_NUM_2_ADDR(ncore) + CORE_R48 + 4*ncore +k);

					}
				}
				if(NCORES>2) {

					if( coreRegDataOut_istim_north  != REG_READ_VAL) {
						cout << "ERROR :wrong value from  core register at end of test " << hex << (istim_north->GetBaseAddress()+ CORE_NUM_2_ADDR(ncore) + CORE_R0 + 4*ncore) << " == " << coreRegDataOut_istim_north<< endl;
						fExpectedDataFromDut->clear();
						wait(10);
						sc_stop();
					}

					if( coreRegDataOut_istim_south  != REG_READ_VAL) {
						cout << "ERROR :wrong value from  core register at end of test " << hex << (istim_south->GetBaseAddress()+ CORE_NUM_2_ADDR(ncore) + CORE_R16 + 4*ncore) << " == " << coreRegDataOut_istim_south<< endl;
						fExpectedDataFromDut->clear();
						wait(10);
						sc_stop();
					}
					if( coreRegDataOut_istim_east  != REG_READ_VAL) {
						cout << "ERROR :wrong value from  core register at end of test " << hex << (istim_east->GetBaseAddress()+ CORE_NUM_2_ADDR(ncore) + CORE_R32 + 4*ncore) << " == " << coreRegDataOut_istim_east<< endl;
						fExpectedDataFromDut->clear();
						wait(10);
						sc_stop();
					}
					if( coreRegDataOut_istim_west  != REG_READ_VAL) {
						cout << "ERROR :wrong value from  core register at end of test " << hex << (istim_west->GetBaseAddress()+ CORE_NUM_2_ADDR(ncore) + CORE_R48 + 4*ncore) << " == " << coreRegDataOut_istim_west<< endl;
						fExpectedDataFromDut->clear();
						wait(10);
						sc_stop();
					}
				} else {
					if( coreRegDataOut_istim_west  != REG_READ_VAL) {
						cout << "ERROR :wrong value from  core register at end of test " << hex << (istim_west->GetBaseAddress()+ CORE_NUM_2_ADDR(ncore) + CORE_R48 + 4*ncore) << " == " << coreRegDataOut_istim_west<< endl;
						fExpectedDataFromDut->clear();
						wait(10);
						sc_stop();
					}
				}

				//cout << "MSG>>CORE ID  " << i << " get R0 back value 0x" << hex << fHostMemArray[(fActiveHostDriver->GetBaseAddress()+ CORE_NUM_2_ADDR(i) + CORE_R0)]<< endl;
			}
			if(isCoreIdEndofTest) {
				cout << "MSG>>DV flow: core registers (R0..R63) back for Core   " << ncore << endl;
			}
		}//while(isCoreIdEndofTest == false)


		wait(50);
		if(NCORES>2) {
			wait(5000);
		}

	}//for(unsigned ncore = 0 ; ncore < NCORES;ncore++) for R regs back
	cout << "MSG>>DV flow: ALL core registers (R0..R63) back for chip    "  << endl;

	while(true) {
		wait(500);
		bool host_done = isHostDriversDone();
		wait(500);
		if(host_done) {
			break;
		}
	}
	if(NCORES>2) {
		wait(2000);
	}
	cout << "MSG>>DV flow:!!!!!!!!!!!!TEST DONE!!!!!!!!!!!!!!!!!!!!!!" << endl;

	wait(1000);

	stop_test();

	wait(100);



}



