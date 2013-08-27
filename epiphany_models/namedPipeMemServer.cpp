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

#include "vmodel_utils.h"
#include "namedPipeMemServer.h"




#define TMP_HOST_START  0x72000000
#define TMP_HOST_END    TMP_HOST_START + 0x30000000

#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<signal.h>
#include<fcntl.h>
#include <errno.h>

#include "named_fifo.h"


#define PERMS 0666


#include "traceSupport.h"
extern TraceSupport* traceFileCtl;

int fifofd_0, fifofd_1;



extern bool gdb_server;
extern bool always_record_wave;
extern bool batch_mode;




void HandleStopSignal(int signum)
{

	cerr << "(GDB SERVER/BATCH MODE): Get OS signal .. exiting ..." << endl;
	wait(100);
	sc_stop();

	if(batch_mode == false) {

		if (close(fifofd_0) < 0) {
			perror("close");
		}

		if (close(fifofd_1) < 0) {
			perror("close");
		}

		if (unlink(FIFO_0) < 0) {
			perror("unlink");
		}
		if (unlink(FIFO_1) < 0) {
			perror("unlink");
		}
	}

	if(!gdb_server  || always_record_wave) {
		std::cerr << "MSG>>DRIVE closing the .vcd file" << endl;
		if(traceFileCtl) {
			traceFileCtl->CloseRecord();
		}
	}
	exit(0);
}

void RegisterBreakISR() {
	if (signal(SIGINT, HandleStopSignal) < 0) {
		perror("signal");
		exit(-1);
	}
}

#define MESSAGE_SIZE 5 // r/w/kill size(w,s,c) addr data(for write or pad by zero) coreID
void TNamedPipeMemServer::mainloop() {




	wait(100);// TODO reset time


	long buffer[MESSAGE_SIZE];

	RegisterBreakISR();

	if (mknod(FIFO_0, S_IFIFO | PERMS, 0) < 0) {
		perror("mknod 0");
		fprintf(stderr,"file creating operation failed for file %s, try rm %s \n",FIFO_0,FIFO_0);
		exit(-1);
	}

	if (mknod(FIFO_1, S_IFIFO | PERMS, 0) < 0) {
		perror("mknod 1");
		fprintf(stderr,"file creating operation failed for file %s, try rm %s \n",FIFO_1,FIFO_1);
		exit(-1);
	}


	if ((fifofd_0 = open(FIFO_0, O_RDWR  )) < 0) {//| O_NONBLOCK O_RDONLY
		perror("open 0");
		exit(-1);
	}
	if ((fifofd_1 = open(FIFO_1, O_RDWR  )) < 0) {//| O_NONBLOCK O_RDONLY
		perror("open 1");
		exit(-1);
	}


	int counter=0;

	// Loop processing commands forever
	while (true)
	{
		wait(1);

		if((counter%1000)==0){
			std::cout <<"MSG>>(VMODEL)Time=" << dec << /*counter  << " CYCLES,  "*/ sc_time_stamp().to_string() << endl ;

		}
		counter++;

		//cerr << "Blocking read" << endl;



		int n = read(fifofd_0, buffer, sizeof(buffer));
		if (n < 0) {
			perror("read");
			continue;
		}
		//cerr << "Blocking read done" << endl;


		if(n > 0 ) {

			assert(n == sizeof(buffer));

			//cerr << " got n bytes " << n << endl;

			bool isValidReq=true;

			unsigned long*p_buff = (unsigned long*) buffer;
			//cerr <<  " F long " << hex << buffer[0] << dec << endl;
			bool isRead;
			if(p_buff[0]==0) {
				isRead = true;
			} else if(p_buff[0]==1){
				isRead = false;
			} else if(p_buff[0]==2) {
				cout << "getting kill request .. the simulation will be stopped" << endl;
				wait(50);
				sc_stop();
				HandleStopSignal(1);
			} else if(p_buff[0]==3) {
				//TODO
				//cout << "TODO getting trace rq  " << p_buff[2] << endl;
				if(p_buff[2] == 0) { //INIT
					//the vcd is set already from Sc_main
				} else if(p_buff[2] == 1) { //START
					traceFileCtl->StartRecord();
				} else if( p_buff[2] == 2 ) { //STOP
					traceFileCtl->CloseRecord();
				} else {
					cerr << "ERROR ; wrong trace req ... ignoring " << endl;
				}
				//do not need response
				continue;
			} else if(p_buff[0]==4) {
				cerr << "WARNING (GDB SERVER/BATCH MODE) :::: getting resume request  ... the model will be disconnected from gdb sever, use the CNTRL^C or -force_stop_at_addr to stop the vmodel" << endl;
				//return;
				//do not need response
				while(true) {wait(20000);}
				continue;

			} else {
				cerr << "unknown request ( should be 0 for read , 1 for write , 2 for kill ) ... ignored  " << endl;
				isValidReq=false;
			}


			ETRAN_TYPE accessType;
			if(p_buff[1]==BYTE_) {
				accessType=BYTE_;
			} else if(p_buff[1]==SHORT_) {
				accessType=SHORT_;
			} else if(p_buff[1]==WORD_)  {
				accessType=WORD_;
			} else {
				cerr << "ERROR: unknown request .. ignored for transaction size " << endl;
				isValidReq=false;
			}

			sc_uint<32> addr= p_buff[2];
			sc_uint<32> data= p_buff[3];


			if(p_buff[4] <  NCORES) {
				//set core ID for memory accesses
				SetCoreID(p_buff[4]);
			} else {
				cerr << "ERROR: core ID number is greater " << p_buff[4] << " than number cores supported by target "<<  NCORES<< endl;
				isValidReq=false;
			}



			bool resOp=false;

			if(isValidReq) {
				if(isRead) {
					resOp = this->readMem(addr,accessType,data);

				} else {
					resOp = this->writeMem(addr,accessType,data);

				}
			}

			//cerr << "blocking write " << endl;


			buffer[0] = 1;//Fail
			if(resOp && isValidReq) {
				buffer[0] = 0;//Good
				if(isRead) {
					buffer[1] = data;//data back
				}
			} else {
				cerr << "Error: memory access fail for address 0x" << hex << addr << dec << endl;
			}


			//cerr << "blocking write "<< hex << buffer[0]<< " " << buffer[1] <<dec  << endl;

			n = write(fifofd_1, buffer, sizeof(buffer));
			if (n < 0) {
				perror("write");
				continue;
			}

			if(n > 0 ) {
				//TODO
			}

			//cerr << "blocking write done" << endl;

		}


	}

}
//#define SPI_ME 1

