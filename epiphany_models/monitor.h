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

#ifndef _MONITOR_H_
#define _MONITOR_H_

#include <stdio.h>
#include <string>
#include "systemc.h"
#include <map>

#ifndef DV_ONLY
#include "top_wrapper.h"
#else
#include "dv_top.h"
#endif

#include "maddr_defs.h"
#include "stimulus.h"

extern bool batch_mode;
extern bool usestdio;

SC_MODULE(Monitor){

	//Inputs
	sc_in<bool>                clk;
	sc_in<bool>                dut_passed;
	sc_in<bool>                dut_failed;
	sc_in<bool>                dut_stopped;

	//local variable
	int timeout;

	//spi mode monitor teh core status from spi port or / host port
	bool spi_mode;

	//Initialize the Simulation
	void initialize(int timeout);

	//Monitor loops compares cycle count with argument passed into simulation
	void  monitor_loop();

	//Monitor for trap and support for printf  (see new lib for more info)
	void StdioHandler();

	//stop test
	void stop_test();



	//Constructor
	//Advance stimulus on the positive edge of the clock
	SC_CTOR(Monitor){



		if(!batch_mode) {

			spi_mode = true;

			SC_THREAD(monitor_loop); //called on every clock cycle
			sensitive << clk.neg();  //Sensitivity of above thread

			SC_THREAD(CheckCoreStatus); //called on every clock cycle
			sensitive << clk.neg();  //Sensitivity of above thread
		} else {

			spi_mode = false;

			if(usestdio) {
				SC_THREAD(StdioHandler);
				sensitive << clk.neg();
			}
		}




		fExpectedDataFromDut=0;

		istim_north=0;
		istim_south=0;
		istim_east=0;
		istim_west=0;
		istim_spi=0;

		fSpiDriver=0;

	}


	void SetDataExpected(map<sc_uint<32> , sc_uint<8> > *ExpectedDataFromDut) {
		fExpectedDataFromDut=ExpectedDataFromDut;
	}


public:

	void SetStimulusRef(	Stimulus* _north,	Stimulus* _south,	Stimulus* _east,	Stimulus* _west,	Stimulus* _spi) {
		istim_north=_north;
		istim_south=_south;
		istim_east=_east;
		istim_west=_west;
		istim_spi=_spi;
	}


	/* spi will check the program flow */
	void SetSpiDriver(Stimulus* d) { fSpiDriver = d; }

private:
	map<sc_uint<32> , sc_uint<8> > *fExpectedDataFromDut;


	Stimulus* istim_north;
	Stimulus* istim_south;
	Stimulus* istim_east;
	Stimulus* istim_west;
	Stimulus* istim_spi;

	Stimulus*fSpiDriver;

	/* check if are there pending transactions in the hosts */

	bool isHostDriversDone() {
		bool res= (istim_north->GetStimulusPendingSize() == 0 ) &&
				(istim_south->GetStimulusPendingSize() == 0 ) &&
				(istim_east->GetStimulusPendingSize() == 0 ) &&
				(istim_west->GetStimulusPendingSize() == 0 ) &&
				(istim_spi->GetStimulusPendingSize() == 0 ) &&

				(istim_north->GetClearISSlaveInfActive() == false)  &&
				(istim_south->GetClearISSlaveInfActive() == false)  &&
				(istim_east->GetClearISSlaveInfActive() == false)  &&
				(istim_west->GetClearISSlaveInfActive() == false)  &&
				(istim_spi->GetClearISSlaveInfActive() == false) ;

		return res;
	}

public:
	void CheckCoreStatus() ;

public:
	//help function to get MMR register from Core/Chip according to core enumeration
	sc_uint<32> GetDataByHost(unsigned ncore, sc_uint<32> mmrRegAddr,unsigned dataMode ) ;
	sc_uint<32> SeDataByHost(unsigned ncore, sc_uint<32> mmrRegAddr, sc_uint<32> data,unsigned dataMode);
};



#endif
