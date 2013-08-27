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

#ifndef _STIMUL_H_
#define _STIMUL_H_

#include <stdio.h>
#include <string>
#include <iostream>
#include <iomanip>
#include "systemc.h"

//#define AUTO_DMA_DEBUG 1


#include <map>
#include "maddr_defs.h"


extern unsigned CHIP_BASE;

enum MODE_TYPE {
	DOUBLE_READ=0xe,
			DOUBLE_WRITE=0xd,
			READ_MO_WRITE_NOT_SUPPORTED=11,
			WORD_READ=0xa,
			WORD_WRITE=9,
			SHORT_READ=6,
			SHORT_WRITE=5,
			BYTE_READ=2,
			BYTE_WRITE=1,
			DUMMY_TRANSACTION=0
};
enum ECTRL_MODE {
	REGULAR_CTRL_MODE=0x0,
			SPI_READ_CTRL_MODE=0x1,
			MULTI_CAST_CTRL_MODE=0x3,
			MODIFY_READ_MODE=0xC
};


using namespace std;

struct StimulusEntry {
	unsigned int srcAddr;
	unsigned int data;
	unsigned int dstAddr;
	unsigned int ctrlMode;
	unsigned int dataMode;
	unsigned int delay;
	bool isWriteBack;
	StimulusEntry(unsigned int srcAddr_,unsigned int data_,unsigned int dstAddr_,unsigned int ctrlMode_,unsigned int dataMode_,unsigned int Delay=0) {
		srcAddr=srcAddr_;
		data=data_;
		dstAddr=dstAddr_;
		ctrlMode=ctrlMode_;
		dataMode=dataMode_;
		delay=Delay;
		isWriteBack=false;
	}
	StimulusEntry() {  delay=0; ctrlMode=0;isWriteBack=false;}
};
ostream& operator<< (ostream& s,  StimulusEntry& p ) ;



