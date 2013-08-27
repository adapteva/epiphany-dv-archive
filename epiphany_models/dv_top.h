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

#ifndef DV_TOP_H_
#define DV_TOP_H_

#include "systemc.h"
#include <map>

using namespace std;

//----------

SC_MODULE(dv_top) {
	/*AUTOATTR(verilated)*/

public:

	// PORTS
	sc_in<bool>	cclk;
	sc_in<bool>	reset;
	sc_in<bool>	itrace;
	sc_out<bool>	dut_passed;
	sc_out<bool>	dut_stopped;
	sc_out<bool>	dut_failed;
	sc_in<uint32_t>	ext_spi_datamode;
	sc_in<uint32_t>	ext_spi_ctrlmode;
	sc_in<bool>	ext_spi_access;
	sc_in<bool>	ext_spi_write;
	sc_in<bool>	ext_spi_wait;
	sc_out<uint32_t>	dut_spi_datamode;
	sc_out<uint32_t>	dut_spi_ctrlmode;
	sc_out<bool>	dut_spi_access;
	sc_out<bool>	dut_spi_write;
	sc_out<bool>	dut_spi_wait;
	sc_in<uint32_t>	ext_west_datamode;
	sc_in<uint32_t>	ext_west_ctrlmode;
	sc_in<bool>	ext_west_access;
	sc_in<bool>	ext_west_write;
	sc_in<bool>	ext_west_wait;
	sc_out<uint32_t>	dut_west_datamode;
	sc_out<uint32_t>	dut_west_ctrlmode;
	sc_out<bool>	dut_west_access;
	sc_out<bool>	dut_west_write;
	sc_out<bool>	dut_west_wait;
	sc_in<uint32_t>	ext_east_datamode;
	sc_in<uint32_t>	ext_east_ctrlmode;
	sc_in<bool>	ext_east_access;
	sc_in<bool>	ext_east_write;
	sc_in<bool>	ext_east_wait;
	sc_out<uint32_t>	dut_east_datamode;
	sc_out<uint32_t>	dut_east_ctrlmode;
	sc_out<bool>	dut_east_access;
	sc_out<bool>	dut_east_write;
	sc_out<bool>	dut_east_wait;
	sc_in<uint32_t>	ext_north_datamode;
	sc_in<uint32_t>	ext_north_ctrlmode;
	sc_in<bool>	ext_north_access;
	sc_in<bool>	ext_north_write;
	sc_in<bool>	ext_north_wait;
	sc_out<uint32_t>	dut_north_datamode;
	sc_out<uint32_t>	dut_north_ctrlmode;
	sc_out<bool>	dut_north_access;
	sc_out<bool>	dut_north_write;
	sc_out<bool>	dut_north_wait;
	sc_in<uint32_t>	ext_south_datamode;
	sc_in<uint32_t>	ext_south_ctrlmode;
	sc_in<bool>	ext_south_access;
	sc_in<bool>	ext_south_write;
	sc_in<bool>	ext_south_wait;
	sc_out<uint32_t>	dut_south_datamode;
	sc_out<uint32_t>	dut_south_ctrlmode;
	sc_out<bool>	dut_south_access;
	sc_out<bool>	dut_south_write;
	sc_out<bool>	dut_south_wait;
	sc_in<uint32_t>	ext_spi_dstaddr;
	sc_in<uint32_t>	ext_spi_srcaddr;
	sc_in<uint32_t>	ext_spi_data;
	sc_out<uint32_t>	dut_spi_srcaddr;
	sc_out<uint32_t>	dut_spi_dstaddr;
	sc_out<uint32_t>	dut_spi_data;
	sc_in<uint32_t>	ext_west_dstaddr;
	sc_in<uint32_t>	ext_west_srcaddr;
	sc_in<uint32_t>	ext_west_data;
	sc_out<uint32_t>	dut_west_srcaddr;
	sc_out<uint32_t>	dut_west_dstaddr;
	sc_out<uint32_t>	dut_west_data;
	sc_in<uint32_t>	ext_east_dstaddr;
	sc_in<uint32_t>	ext_east_srcaddr;
	sc_in<uint32_t>	ext_east_data;
	sc_out<uint32_t>	dut_east_srcaddr;
	sc_out<uint32_t>	dut_east_dstaddr;
	sc_out<uint32_t>	dut_east_data;
	sc_in<uint32_t>	ext_north_dstaddr;
	sc_in<uint32_t>	ext_north_srcaddr;
	sc_in<uint32_t>	ext_north_data;
	sc_out<uint32_t>	dut_north_srcaddr;
	sc_out<uint32_t>	dut_north_dstaddr;
	sc_out<uint32_t>	dut_north_data;
	sc_in<uint32_t>	ext_south_dstaddr;
	sc_in<uint32_t>	ext_south_srcaddr;
	sc_in<uint32_t>	ext_south_data;
	sc_out<uint32_t>	dut_south_srcaddr;
	sc_out<uint32_t>	dut_south_dstaddr;
	sc_out<uint32_t>	dut_south_data;

	// INTERNAL VARIABLES

	// PARAMETERS

	// METHODS

private:
	unsigned incr_cycle_count;


public:

	struct RestponseEntry {
		sc_uint<32> dstAddr,srcAddr;
		sc_uint<2>  data_size;
		RestponseEntry(sc_uint<32> DstAddr,sc_uint<2>  dataSize, sc_uint<32> SrcAddr) {
			dstAddr=DstAddr;
			data_size=dataSize;
			srcAddr=SrcAddr;
		}
		RestponseEntry() {dstAddr=0;data_size=0;srcAddr=0;}
	};

	void ClockPrint() {
		while(true) {

			//cout << "time " << sc_time() << endl;
			wait(1);
		}
	}


	void main_north_master()  {
		while(true) {
			wait(1);
		}
	}


	void main_north_respond() {
		dut_passed.write(0);
		dut_stopped.write(0);
		dut_failed.write(0);

		dut_north_write.write(false);
		dut_north_access.write(false);

		while(true) {
			if(!ext_north_wait) {

				if(!respondFIFO.empty()) {
					RestponseEntry restponseEntry = respondFIFO[0];
					respondFIFO.erase(respondFIFO.begin());

					dut_north_ctrlmode.write(0);
					dut_north_dstaddr.write(restponseEntry.srcAddr);
					dut_north_srcaddr.write(restponseEntry.dstAddr);//only for debug --not used since write
					dut_north_datamode.write(restponseEntry.data_size);
					dut_north_write.write(true);
					dut_north_access.write(false);

					sc_uint<32> addr = restponseEntry.dstAddr;
					sc_uint<32> data = 0xffffffff;// used for easy monitoring
					for ( unsigned i = 0 ; i < (1 << (restponseEntry.data_size)) ; i++ ) {
						if(memArray.find(addr+i) != memArray.end()) {
							data(8*(i+1)-1,8*i) = memArray[addr+i];
						}
					}
					dut_north_data.write(data);

//					cout << "Dut response from " <<  hex << restponseEntry.dstAddr << " to " << restponseEntry.srcAddr << " [" << data << "]" << dec << endl;


					incr_cycle_count=0;

				} else {

					dut_north_write.write(false);
					dut_north_access.write(false);

				}

			}

			incr_cycle_count++;
			if(incr_cycle_count > 1000) {
				cout << "DV DUT no transactions ... end of test ???" << endl;
				dut_stopped.write(1);

			}
			wait(1);
		}
	}

	void main_north_slave() {


		while(true) {

			if(ext_north_access.read() == true && ext_north_write.read() == true ) {
				cerr << "Protocol violation(Write & Read enable in the same cycle)  @ " << sc_time() << endl;
				//assert(0);
				wait(10);
				sc_stop();
			}



			if(ext_north_access.read() == true) {


				assert(ext_north_datamode.read()<3);
#ifdef DEBUG
				for ( unsigned i = 0 ; i < (1 << (ext_north_datamode.read())) ; i++ ) {
					sc_uint<8> data =0xff;

					cout << hex << "Read " << (ext_north_dstaddr.read() +i) << "===>" << ( ext_north_srcaddr.read() +i);
					if(memArray.find((ext_north_dstaddr.read() +i)) != memArray.end()) {
						data = memArray[(ext_north_dstaddr.read() +i)];
						cout << "(" << hex << data << ")" << dec;
					}
					cout << endl;
				}
#endif
				respondFIFO.push_back(RestponseEntry(ext_north_dstaddr.read(),ext_north_datamode.read(),ext_north_srcaddr.read()));

				incr_cycle_count=0;
			}

			if(ext_north_write.read() == true) {


				assert(ext_north_datamode.read()<3);
				sc_uint<32> data = sc_uint<32>(ext_north_data.read());

				for ( unsigned i = 0 ; i < (1 << (ext_north_datamode.read())) ; i++ ) {
					sc_uint<32> addr = ext_north_dstaddr.read()+i;

					memArray[addr] = data(8*(i+1)-1,8*i);
#ifdef DEBUG
					cout << hex << "Write " << addr << "===>" << memArray[addr]  << endl;
#endif
				}
				incr_cycle_count=0;
			}

//			unsigned int do_wait = rand() % 5;
//			for(unsigned k =0 ; k < do_wait; k++) {
//				dut_north_wait.write(1);
//				wait(1);
//			}

			dut_north_wait.write(0);
			wait(1);


		}
	}


	SC_CTOR(dv_top) {
		SC_THREAD(main_north_slave);     //called on every clock cycle
		sensitive << cclk.neg(); //driving stimulus on opposite edge to solve hold problem

		SC_THREAD(main_north_master);//TODO
		sensitive << cclk.pos();

		SC_THREAD(main_north_respond);
		sensitive << cclk.pos();

		SC_THREAD(ClockPrint);
		sensitive << cclk.pos();

		incr_cycle_count=0;
	}


private:
	map< sc_uint<32> , sc_uint<8> > memArray;

	vector<RestponseEntry> respondFIFO;

};


#endif /*guard*/
