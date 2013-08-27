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
#include <iostream>
#include <iomanip>
#include "systemc.h"

#include <map>

#include "stimulus.h"

using namespace std;

#include "systemc.h"	// SystemC
#include "vmodel_utils.h"

extern bool batch_mode;




/* STIMULUS FLOW:                                       */
/* 1.) Initialize inputs                                */
/* 2.) Assert reset                                     */
/* 3.) Write in program                                 */
/* 4.) Release reset to start program                   */
/* 5.) Write in data (in parallel to program executing) */

void Stimulus::initialize(vector<StimulusEntry> StimulusArray,int t, unsigned waitForOtherHostBeforeDrive){
	trace   = t;
	//Stimulus file
	stimulusArray      = StimulusArray;//creating a local pointer for use by class methods

	nCyclesToWaitBeforeDrive=waitForOtherHostBeforeDrive;
}


void Stimulus::stimuli(void){

	//STEP0-INIT

	//cout <<  "-------MSG>>: host driver 0x"<< hex << fHostBaseAddress << " INITIATING " << endl;

	stim_reset          = 1;
	stim_trace          = trace;
	wait(100);
	wait();
	wait();
	wait();
	wait();
	wait();
	//STEP1-RESETING
	//std::cout << "MSG>>RELEASING RESET" << endl;
	stim_reset=0;

	//STEP2-LOADING
	drive_stimulus();
	//std::cout << "MSG>>FINISHED PROGRAM LOAD" << endl;
	//STEP3-WAIT
	while(1){
		wait();
	}
}

/*********************************************************
 *
 *DRIVING STIMULUS TO DUT:
 *
 *
 * array[0]=srcaddr
 * array[1]=data
 * array[2]=dstadddr
 * array[3]=ctrlmode
 * array[4]=transaction type(see legal combos below)
 *
 *
 * Note: The transactor stops driving on "wait" back-pressure
 *       signal going high.
 *
 *
 *
 *
 *
 **********************************************************/
extern bool gdb_server;
extern bool trace_external;
extern unsigned long force_stop_at_addr;