bool  TNamedPipeMemServer::readMem (uint32_t  addr, ETRAN_TYPE t_type, sc_uint<32>& data) {

	sc_uint<8> data_b;

	//	sc_uint<32> aaa = addr;
	//	if(aaa(19,0) < 0x7fff) {
	//		cout << "read data --- " << hex << aaa(19,0) <<  dec << endl;
	//	}

	unsigned int n_bytes;
	unsigned int data_mode;
	switch (t_type) {
	case WORD_ :
		data_mode=WORD_READ;
		n_bytes=4;
		break;
	case SHORT_ :
		data_mode=SHORT_READ;
		n_bytes=2;
		break;
	case BYTE_ :
		data_mode=BYTE_READ;
		n_bytes=1;
		break;
	default:
		assert(0);
		break;
	}

	if(isControlledBySpi) {
		assert(fSpiDriver);
	} else {
		assert(fHostDriver);
	}


	sc_uint<32> dstAddr;
	unsigned srcAddr;

	if(IS_ADDRESS_IN_CHIP_RANGE(addr)) {

		//cerr << "RE full access " << hex << addr << dec << endl;

		dstAddr=addr ;
		if(isControlledBySpi) {
		} else {
			srcAddr=fHostDriver->GetBaseAddress() +CORE_NUM_2_ADDR(fCoreID) + MAKE_ADDRESS_INTERNAL( addr );
		}
		//TODO check bank range

	} else 	if(addr < CORE_SPACE) {

		dstAddr=(CHIP_BASE+CORE_NUM_2_ADDR(fCoreID)+addr) ;
		if(isControlledBySpi) {
		} else {
			srcAddr = fHostDriver->GetBaseAddress() +CORE_NUM_2_ADDR(fCoreID) + ( addr );
		}

		if(addr==CORE_DEBUG) {

			//wait(500);

		}

	} else {

		//check if address belongs to host
		if(addr >= TMP_HOST_START && addr < TMP_HOST_END ) {

			for( unsigned int i = 0 ; i < n_bytes ; i++ ) {
				if(isControlledBySpi) {
					if( fSpiDriver->GetDataFromHost( addr + i , data_b) ) {
						data(7+i*8,i*8) = data_b;
					}
				} else {
					if( fHostDriver->GetDataFromHost( addr + i , data_b) ) {
						data(7+i*8,i*8) = data_b;
					}
				}
			}
			return true;

		} else {

			cerr << "Warning access to out of the memoryrange  ("  << hex << addr<<  dec<<  ") will be ignored" << endl;
			return false;

		}
	}



	if(isControlledBySpi) {

		//assert(fSpiDriver->GetStimulusPendingSize()  == 0);
		while(fSpiDriver->GetStimulusPendingSize()  != 0) {
			cout << "Warning: SPI driver has pending transactions" <<endl;
			wait(100);
		}

		sc_uint<32> spiAddrBack=(fSpiDriver->GetBaseAddress() + ( (SPI_BACK_CORE_ROW_OFFSET) <<  26)   |  ( (SPI_BACK_CORE_COL_OFFSET) <<  20)  + SPI_RETURN_ADDR);
		StimulusEntry spiReadEntry=StimulusEntry(spiAddrBack  , 0,  dstAddr , SPI_READ_CTRL_MODE, data_mode);
		cout << "-------------------- Drive spi READ-----" << hex <<  spiReadEntry << dec << "-------------------" << endl;
		fSpiDriver->AddStimulus(spiReadEntry);


		while(true) {
			wait(5);
			if( fSpiDriver->IsDataReady(spiAddrBack) ) {
				break;
			}
		}

		for( unsigned int i = 0 ; i < n_bytes ; i++ ) {

			if(fSpiDriver->GetDataFromHost(spiAddrBack+i,data_b)) {

			} else {
				assert(0);// data not found
			}

			data(7+i*8,i*8) = data_b;

			fSpiDriver->ClearHostData(spiAddrBack + i );

		}


		//	if(dstAddr(19,0) < 0x7fff) {
		cout << "read data " << hex << dstAddr(19,0) << " "<< data << dec << endl;
		//	}


	} else {


		StimulusEntry stReadEntry = StimulusEntry( srcAddr , 0 , dstAddr ,0, data_mode );
		//cout << "-------------------- Drive GDB READ-----" << hex <<  stReadEntry << dec << "-------------------" << endl;
		fHostDriver->AddStimulus(stReadEntry);

		//wait for data comes back
		while(!fHostDriver->GetDataFromHost(srcAddr,data_b)) {
			wait(5);
		}

		//remove from host array
		for( unsigned int i = 0 ; i < n_bytes ; i++ ) {
			assert(fHostDriver->GetDataFromHost(srcAddr + i , data_b));
			data(7+i*8,i*8) = data_b;
			fHostDriver->ClearHostData(srcAddr + i );
		}

		if(dstAddr(19,0) < 0x7fff) {
			//cout << "read data " << hex << dstAddr(19,0) << " "<< data << dec << endl;
		}

	}

	return true;



}