SC_MODULE(Stimulus){


	int                    stim_type;
	int                    trace;

	//Test Harness Signals  (3)
	sc_in<bool>                clk;
	sc_out<bool>               stim_reset;
	sc_out<bool>               stim_trace;

	//Mesh Transmit (Master) Interface(8)
	sc_out<bool>               stim_write;
	sc_out<bool>               stim_access;
	sc_out<uint32_t>           stim_datamode;
	sc_out<uint32_t>           stim_ctrlmode;
	sc_out<uint32_t>           stim_dstaddr;
	sc_out<uint32_t>           stim_data;
	sc_out<uint32_t>           stim_srcaddr;
	sc_in<bool>                dut_wr_wait;
	sc_in<bool>                dut_rd_wait;


	//Mesh Receive ( Slave) Interface(8)
	sc_in<bool>               dut_write;
	sc_in<bool>               dut_access;
	sc_in<uint32_t>           dut_datamode;
	sc_in<uint32_t>           dut_ctrlmode;
	sc_in<uint32_t>           dut_dstaddr;
	sc_in<uint32_t>           dut_data;
	sc_in<uint32_t>           dut_srcaddr;
	sc_out<bool>              stim_wr_wait;
	sc_out<bool>              stim_rd_wait;

	//dv signals:
	sc_in<bool>            wait_for_hloadr_sync_in;
	sc_out<bool>           wait_for_hloadr_sync_out;

	//Init method

	void initialize(vector <StimulusEntry>,	int t	, unsigned waitForOtherHostBeforeDrive);
	//Main method
	void  stimuli();


	void  drive_stimulus();
	void  Sample();


	//Constructor
	//Advance stimulus on the positive edge of the clock
	SC_CTOR(Stimulus){

		fHostMemArray=0;
		fHostBaseAddress=0;

		SC_THREAD(stimuli);     //called on every clock cycle
		sensitive << clk.neg(); //driving stimulus on opposite edge to solve hold problem


		SC_THREAD(Sample);
		sensitive << clk.neg();


		//reset counter for incomming transaction
		ResetNumberTransactionRecordedAtHost();

		fSlaveInterfaceActive=false;


		fDMAStatusTranReadBackOntheWay[0]=false;
		fDMAStatusTranReadBackOntheWay[1]=false;

		fAutoDMAtranCounterForDebug=0;
		isAllTransactionsHostFileAreDriven=false;

		mc_file_out=0;

		nCyclesToWaitBeforeDrive=0;


	}

	void SetDataExpected(map<sc_uint<32> , sc_uint<8> > *ExpectedDataFromDut) {
		fHostMemArray=ExpectedDataFromDut;
	}
	void SetDataToHost(sc_uint<32> addr, sc_uint<8> data) {
		(*fHostMemArray)[addr]=data;
	}
	bool IsDataReady(sc_uint<32> addr) {
		return  ((*fHostMemArray).find(addr) !=  (*fHostMemArray).end());
	}

	bool GetDataFromHost(sc_uint<32> addr, sc_uint<8>& data) {

		if ( (*fHostMemArray).find(addr) ==  (*fHostMemArray).end() ) {
			return false;
		} else {

			data = (*fHostMemArray)[addr];
			return true;
		}
	}
	//used by Debugger ONLY
	void ClearHostData(sc_uint<32> addr)  {

		if ( (*fHostMemArray).find(addr) ==  (*fHostMemArray).end() ) {
			assert(0);

		} else {
			(*fHostMemArray).erase((*fHostMemArray).find(addr));

		}
	}


private:
	map<sc_uint<32> , sc_uint<8> > *fHostMemArray;

	vector <StimulusEntry> stimulusArray;

	vector <StimulusEntry> signoffArray;

	unsigned fHostBaseAddress;

	sc_uint<64> fNumberTransactionRecordedAtHost;

	bool fSlaveInterfaceActive;

	map<sc_uint<32> , bool > fAutoDMAReadyMap;


	unsigned nCyclesToWaitBeforeDrive; // used to wait until the pre-configuration is done



public:

	sc_uint<64> GetNumberTransactionRecordedAtHost() {
		return fNumberTransactionRecordedAtHost;
	}

	void ResetNumberTransactionRecordedAtHost() {
		fNumberTransactionRecordedAtHost=0;
	}


	/* get number stimulus entries pending for delivery*/
	unsigned GetStimulusPendingSize() {
		return stimulusArray.size();
	}

	bool GetClearISSlaveInfActive() {
		bool res = fSlaveInterfaceActive;
		fSlaveInterfaceActive=false;
		return res;
	}

	void AddStimulus(StimulusEntry e) {

		///cout << hex << " GDB " << this->fHostBaseAddress << dec << endl;
		stimulusArray.push_back(e);
	}
	//used in auto DMA to check status
	void AddFirstStimulus(StimulusEntry e) {

		stimulusArray.insert(stimulusArray.begin(),e);
	}
	void AddWriteBackStimulus(StimulusEntry e) {
		for( vector <StimulusEntry>::iterator it = stimulusArray.begin(); it != stimulusArray.end(); it++) {
			sc_uint<32> sRAMspace = sc_uint<32>(it->dstAddr)(19,0);
			bool mmrChipWrite = (((it->dataMode % 2) != 0)/*write*/  &&
					sc_uint<32>(it->dstAddr)(31,28) == sc_uint<32>(CHIP_BASE)(31,28)&&
					sc_uint<32>(it->dstAddr)(25,22) == sc_uint<32>(CHIP_BASE)(25,22)
					&&(sRAMspace == CORE_ILAT));

			if(it->isWriteBack == false /*&& (fDMAStatusTranReadBackOntheWay[0] == false)*/ && !mmrChipWrite) {
				stimulusArray.insert(it,e);
				//out << " >>>>>>>>>>> AddWriteBackStimulus  " << stimulusArray.size() << " pos " << it-stimulusArray.begin() << endl;
				return;
			} else {
				//cout << " >>>>>>>>>>> AddWriteBackStimulus  " << stimulusArray.size() << " pos " << it-stimulusArray.begin() << endl;
			}
		}
		//cout << " >>>>>>>>>>>END ...... AddWriteBackStimulus  " << stimulusArray.size() << endl;
		stimulusArray.push_back(e);

	}


	unsigned GetBaseAddress() {
		return fHostBaseAddress;
	}

	void SetBaseAddress(unsigned baseAddress) {



		fHostBaseAddress=baseAddress;

		stringstream ss;//create a stringstream
		ss << "mc_dut_out_" << hex << fHostBaseAddress << ".txt" <<dec ;//add number to the stream

		mc_file_out = new ofstream(ss.str().c_str());

		stringstream sb;//create a stringstream
		sb << "host_addr_out_" << hex << fHostBaseAddress << ".txt" <<dec ;//add number to the stream
		host_addr_file_out = new ofstream(sb.str().c_str());

		signOffStarted=false;
	}

	//copy the signoff array to stimulus
	void DispathSignOff() {

		signOffStarted=true;

		//std::cout << "MSG>>DV flow: host (base=" << hex << fHostBaseAddress<< dec <<  ")READING CORE MEMORY FOR FUTHER COMPARISON " << endl;
		for(vector <StimulusEntry>::iterator it =  signoffArray.begin(); it != signoffArray.end(); it++ ) {

			stimulusArray.push_back(*it);
		}
	}

private:
	// due to possible DAM buffer overflow the DMA status need to be polled
	void CheckAutoDmaAndDipatchReadIfNeeded(bool &readyToDrive,  unsigned int aDmaNumber,vector<StimulusEntry>::iterator tranP ) ;
	bool fDMAStatusTranReadBackOntheWay[2];

	unsigned fAutoDMAtranCounterForDebug;
	//if all transaction in the host file driven
	bool isAllTransactionsHostFileAreDriven;

	ofstream *mc_file_out;

	ofstream *host_addr_file_out;

	bool signOffStarted;

public:
	bool isDriverSpi() {
		return(sc_uint<32>(fHostBaseAddress)(31,28) == sc_uint<32>(CHIP_BASE)(31,28) && sc_uint<32>(fHostBaseAddress)(25,22) == sc_uint<32>(CHIP_BASE)(25,22)) ;
	}

public :
	virtual ~Stimulus() {
		assert(mc_file_out);
		mc_file_out->close();
		delete mc_file_out;

		assert(host_addr_file_out);
		host_addr_file_out->close();
		delete host_addr_file_out;


	}

};

#endif