void Stimulus::drive_stimulus(){

	assert(fHostMemArray);

	vector<StimulusEntry>::iterator it =   stimulusArray.begin();

	//check for host driver designed transaction
	while(it!=stimulusArray.end()) {
		//vector<StimulusEntry>::iterator next_it = it++;

		StimulusEntry sEntry = (*it);

		//cerr << sEntry << endl;

		//check if transaction is designated to hosts
		if( !(isDriverSpi()) && //not to chip TODO check host ranges
				(
						((sc_uint<32>(fHostBaseAddress)(31,29) == sc_uint<32>(sEntry.dstAddr)(31,29) ) &&
						(sc_uint<32>(fHostBaseAddress)(25,23) == sc_uint<32>(sEntry.dstAddr)(25,23) ))

				||
						( 	(batch_mode	|| gdb_server) &&		//in not DV random mode : don't drive host2host
						( sc_uint<32>(sEntry.dstAddr)(31,29) !=   sc_uint<32>(CHIP_BASE)(31,29) ||
						  sc_uint<32>(sEntry.dstAddr)(25,23) !=   sc_uint<32>(CHIP_BASE)(25,23) ) )
				)



		) {

			//the transaction designated for host
			if( (sEntry.dataMode % 2) != 0  ) { //write
				//record data
				for ( unsigned i = 0 ; i < (1 << sc_uint<32>(  (sEntry.dataMode)>>2  )) ; i++) {
					if( i< 4 ) {
						(*fHostMemArray)[sEntry.dstAddr+i] =sc_uint<32>( sEntry.data )( (i+1)*8-1,i*8  );
					} else {
						unsigned k = i - 4;
						assert(k<4);
						(*fHostMemArray)[sEntry.dstAddr+i] =sc_uint<32>( sEntry.srcAddr )( (k+1)*8-1,k*8  );
					}

					//cout  << "SET "<< hex << sEntry.dstAddr+i << " " << sc_uint<32>( sEntry.data )( (i+1)*8-1,i*8  )<< endl;
				}
			} else {
				cout <<  "ERROR: host driver 0x"<< hex << fHostBaseAddress << " Stimulus error : The read request from (active) host " << hex  <<  sEntry.dstAddr << endl;
				wait(10);
				sc_stop();
			}
#ifdef DEBUG
			std::cout << "MSG>>NOT DRIVING STIM ---> TRANSACTION DESIGNATED TO HOST(s)";
			std::cout << "Host [" <<  hex << sc_uint<32>(fHostBaseAddress)<< "] " << sEntry << dec << endl;

#endif

			stimulusArray.erase(it);
		} else {
			//make further check in the run time !!!!  (see below)
			it++;
		}

		//it = next_it;
	}

	wait(1);
	stim_write=0;
	stim_access=0;
	wait(100);


	//wait while other host drives the config
	wait(20*nCyclesToWaitBeforeDrive+1);

	cout<< hex << "DV drive host file -start- [" << sc_uint<32>(fHostBaseAddress)<< "] " << (isDriverSpi() ? "SPI" : "HOST")<< endl;


	while(true) {

		//check this is not wait for spi interface
		if((dut_wr_wait || dut_rd_wait ) && isDriverSpi()==true) {
			cout <<  "ERROR (SPI): host driver 0x"<< hex << fHostBaseAddress << " Getting the DUT wait for SPI master port" << endl;
			wait(10);
			sc_stop();
		}


		if(!stimulusArray.empty()) {

			StimulusEntry sEntry = *(stimulusArray.begin());
			stimulusArray.erase(stimulusArray.begin());


			//check we drive valid transaction destination address belong to chip or one of hosts
			//check if address is designated for host
			if( !gdb_server  && !batch_mode && (sEntry.ctrlMode != MULTI_CAST_CTRL_MODE)) {
				if(
						( sc_uint<32>(sEntry.dstAddr)(31,29) != sc_uint<32>(CHIP_BASE)(31,29)  || sc_uint<32>(sEntry.dstAddr)(25,23) != sc_uint<32>(CHIP_BASE)(25,23)  ) &&
						( sc_uint<32>(sEntry.dstAddr)(31,29) != sc_uint<32>(NORTH_BASE)(31,29) || sc_uint<32>(sEntry.dstAddr)(25,23) != sc_uint<32>(NORTH_BASE)(25,23) ) &&
						( sc_uint<32>(sEntry.dstAddr)(31,29) != sc_uint<32>(SOUTH_BASE)(31,29) || sc_uint<32>(sEntry.dstAddr)(25,23) != sc_uint<32>(SOUTH_BASE)(25,23) )  &&
						( sc_uint<32>(sEntry.dstAddr)(31,29) != sc_uint<32>(WEST_BASE)(31,29)  || sc_uint<32>(sEntry.dstAddr)(25,23) != sc_uint<32>(WEST_BASE)(25,23) ) &&
						( sc_uint<32>(sEntry.dstAddr)(31,29) != sc_uint<32>(EAST_BASE)(31,29)  || sc_uint<32>(sEntry.dstAddr)(25,23) != sc_uint<32>(EAST_BASE)(25,23) )
				) {

					cerr<< hex << "DV ERROR: Host [" << sc_uint<32>(fHostBaseAddress)<< "] drives ( HOST write or read )  transaction which is not designated for DUT " <<  (sEntry.dstAddr) << dec << endl;
					cerr << sEntry << endl;
					fHostMemArray->clear();
					wait(10);
					sc_stop();
				}
			}

			bool iSIlatToBeDriven = (sc_uint<32>(sEntry.dstAddr)(25,23) == sc_uint<32>(CHIP_BASE)(25,23)  &&
					sc_uint<32>(sEntry.dstAddr)(31,29) == sc_uint<32>(CHIP_BASE)(31,29)  &&
					sc_uint<32>(sEntry.dstAddr)(19,0) == CORE_ILAT &&
					sc_uint<4>(sEntry.dataMode)[0] == 1 /*WR*/);
			if(iSIlatToBeDriven) {
				std::cout << hex << "MSG>> Host [" << sc_uint<32>(fHostBaseAddress)<< "]ILAT (  "  << sEntry.dstAddr << ") set to "<<  sEntry.data << dec<< endl;
				//wait_for_hloadr_sync_out.write(false);
			}
			//prepare sign off read
			if( (sEntry.dataMode % 2) != 0  && (sEntry.isWriteBack == false) && sEntry.ctrlMode != MULTI_CAST_CTRL_MODE) { //write , but not write back or multicast

				StimulusEntry sEntrySignOff = sEntry;

				sc_uint<32> sRAMspace = sc_uint<32>(sEntrySignOff.dstAddr)(19,0);

				sEntrySignOff.dataMode ++;//make write ---> read
				sc_uint<32> srcAddr;
				srcAddr = sc_uint<32>(sEntrySignOff.dstAddr);
				srcAddr(31,29) = sc_uint<32>(fHostBaseAddress)(31,29);
				srcAddr(25,23) = sc_uint<32>(fHostBaseAddress)(25,23);
				sEntrySignOff.srcAddr=srcAddr;
				sEntrySignOff.ctrlMode=REGULAR_CTRL_MODE;


				if(sc_uint<32>(sEntrySignOff.dstAddr)(31,29) == sc_uint<32>(CHIP_BASE)(31,29)   && sc_uint<32>(sEntrySignOff.dstAddr)(25,23) == sc_uint<32>(CHIP_BASE)(25,23)   &&(sRAMspace >= MMR_START) ) {
					// drive or data back to MMR space
				} else {

#ifdef DEBUG
					std::cerr << "MSG>>SIGN OFF STIM :Host [" <<  hex << sc_uint<32>(fHostBaseAddress)<< "] ";
					std::cerr <<  sEntry;
#endif

					signoffArray.push_back(sEntrySignOff);
				}
			}

			//DRIVE

			stim_type          = sEntry.dataMode;
			unsigned int do_wait = sEntry.delay;

			bool do_write;

			if(stim_type%2 == 0 ) {
				do_write=false;
			} else {
				do_write=true;
			}

			while(true) {
				if(do_wait>0) {
					do_wait--;
				} else {
					if(!dut_wr_wait  && do_write) { //WRITE
						break;
					}
					if(!dut_rd_wait && !do_write) { //READ
						break;
					}
				}

				stim_write.write(false);
				stim_access.write(false);
				wait(1);
			}

			//Drive transaction
#ifdef DEBUG
			std::cout << hex;
			std::cout << "MSG>>DRIVING STIM: Host [" << sc_uint<32>(fHostBaseAddress)<< "] ";
			cout << sEntry << endl;
			std::cout << dec;
#endif

			stim_access.write(1);
			stim_write.write(do_write);
			stim_ctrlmode.write(sEntry.ctrlMode);
			stim_dstaddr.write(sEntry.dstAddr);
			stim_data.write(sEntry.data);
			stim_srcaddr.write(sEntry.srcAddr);


			if(stim_type==14){//double read
				stim_datamode=3;
			}
			else if(stim_type==13){//double write,1101
				stim_datamode=3;
			}
			else if(stim_type==11){//word read-modify write,1011
				cerr << "Stimulus Error: the read-modify write is not supported any more  " << endl;
				sc_stop();
			}
			else if(stim_type==10){//word read,1010
				stim_datamode=2;
			}
			else if(stim_type==9){//word write,1001
				stim_datamode=2;
			}

			else if(stim_type==6){//short read,110
				stim_datamode=1;
			}
			else if(stim_type==5){//short write,101
				stim_datamode=1;
			}
			else if(stim_type==2){//byte read,0010
				stim_datamode=0;
			}
			else if(stim_type==1){//byte write,0001
				stim_datamode=0;
			}
			else if(stim_type==0){//dummy transaction
				assert(0);
				stim_datamode=0;
			}
			else{
				assert(0);
				stim_datamode=0;
				std::cout << "MSG>>INVALID TRANSACTION TYPE: "<< setfill('0') << setw(1) << hex << stim_type << endl;
			}


			wait(1);


			//WAIT UNTIL SLAVE IS READY
			unsigned chip_slave_wait=0;
			while(true) {//wait until ready

				if(!dut_wr_wait  && do_write) { //WRITE
					break;
				}

				if(!dut_rd_wait && !do_write) { //READ
					break;
				}

				chip_slave_wait++;
				if(chip_slave_wait> 300000) {
					cout<< hex << "DV ERROR: Host [" << sc_uint<32>(fHostBaseAddress)<< "] getting wait from DUT slave interface for more than 300000 cycles" << dec << endl;

					cout << " .... next transaction to drive " << *(stimulusArray.begin() ) << endl;
					fHostMemArray->clear();
					wait(10);
					sc_stop();
				}

				wait(1);

			}
			//		}

		} else {//no more stimulus


			stim_write=0;
			stim_access=0;

			wait(1);
		}
	}//while (true)
}