bool  TNamedPipeMemServer::writeMem (uint32_t  addr, ETRAN_TYPE t_type, sc_uint<32>  data) {

	sc_uint<8> data_b;

	unsigned int n_bytes;
	unsigned int data_mode;
	switch (t_type) {
	case WORD_ :
		data_mode=WORD_WRITE;
		n_bytes=4;
		break;
	case SHORT_ :
		data_mode=SHORT_WRITE;
		n_bytes=2;
		break;
	case BYTE_ :
		data_mode=BYTE_WRITE;
		n_bytes=1;
		break;
	default:
		assert(0);
		break;
	}


	if(isControlledBySpi) {
		assert(fSpiDriver);

	} else {
		assert(fHostDriver);
	}


	unsigned dstAddr;

	if( IS_ADDRESS_IN_CHIP_RANGE(addr)) {
		//make address internal
		//do nothing

		//cerr << "WR full access " << hex << addr << dec << endl;

		dstAddr=addr ;

	} else 	if(addr < CORE_SPACE) {// core access

		dstAddr=(CHIP_BASE+CORE_NUM_2_ADDR(fCoreID)+addr) ;

	} else {


		//check if address belongs to host
		if(addr >= TMP_HOST_START && addr < TMP_HOST_END ) {

			if(isControlledBySpi) {
				for( unsigned int i = 0 ; i < n_bytes ; i++ ) {
					data_b=data(7+i*8,i*8) ;
					fSpiDriver->SetDataToHost(addr+i,data_b);
				}
			} else {
				for( unsigned int i = 0 ; i < n_bytes ; i++ ) {
					data_b=data(7+i*8,i*8) ;
					fHostDriver->SetDataToHost(addr+i,data_b);
				}

			}
			return true;

		} else {

			cerr << "Warning access to out of core 0 memory ("  << hex << addr<<  dec<<  ") will be ignored" << endl;
			return false;
		}

	}


	sc_uint<32> spiAddrBack;
	if(isControlledBySpi) {

		while(fSpiDriver->GetStimulusPendingSize()  != 0) {
			cout << "Warning: SPI driver has pending transactions" <<endl;
			wait(100);
		}
		spiAddrBack=(fSpiDriver->GetBaseAddress() + ( (SPI_BACK_CORE_ROW_OFFSET) <<  26)   |  ( (SPI_BACK_CORE_COL_OFFSET) <<  20)  + SPI_RETURN_ADDR);
		StimulusEntry spiWriteEntry=StimulusEntry(spiAddrBack  , data, dstAddr , REGULAR_CTRL_MODE, data_mode);
		//cout << "-------------------- Drive spi WRITE-----" << hex <<  spiWriteEntry << dec << "-------------------" << endl;
		fSpiDriver->AddStimulus(spiWriteEntry);
	} else {


		while(fHostDriver->GetStimulusPendingSize()  != 0) {
			wait(5);
		}
		StimulusEntry stWriteEntry = StimulusEntry(0/*no source address*/,data, dstAddr  ,0, data_mode );
		//cout << "-------------------- Drive GDB WRITE-----" << hex <<  stWriteEntry << dec << "-------------------" << endl;
		fHostDriver->AddStimulus(stWriteEntry);
	}



	if(isControlledBySpi) {
		while(true) {
			wait(5);
			if( fSpiDriver->IsDataReady(spiAddrBack) ) {
				break;
			}
		}

		for( unsigned int i = 0 ; i < n_bytes ; i++ ) {

			sc_uint<8> byte_data;

			if(fSpiDriver->GetDataFromHost(spiAddrBack+i,byte_data)) {

			} else {
				assert(0);// data not found
			}


			fSpiDriver->ClearHostData(spiAddrBack + i );
		}
	}
	return true;

}



