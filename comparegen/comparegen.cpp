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

#include <systemc.h>
#include <vector>
#include <set>
#include <iterator>

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <map>
#include <set>

sc_uint<6> chip_id = 0x24;
unsigned dma_sr_region_address=0;

#define ATDSP_DEFAULT_MEM_SIZE 0x100000 /* 1M from  ./sim/atdsp/sim-main.h:*/
using namespace std;

string result_file_name = "dut_trans_data_out.txt";


bool no_check_for_garbage_out = false;

void usage() {
	cerr << "Usage " << "comparegen -result_file <dut_trans_data_out.txt> -c <num cores>"   << endl;
}



int sc_main (int argc, char** argv)
{



	for (unsigned int n = 1; n < argc; n++) {

		if(!strcmp(argv[n],"-dma_sr_region_address")) {
			n+=1;
			if(n< argc) {
				//NCORES = (std::atoi(argv[n]));
				dma_sr_region_address=(std::strtoul(argv[n],NULL,16));
				cout << "dma_sr_region_address" << hex << dma_sr_region_address << dec << endl;
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


		if(!strcmp(argv[n],"-c")) {
			n+=1;
			if(n< argc) {
				//NCORES = (std::atoi(argv[n]));

			} else {
				usage();
				return 3;
			}
			continue;
		}
		if(!strcmp(argv[n],"-result_file")) {
			n+=1;
			if(n< argc) {
				result_file_name = argv[n];

			} else {
				usage();
				return 3;
			}
			continue;
		}
		if(!strcmp(argv[n],"-no_check_for_garbage_out")) {

			no_check_for_garbage_out = true;
			continue;
		}

		//should not get here
		usage();
		return 55;

	}


	map< sc_uint<32>, sc_uint<8> > fDutMem;//smapled bt host: dut out
	map< sc_uint<32>, sc_uint<8> > fCoreMem;//result of cgen simulator
	map< sc_uint<32>, sc_uint<8> > fHostMem;//result of mgen host transactor
	set< sc_uint<32>  >		       fHostAddrAccess;// result of mgen host transactor

	//parse the result_file --- the file produced by DUT
	cout << " reading result dut file " << result_file_name.c_str() << endl;
	ifstream tmp_file(result_file_name.c_str());

	while(tmp_file.good()) {
		char c;
		tmp_file>> c;
		if(tmp_file.eof()) break;
		else tmp_file.putback(c);

		unsigned long val;
		tmp_file>> noshowbase >> hex  >> val;
		sc_uint<32> addr = val;

		//strip out host space;
		addr(31,29)=0;
		addr(25,23)=0;

		tmp_file>> noshowbase >> hex >> val;
		sc_uint<8> data = val;
		//cerr << hex << addr << " :::" << data << endl;
		fDutMem[addr]=data;
	}

	tmp_file.close();




	bool testfail = false;
	cout << " first comparison ::: Cores data ... core_mem_data_result.txt" << endl;
	//  result cgen simulator run
	tmp_file.open("core_mem_data_result.txt");

	while(tmp_file.good()) {
		char c;
		tmp_file>> c;
		if(tmp_file.eof()) break;
		else tmp_file.putback(c);

		unsigned long val;
		tmp_file>> noshowbase >> hex  >> val;
		sc_uint<32> addr = val;

		//strip out host space;
		addr(31,29)=0;
		addr(25,23)=0;

		tmp_file>> noshowbase >> hex >> val;
		sc_uint<8> data = val;
		//cerr << hex << addr << " :::" << data << endl;
		fCoreMem[addr]=data;
	}
	tmp_file.close();





	for( map<sc_uint<32> , sc_uint<8> >::iterator it =   fCoreMem.begin(); it !=  fCoreMem.end(); it++ ) {
		sc_uint<32> addr = it->first;
		//TODO check address (31,26)

		if(fDutMem.find(addr)  != fDutMem.end() ) {
			if(  fDutMem[addr]  == fCoreMem[addr] ) {
				//GREAT !!!!!!!!!!!
			} else {

				//if(addr(19,0) >= 0x800/*first slice in mgen */ )//TODO
				//{
				cerr << "Expected CORE data for address " << hex <<  it->first << " is " << fCoreMem[addr]  << " but got " <<  fDutMem[addr]  << endl;
				testfail=true;
				break;
				//} else {
				//	cerr << "Ignoring address ISR save/restore space TODO -- more precise calculation <0x800 " << hex << addr << endl;
				//}
			}

		} else {
			if(  addr(19,0) >= 0xf0000 &&  addr(19,0)  <   0xf0200 ) {
				cerr << "Ignoring address MMR space " << hex << addr << endl;
			} else {
				cerr << "Error(comparegen):Expected CORE address not found " << hex << it->first << endl;
				testfail=true;
				break;
			}
		}
	}



	cout << " second comparison ::: Host data ... host_expected_data_out.txt" << endl;
	//  result of mgen : host random accees to memory
	tmp_file.open("host_expected_data_out.txt");
	while(tmp_file.good()) {
		char c;
		tmp_file>> c;
		if(tmp_file.eof()) break;
		else tmp_file.putback(c);

		unsigned long val;
		tmp_file>> noshowbase >> hex  >> val;
		sc_uint<32> addr = val;

		//strip out host space;
		addr(31,29)=0;
		addr(25,23)=0;

		tmp_file>> noshowbase >> hex >> val;
		sc_uint<8> data = val;
		//cerr << hex << addr << " :::" << data << endl;
		fHostMem[addr]=data;
	}
	tmp_file.close();


	for( map<sc_uint<32> , sc_uint<8> >::iterator it =   fHostMem.begin(); it !=  fHostMem.end(); it++ ) {
		sc_uint<32> addr = it->first;
		//TODO check address ext

		if(fDutMem.find(addr)  != fDutMem.end() ) {
			if(  fDutMem[addr]  == fHostMem[addr] ) {

			} else {
				cerr << "Expected Host transaction data for address " << hex <<  it->first << " is " << fHostMem[addr]  << " but got " <<  fDutMem[addr]  << endl;
				testfail=true;
				break;
			}

		} else {
			cerr << "Error(comparegen): Expected Host transaction address not found " << hex << it->first << endl;
			testfail=true;
			break;
		}
	}

	cout << " third comparison ::: host address read  (even from uninitialized ) ... host_address_read.txt" << endl;
	tmp_file.open("host_address_read.txt");
	while(tmp_file.good()) {
		char c;
		tmp_file>> c;
		if(tmp_file.eof()) break;
		else tmp_file.putback(c);

		unsigned long val;
		tmp_file>> noshowbase >> hex  >> val;
		sc_uint<32> addr = val;


		//TODO
		//strip out host space;
		addr(31,29)=0;
		addr(25,23)=0;

		//cerr << hex << addr  << endl;
		fHostAddrAccess.insert(addr);
	}
	tmp_file.close();

	for( set<sc_uint<32> >::iterator it =   fHostAddrAccess.begin(); it !=  fHostAddrAccess.end(); it++ ) {
		sc_uint<32> addr = *it;
		if(fDutMem.find(addr)  == fDutMem.end() ) {
			cerr << "Error(comparegen): Expected Host read transaction for address never returned back" << hex << addr << endl;
			testfail=true;
			break;
		}
	}
	//TODO
	cout << " fourth comparison ::: slave DMA data ... TODO" << endl;
	//TODO
	if(no_check_for_garbage_out) {
		cout << "NO fifth comparison ::: checking for 'garbage' out from chip for core due to DMA, HOST or interrupt ON (only core master has been supported in DV)" << endl;
	}
	else {
		cout << " fifth comparison ::: checking for 'garbage' out from chip for core, DMA and HOST -- TODO" << endl;
		vector<sc_uint<32> > hostAddrIncomingCoreDut;
		//
		tmp_file.open("host_addr_incoming_core_dut.mem");
		while(tmp_file.good()) {
			char c;
			tmp_file>> c;
			if(tmp_file.eof()) break;
			else tmp_file.putback(c);

			unsigned long val;
			tmp_file>> noshowbase >> hex  >> val;
			sc_uint<32> addr = val;


			//cerr << hex << addr  << endl;
			//fHostAddrAccess.insert(addr);
			if(  !( (addr(19,0) >= 0xf0000) &&  (addr(19,0)  <   0xf0800)) ) {
				//cerr << "\t ++++ " << endl;
				hostAddrIncomingCoreDut.push_back(addr);
			}

		}
		tmp_file.close();

		//cout << "+++++++++++++++++++" << endl;

		vector<sc_uint<32> > hostAddrIncomingCoreExpected;
		tmp_file.open("host_addr_incoming_core_expected.mem");
		while(tmp_file.good()) {
			char c;
			tmp_file>> c;
			if(tmp_file.eof()) break;
			else tmp_file.putback(c);

			unsigned long val;
			tmp_file>> noshowbase >> hex  >> val;
			sc_uint<32> addr = val;


			//cerr << hex << addr  << endl;
			//fHostAddrAccess.insert(addr);
			if(  (addr(31,20) != 0) &&   ( (addr(31,29) != chip_id(5,3)) || (addr(25,23) != chip_id(2,0)) ) ) {
				//cerr << "\t ++++ " << endl;
				hostAddrIncomingCoreExpected.push_back(addr);
			}

		}
		tmp_file.close();



		if(hostAddrIncomingCoreExpected.size() != hostAddrIncomingCoreDut.size()) {
			cerr << "Error(comparegen): This is 'garbage' out coming to host from cores, check the host_addr_incoming_core_expected_vs_dut.mem originated from check *.mem files"  << endl;


			ofstream f_expected_vs_dut_mem("host_addr_incoming_core_expected_vs_dut.mem");

			f_expected_vs_dut_mem << "Recored cores writes to hosts" << endl;


			for(vector<sc_uint<32> >::iterator it = hostAddrIncomingCoreDut.begin() ; it !=  hostAddrIncomingCoreDut.end() ; it++ ) {
				f_expected_vs_dut_mem << hex << *it << dec << endl;
			}

			f_expected_vs_dut_mem << "Expected cores writes to hosts" << endl;
			for(vector<sc_uint<32> >::iterator it = hostAddrIncomingCoreExpected.begin() ; it !=  hostAddrIncomingCoreExpected.end() ; it++ ) {
				f_expected_vs_dut_mem << hex << *it << dec << endl;
			}

			f_expected_vs_dut_mem.close();

			testfail=true;
		}

	}

	if(testfail ) {
		cout << "TEST FAIL >>>>>>>>>>>>> FIX design NOW" << endl;
		return -3;
	} else {
		cout << "TEST PASS >>>>>>>>>>>>> TAPEOUT NOW" << endl;
	}
	return 0;
}

