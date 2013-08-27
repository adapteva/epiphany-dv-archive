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

#ifndef TNamedPipeMemServer_H_
#define TNamedPipeMemServer_H_

#include "systemc.h"
#include "stimulus.h"

//-----------------------------------------------------------------------------
//! Module implementing a verilator tar

//! A thread invoked by RSP memory access commands and build the entry transactions list in stimulus
//! The call is blocking. The gdb doesn't gain control until the call is done
//-----------------------------------------------------------------------------



SC_MODULE (TNamedPipeMemServer) {
public:

	//clock for main loop thread
	sc_in<bool>                clk;

	enum ETRAN_TYPE { BYTE_, SHORT_ , WORD_ };

	//Constructor
	SC_CTOR(TNamedPipeMemServer) {
		fSpiDriver=0;
		fHostDriver=0;

		fCoreID=0;

		SC_THREAD (mainloop);
		sensitive << clk.neg();  //Sensitivity of above thread

		isControlledBySpi=false;

	}

	//main loop to care of incoming requests from gdb server
	void mainloop();

	//Bind stimulus driver to get control
	void BindSpiStimulusDriver (Stimulus*fSpiDriver_) {
		fSpiDriver=fSpiDriver_;
		isControlledBySpi=true;
	}
	void BindHostStimulusDriver (Stimulus*fHostDriver_) {
		fHostDriver=fHostDriver_;
		isControlledBySpi=false;
	}


	// Functions to access memory. All register access on the ATDSP is via memory
	bool  readMem32 (uint32_t  addr,uint32_t&);
	bool      writeMem32 (uint32_t  addr,			uint32_t   value);

	bool  readMem16 (uint32_t  addr, uint16_t&);
	bool      writeMem16 (uint32_t  addr,			uint16_t   value);

	bool   readMem8 (uint32_t  addr, uint8_t&);
	bool      writeMem8 (uint32_t  addr,			uint8_t   value);

	void SetCoreID(unsigned ID){ fCoreID=ID; }//printf( "set core id %d\n",fCoreID);
	unsigned GetCoreID() { return fCoreID; }

private :
	Stimulus*fSpiDriver;
	Stimulus*fHostDriver;
private :
	bool  readMem (uint32_t  addr, ETRAN_TYPE t_type, sc_uint<32>&);
	bool  writeMem (uint32_t  addr, ETRAN_TYPE t_type, sc_uint<32>  data);

	unsigned fCoreID;

	bool isControlledBySpi;

};


#endif /* TARGETCNTRL_H_ */