// Functions to access memory. All register access on the ATDSP is via memory
bool TNamedPipeMemServer::readMem32 (uint32_t  addr, uint32_t& data) {
	sc_uint<32> d_32;
	bool res=  (readMem (  addr, WORD_, d_32));
	data = d_32(31,0);;
	return res;
}

bool TNamedPipeMemServer::readMem16 (uint32_t  addr, uint16_t& data) {
	sc_uint<32> d_32;
	bool res = (readMem (  addr, SHORT_, d_32));
	data = d_32(15,0);
	return res;
}

bool  TNamedPipeMemServer::readMem8 (uint32_t  addr, uint8_t& data) {
	sc_uint<32> d_32;
	bool res = (readMem (  addr, BYTE_, d_32));
	data = d_32(7,0);
	return res;
}


bool      TNamedPipeMemServer::writeMem32 (uint32_t  addr,		uint32_t   value) {
	sc_uint<32> v = value;
	return writeMem(addr,WORD_,v);
}


bool      TNamedPipeMemServer::writeMem16 (uint32_t  addr,		uint16_t   value){
	sc_uint<32> v=0;
	v(15,0) = value;
	return writeMem(addr,SHORT_,v);
}


bool      TNamedPipeMemServer::writeMem8 (uint32_t  addr,		uint8_t   value){
	sc_uint<32> v=0;
	v(7,0) = value;
	return writeMem(addr,BYTE_,v);
}