/*
 * Sample stimulus from DUT --- host slave interface, only Write has been implemented at this stage
 *
 *
 *
 */
//GDB server: stop on WP match
extern void HandleStopSignal(int signum);

void Stimulus::Sample() {



	stim_wr_wait.write(false);
	stim_rd_wait.write(false);


	assert(fHostMemArray);

	while(true) {


		if( (dut_access.read() == true)  ) {
			fNumberTransactionRecordedAtHost++;
		}


		if( dut_access.read() == true && dut_write.read() == false ) {


			fSlaveInterfaceActive=true;

			StimulusEntry recStimulus = StimulusEntry(dut_srcaddr.read(), dut_data.read() , dut_dstaddr.read() , dut_ctrlmode.read() ,dut_datamode.read());

#ifdef DEBUG
			std::cerr << "MSG>>SAMPLING STIM Read from DUT :Host [" <<  hex << sc_uint<32>(fHostBaseAddress)<< "] ";
			std::cerr << recStimulus ;
#endif

			if(isDriverSpi() == true) {
				cout <<  "ERROR (SPI): host driver 0x"<< hex << fHostBaseAddress << " Getting the read request from DUT for SPI master port" << endl;
				wait(10);
				sc_stop();
			}


			sc_uint<32> dstAddr=dut_dstaddr.read() ;
			sc_uint<64> data_out;
			unsigned data_mode=dut_datamode.read();

			//TODO OLEG
			assert(data_mode < 4);


			//check if address is designated for host
			if(!gdb_server  && !batch_mode && (dstAddr(31,29) != sc_uint<32>(fHostBaseAddress)(31,29)  || dstAddr(25,23) != sc_uint<32>(fHostBaseAddress)(25,23)   ) ) {
				cerr << hex << "DUT ERROR: Host[" <<sc_uint<32>(fHostBaseAddress) << "] receives ( DUT read) destination  transaction with out of the host address range  "  <<  dstAddr << dec  << endl;
				std::cout << recStimulus ;
				fHostMemArray->clear();
				wait(20);
				sc_stop();
			}
			if(!gdb_server  && !batch_mode &&(
					(	sc_uint<32>(dut_srcaddr.read())(31,29) != sc_uint<32>(CHIP_BASE)(31,29)  || sc_uint<32>(dut_srcaddr.read())(25,23) != sc_uint<32>(CHIP_BASE)(25,23))  &&
					(	sc_uint<32>(dut_srcaddr.read())(31,29) != sc_uint<32>(NORTH_BASE)(31,29) || sc_uint<32>(dut_srcaddr.read())(25,23) != sc_uint<32>(NORTH_BASE)(25,23)) &&
					(	sc_uint<32>(dut_srcaddr.read())(31,29) != sc_uint<32>(SOUTH_BASE)(31,29) || sc_uint<32>(dut_srcaddr.read())(25,23) != sc_uint<32>(SOUTH_BASE)(25,23)) &&
					(	sc_uint<32>(dut_srcaddr.read())(31,29) != sc_uint<32>(WEST_BASE)(31,29)  || sc_uint<32>(dut_srcaddr.read())(25,23) != sc_uint<32>(WEST_BASE)(25,23))  &&
					(	sc_uint<32>(dut_srcaddr.read())(31,29) != sc_uint<32>(EAST_BASE)(31,29)  || sc_uint<32>(dut_srcaddr.read())(25,23) != sc_uint<32>(EAST_BASE)(25,23))
			) ){
				cerr<< hex << "DUT ERROR: Host[" << sc_uint<32>(fHostBaseAddress)<< "] receives ( DUT read) ( source addr) transaction with out of the chip or hosts address range  "  <<  dut_srcaddr.read() << dec << endl;
				fHostMemArray->clear();
				wait(20);
				sc_stop();
			}



#ifdef MIS_ALIGNMENT_CHECK
			//check aligment
			if( data_mode == 3 ) {
				if(dstAddr%8 != 0) {
					cout << "ERROR: (protocol violation)misalignment  double access to address " <<  hex << dstAddr << endl;
					wait(10);
					sc_stop();
				}
			}
			if( data_mode == 2 ) {
				if(dstAddr%4 != 0) {
					cout << "ERROR: (protocol violation) misalignment  word access to address " <<  hex << dstAddr << endl;
					wait(10);
					sc_stop();
				}
			}
			if( data_mode == 1 ) {
				if(dstAddr%2 != 0) {
					cout << "ERROR: (protocol violation) misalignment  half access to address " <<  hex << dstAddr << endl;
					wait(10);
					sc_stop();
				}
			}

#else
			//make alignment
			dstAddr = ( dstAddr >> data_mode ) <<  data_mode;

#endif

			for ( unsigned i = 0 ; i < (1 << data_mode) ; i++) {


				if ( (*fHostMemArray).find(dstAddr+i) ==  (*fHostMemArray).end() ) { //

					//cout << "WARNING the chip reads from uninitialized address: " << hex<< dstAddr+i<< endl;
					data_out ( ((i+1)*8-1),(i*8)  ) = 0;// TODO rand();
				} else {
					data_out ( ((i+1)*8-1),(i*8)  )   = (*fHostMemArray)[dstAddr+i] ;

				}


			}

			StimulusEntry sBack;
			if(data_mode == 3) {
				sBack = StimulusEntry( data_out(63,32)   ,data_out(31,0) , (dut_srcaddr.read())  , dut_ctrlmode.read() ,  (data_mode<<2)+1);
			} else {
				sBack = StimulusEntry( dstAddr  ,data_out(31,0) , (dut_srcaddr.read())  , dut_ctrlmode.read() ,  (data_mode<<2)+1);
			}

			//check if transaction is not multicast
			if( sBack.ctrlMode == MULTI_CAST_CTRL_MODE) {
				cerr << hex << "DUT ERROR: Host[" <<sc_uint<32>(fHostBaseAddress) << "] receives ( DUT read) destination multicast read  transaction  "  << dec  << endl;
				std::cout << recStimulus ;
				fHostMemArray->clear();
				wait(20);
				sc_stop();
			}

#ifdef DEBUG
			std::cerr << "MSG>>DRIVE back to core :Host [" <<  hex << sc_uint<32>(fHostBaseAddress)<< "] ";
			std::cerr << sBack ;
#endif
			//return data write transaction to chip
			unsigned rb_delay = (rand() % 10);//only 25% transaction will have delays
			if((rb_delay%4) == 0 ) {
				sBack.delay= 1 + (rand() % 10);
			} else {
				sBack.delay= 0;
			}

			sBack.isWriteBack=true;
			AddWriteBackStimulus(sBack);


			if((gdb_server   ||  batch_mode) && trace_external) {
				*host_addr_file_out << "READ:" << hex << dstAddr << " <--- "<< sBack.data << endl;
				if(force_stop_at_addr) {
					if(force_stop_at_addr==dstAddr) {
						cerr << "(GDB SERVER/BATCH MODE): Watch point match ... stop simulation" << endl;
						HandleStopSignal(1);
					}
				}
			}

		}



		if(dut_access.read() == true &&  dut_write.read() == true) {

			fSlaveInterfaceActive=true;


#ifdef DEBUG
			std::cout << "MSG>>SAMPLING STIM Write from DUT :Host [" <<  hex << sc_uint<32>(fHostBaseAddress)<< "] ";
			StimulusEntry recStimulus = StimulusEntry(dut_srcaddr.read(), dut_data.read() , dut_dstaddr.read() , dut_ctrlmode.read() ,dut_datamode.read()<<2);

			std::cout << recStimulus ;
#endif


			sc_uint<32> dstAddr=dut_dstaddr.read() ;
			unsigned tranWriteMode =  dut_ctrlmode.read();
#ifdef AUTO_DMA_DEBUG
			if( sc_uint<32>(dut_dstaddr.read())(19,0) == sc_uint<32>(DMA0_STATUS)(19,0) || sc_uint<32>(dut_dstaddr.read())(19,0) == sc_uint<32>(DMA1_STATUS)(19,0) ) {
				std::cout << "(AUTO_DMA_DEBUG) MSG>>SAMPLING STIM Write from DUT :Host [" <<  hex << sc_uint<32>(fHostBaseAddress)<< "] ";
				StimulusEntry recStimulus = StimulusEntry(dut_srcaddr.read(), dut_data.read() , dut_dstaddr.read() , dut_ctrlmode.read() ,dut_datamode.read()<<2);

				std::cout << recStimulus ;

			}
#endif



			//check if address is designated for host
			if(!gdb_server  && !batch_mode &&( (dstAddr(31,29) != sc_uint<32>(fHostBaseAddress)(31,29) ||  dstAddr(25,23) != sc_uint<32>(fHostBaseAddress)(25,23)) && (tranWriteMode != MULTI_CAST_CTRL_MODE)  )) {
				cout << hex << "DUT ERROR: Host[" <<sc_uint<32>(fHostBaseAddress) << "] receives ( DUT write ) destination  transaction with out of the host range " <<  dstAddr << dec << endl;
				fHostMemArray->clear();
				wait(10);
				sc_stop();
			}



			unsigned data_mode = dut_datamode.read();
			//TDDO OLEG
			assert(data_mode < 4);


			//TODO
#ifdef MIS_ALIGNMENT_CHECK
			//check alignment
			if( data_mode == 3 ) {
				if(dstAddr%8 != 0) {
					cout << "ERROR: (protocol violation)misalignment  double access to address " <<  hex << dstAddr << endl;
					wait(10);
					sc_stop();
				}
			}
			if( data_mode == 2 ) {
				if(dstAddr%4 != 0) {
					cout << "ERROR: (protocol violation) misalignment  word access to address " <<  hex << dstAddr << endl;
					wait(10);
					sc_stop();
				}
			}
			if( data_mode == 1 ) {
				if(dstAddr%2 != 0) {
					cout << "ERROR: (protocol violation)misalignment  half access to address " <<  hex << dstAddr << endl;
					wait(10);
					sc_stop();
				}
			}
#else
			//make alignment
			dstAddr = ( dstAddr >> data_mode ) <<  data_mode;

#endif



			sc_uint<64> data;
			data(31,0) = dut_data.read();
			data(63,32) = dut_srcaddr.read();//data placed in src addr

			unsigned nBytesData = (1 << sc_uint<32>(dut_datamode.read()));
			for ( unsigned i = 0 ; i < nBytesData ; i++) {
				//cout << hex<<"W C-->H  " << dstAddr+i << " [" << ((i+1)*8-1 ) << endl;
				(*fHostMemArray)[dstAddr+i] = data ( ((i+1)*8-1),(i*8)  );

			}

			if(! gdb_server  && !batch_mode && !signOffStarted) {
				*host_addr_file_out << hex << dstAddr << endl;
			}
			if(batch_mode &&trace_external ) {
				*host_addr_file_out << "WRITE:" << hex << dstAddr << " ---> "<<  data( ((nBytesData)*8-1),0  ) << endl;

				if(force_stop_at_addr) {
					if(force_stop_at_addr==dstAddr) {
						cerr << "(GDB SERVER/BATCH MODE): Watch point match ... stop simulation" << endl;
						HandleStopSignal(1);
					}
				}
			}

			//multicast care
			if(tranWriteMode == MULTI_CAST_CTRL_MODE) {
				sc_uint<32> stdMCAddr = data(63,32);
				*mc_file_out << hex << dstAddr << " " <<sc_uint<32>(fHostBaseAddress)(31,20) << " " << data((nBytesData*8)-1,0) << " " << stdMCAddr(31,20) << dec <<  endl;
			}

		}

		//TODO WAIT
		if(NCORES>2 && isDriverSpi()==false) {

			unsigned int do_wait = rand() % 10;
			if(do_wait==0) {//do wait with  low probability

				do_wait = rand() % 10;

				for(unsigned k =0 ; k < do_wait; k++) {
					stim_wr_wait.write(1);
					stim_rd_wait.write(1);
					wait(1);
				}
			}
		}

		stim_wr_wait.write(0);
		stim_rd_wait.write(0);
		wait(1);

	}
}

ostream& operator<< (ostream& s,  StimulusEntry& p ) {

	s << " SRCADDR="   << setfill('0') << setw(8) << hex << p.srcAddr;
	s << " DATA="      << setfill('0') << setw(8) << hex << p.data;
	s << " DSTADDR="   << setfill('0') << setw(8) << hex << p.dstAddr;
	s << " CTRLMODE="  << setfill('0') << setw(8) << hex << p.ctrlMode;
	s << " DATAMODE="  << setfill('0') << setw(8) << hex << p.dataMode;
	s << " DELAY="  << setfill('0') << setw(8) << hex << p.delay;
	s << std::endl;
	return s;
}

//void Stimulus::CheckAutoDmaAndDipatchReadIfNeeded(bool &readyToDrive,  unsigned int aDmaNumber,vector<StimulusEntry>::iterator tranEntryP ) {
//
//	unsigned long addfAutoDMA;
//	if(aDmaNumber==0) {
//		addfAutoDMA=DMA0_AUTODMA0;
//	} else if(aDmaNumber==1)  {
//		addfAutoDMA=DMA1_AUTODMA0;
//	} else {
//		assert(0);//2 DMAs
//	}
//
//	if(sc_uint<32>(tranEntryP->dstAddr)(19,0) == addfAutoDMA && sc_uint<4>(tranEntryP->dataMode)[0] == 1 /*WR*/) {
//
//		if( fDMAStatusTranReadBackOntheWay[aDmaNumber]==true)  { //read pending
//			readyToDrive=false;
//#ifdef AUTO_DMA_DEBUG
//			cout << " NOT READY, read DMA status on the way  " << endl;
//#endif
//		} else {
//
//
//			sc_uint<32> statusDMABackAddr;
//			statusDMABackAddr(25,20) = sc_uint<32>(tranEntryP->dstAddr)(25,20);//core ID
//			if(aDmaNumber==0) {
//				statusDMABackAddr(19,0) = sc_uint<32>(DMA0_STATUS)(19,0);//internal
//			} else {
//				statusDMABackAddr(19,0) = sc_uint<32>(DMA1_STATUS)(19,0);//internal
//			}
//
//			statusDMABackAddr(31,26) = sc_uint<32>(fHostBaseAddress)(31,26);//host back
//#ifdef AUTO_DMA_DEBUG
//			std::cout << "+++++MSG>> DMA_AUTODMA%" <<( (aDmaNumber==0) ? (0) : (1)) <<  "("<< hex << *tranEntryP << ") check ... N% " << fAutoDMAtranCounterForDebug << " ... ";
//#endif
//
//
//			if(fAutoDMAReadyMap.find(statusDMABackAddr) != fAutoDMAReadyMap.end()  && fAutoDMAReadyMap[statusDMABackAddr] == true) {
//#ifdef AUTO_DMA_DEBUG
//				cout << " READY (status tran returned) " << endl;
//#endif
//				fAutoDMAReadyMap[statusDMABackAddr] = false;
//				fAutoDMAtranCounterForDebug++;
//			} else {
//#ifdef AUTO_DMA_DEBUG
//				cout << " NOT READY (status tran returned) " << endl;
//#endif
//				readyToDrive=false;
//
//
//				//now to check if any previous dma status read transaction have been arrived
//
//				assert(fDMAStatusTranReadBackOntheWay[aDmaNumber]==false);
//
//				StimulusEntry sAutoDMAEntry;
//				//build status read
//				//WORD
//				sAutoDMAEntry.dataMode = WORD_READ;
//				//set as source
//				sAutoDMAEntry.srcAddr=statusDMABackAddr;
//
//				//data
//				sAutoDMAEntry.data = fAutoDMAtranCounterForDebug;
//
//				//dest (31,20) same AUTO DMA write
//				sc_uint<32> dAutoDMAaddr = tranEntryP->dstAddr;
//				//DMA status
//				dAutoDMAaddr(19,0)=statusDMABackAddr(19,0);
//				//set as dest
//				sAutoDMAEntry.dstAddr=dAutoDMAaddr;
//
//				AddFirstStimulus(sAutoDMAEntry);
//#ifdef AUTO_DMA_DEBUG
//				cout << "ADDED for DMA check >>>>>>>  " << sAutoDMAEntry << endl;
//#endif
//				fDMAStatusTranReadBackOntheWay[aDmaNumber] = true;
//
//			}
//
//
//
//		}
//	}
//}
