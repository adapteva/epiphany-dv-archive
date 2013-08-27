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

static char *sc_main_verilator_ver = "$Rev: 1307 $";

#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <sys/times.h>
#include <sys/stat.h>


#include "systemc.h"	  //SystemC + SystemPerl global header
#include "vmodel_utils.h"

#include "maddr_defs.h"

#ifndef DV_ONLY
#include "top_wrapper.h"         //Verilated RTL Header
#else
#include "dv_top.h"
#endif

using namespace std;


#include "stimulus.h"    //Stimulus Block
#include "monitor.h"     //Monitor Block


// the target control frontend
#include "namedPipeMemServer.h"


unsigned   NCORES = 1;

int load_stimulus(char *filename, vector<StimulusEntry>&);

#define MAX_NCORES 64
//since the compiled design has holes, need map count core to core num
unsigned fMapCoreCountToCoreNum[MAX_NCORES];

unsigned  NORTH_BASE;
unsigned SOUTH_BASE;
unsigned WEST_BASE ;
unsigned EAST_BASE ;

unsigned CHIP_BASE	 ;
unsigned long force_stop_at_addr = 0;

bool random_stop_resume_on=false;

bool usestdio =false;

void usage() {
	std::cerr << "USAGE>>: atdsp-unknown-elf-vmodel-run [-version]\n" <<
			"[ -northfile <north.dat>  ]\n[ -southfile  <south.dat> ]\n[ -eastfile <east.dat> ]\n" <<
			"[ -westfile  <west.dat> ]\n[ -spifile <spi.dat> ]\n" <<
			"[ -vcdfile <vcd_filename> ]\n" <<
			"[ -ptrace <on=1/off=0> ] [ -vcdlevel <on=[1..]/off=0> ] [ -cclk_period  <cclk period> ]  \n" <<
			"[ -ncycles <max number cycles  until timeout = 160000> ]" <<
			"\n[ -batch [ -usestdio] [-trace_external  [-force_stop_at_addr <external_addr> ]  ] ]"
			"\n[ -gdb_server  [ -always_record ] [-trace_external  [-force_stop_at_addr <external_addr> ]  ] [-spi_gdb_server | -north_gdb_server | -south_gdb_server | -west_gdb_server | -east_gdb_server] <west> ] \n"<<
			hex << "[-north_base <addr = "<< NORTH_BASE_DEFAULT << ">]\n" <<
			hex << "[-south_base <ddr ="<< SOUTH_BASE_DEFAULT << ">]\n" <<
			hex << "[-west_base <addr ="<< WEST_BASE_DEFAULT << ">]\n" <<
			hex << "[-east_base <addr ="<< EAST_BASE_DEFAULT << ">]\n" <<
			hex << "[-chip_base <addr ="<< CHIP_BASE_DEFAULT << ">]\n" <<
			" [ -n <numbers cores =1 >] " <<  std::endl;
	std::cerr << "Examples: \n";
	std::cerr << "\t Batch mode: -batch -westfile host_load_west.txt -force_stop_at_addr 81c01ae4 -trace_external -always_record_wave -vcdlevel 99" <<std::endl;
}

#include "traceSupport.h"


TraceSupport* traceFileCtl=0;

/*
 * Use the HW env mode:
 * 1. The don't apply checking of address for host driving transactions
 */

bool batch_mode= false;
bool gdb_server=false;

bool spi_gdb_server=false;
bool north_gdb_server=false;
bool south_gdb_server=false;
bool west_gdb_server=true;
bool east_gdb_server=false;


bool always_record_wave=false;
bool trace_external=false;


static char const * revision= "$Rev: 1307 $";

int sc_main(int argc, char* argv[]) {


	NORTH_BASE =NORTH_BASE_DEFAULT;
	SOUTH_BASE =SOUTH_BASE_DEFAULT;
	WEST_BASE  =WEST_BASE_DEFAULT;
	EAST_BASE  =EAST_BASE_DEFAULT;
	CHIP_BASE  =CHIP_BASE_DEFAULT;

	sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", SC_DO_NOTHING);


	/**************************************************/
	/*Static Local Variables Used by Stimulus Engine  */
	/**************************************************/

	//WATCH OUT FOR THESE GETTING OVER-WRITTEN!!
	vector<StimulusEntry>  north_array;     //Declaring static data stimulus arrays
	vector<StimulusEntry>  south_array;     //
	vector<StimulusEntry>  east_array;      //
	vector<StimulusEntry>  west_array;      //
	vector<StimulusEntry>  spi_array;       //

	vector<StimulusEntry>  preconfig_array;       //Done before load starts
	//

	/**************************************************/
	/*Object Dumping Elf File                         */
	/**************************************************/
	//Reading Arguments

	char  preconfigfile[1024]; strcpy(preconfigfile,"custom_config.memh");
	char  northfile[1024]; strcpy(northfile,"dummy.memh");
	char  southfile[1024]; strcpy(southfile,"dummy.memh");
	char  eastfile[1024];  strcpy(eastfile,"dummy.memh");
	char  westfile[1024];  strcpy(westfile,"dummy.memh");
	char  spifile[1024];   strcpy(spifile,"dummy.memh");

	char  vcdfile[1024];   strcpy(vcdfile,"dump.vcd");

	int unsigned ptrace     = 1; //Program Trace(ON/OFF)
	int unsigned vcd        = 0;   //VCD Dump Level(0=OFF)
	int unsigned cycles     = 160000;   //clock cycles to simulate
	int unsigned cclkp      = 40;  //cclk period


	//init core num to real core num map
	for(unsigned c = 0 ; c <  MAX_NCORES ; c++ ) {
		fMapCoreCountToCoreNum[c]=c;
	}


	for (unsigned int n = 1; n < argc; n++) {

		if(!strcmp(argv[n],"-version")) {
			std::cerr << " SNV revision:" << revision << std::endl;

			return 1;
		}


		if(!strcmp(argv[n],"-h")) {
			usage();
			return 1;
		}
		if(!strcmp(argv[n],"--help")) {
			usage();
			return 1;
		}

		if(!strcmp(argv[n],"-gdb_server")) {

			if(n< argc) {
				gdb_server = true;
				cout << " Entering to gdb server mode ...." << endl;
			} else {
				usage();
				return 3;
			}
			continue;
		}

		if(!strcmp(argv[n],"-spi_gdb_server")) {

			if(n< argc) {
				spi_gdb_server = true;
				cout << " The gdb server will control the target by spi port ...." << endl;
			} else {
				usage();
				return 3;
			}
			continue;
		}

		if(!strcmp(argv[n],"-north_gdb_server")) {

			if(n< argc) {
				north_gdb_server = true;
				cout << " The gdb server will control the target by north port ...." << endl;
			} else {
				usage();
				return 3;
			}
			continue;
		}

		if(!strcmp(argv[n],"-south_gdb_server")) {

			if(n< argc) {
				south_gdb_server = true;
				cout << " The gdb server will control the target by south port ...." << endl;
			} else {
				usage();
				return 3;
			}
			continue;
		}


		if(!strcmp(argv[n],"-west_gdb_server")) {

			if(n< argc) {
				west_gdb_server = true;
				cout << " The gdb server will control the target by west port ...." << endl;
			} else {
				usage();
				return 3;
			}
			continue;
		}

		if(!strcmp(argv[n],"-east_gdb_server")) {

			if(n< argc) {
				east_gdb_server = true;
				cout << " The gdb server will control the target by east port ...." << endl;
			} else {
				usage();
				return 3;
			}
			continue;
		}

		if(!strcmp(argv[n],"-trace_external")) {

			if(n< argc) {
				trace_external = true;
				cout << "...(GDG SERVER/BATCH MODE)......The all external transactions will be traced" << endl;
			} else {
				usage();
				return 3;
			}
			continue;
		}


		if(!strcmp(argv[n],"-batch")) {

			if(n< argc) {
				batch_mode = true;
				cout << " Running in the batch mode " << endl;
			} else {
				usage();
				return 3;
			}
			continue;
		}

		if(!strcmp(argv[n],"-usestdio")) {

			if(n< argc) {
				usestdio = true;
				cout << "Supporting stdio " << endl;
			} else {
				usage();
				return 3;
			}
			continue;
		}



		if(!strcmp(argv[n],"-gdb_multi")) {

			if(n< argc) {

				cerr << "This option has been depricated .. use -n " << endl;
				//cout << "  Multicore mode is ON " << endl;
			} else {
				usage();
				return 3;
			}
			continue;
		}




		if(!strcmp(argv[n],"-random_stop_resume_on")) {

			if(n< argc) {

				random_stop_resume_on=true;

			} else {
				usage();
				return 3;
			}
			continue;
		}

		if(!strcmp(argv[n],"-always_record_wave")) {

			if(n< argc) {
				always_record_wave = true;
				cout << " The wave recording will be NOT controlled by gdb tstart/tstop commands !!! " << endl;
			} else {
				usage();
				return 3;
			}
			continue;
		}


		if(!strcmp(argv[n],"-cclk_period")) {
			n+=1;
			if(n< argc) {
				cclkp      = atoi(argv[n]);  //cclk period
				cout << "setting cclk period " <<  cclkp << endl;
			} else {
				usage();
				return 3;
			}
			continue;
		}

		if(!strcmp(argv[n],"-n")) {
			n+=1;
			if(n< argc) {
				NCORES      = atoi(argv[n]);
				cout << "setting number cores to   " <<  NCORES << endl;
				assert(NCORES>0 && NCORES<=16);
			} else {
				usage();
				return 3;
			}
			continue;
		}
		if(!strcmp(argv[n],"-seed")) {
			n+=1;
			if(n< argc) {

				srand (atoi(argv[n]));

			} else {
				usage();
				return 3;
			}
			continue;
		}

		if(!strcmp(argv[n],"-north_base")) {
			n+=1;
			if(n< argc) {
				NORTH_BASE    = strtol(argv[n], NULL, 16);
				cout << "setting north base to   " <<  hex << NORTH_BASE  <<dec << endl;

			} else {
				usage();
				return 3;
			}
			continue;
		}
		if(!strcmp(argv[n],"-west_base")) {
			n+=1;
			if(n< argc) {
				WEST_BASE    = strtol(argv[n], NULL, 16);
				cout << "setting west base to   "<<  hex  <<  WEST_BASE <<dec << endl;

			} else {
				usage();
				return 3;
			}
			continue;
		}
		if(!strcmp(argv[n],"-south_base")) {
			n+=1;
			if(n< argc) {
				SOUTH_BASE    = strtol(argv[n], NULL, 16);
				cout << "setting south base to   " <<  hex <<  SOUTH_BASE <<dec << endl;

			} else {
				usage();
				return 3;
			}
			continue;
		}
		if(!strcmp(argv[n],"-east_base")) {
			n+=1;
			if(n< argc) {
				EAST_BASE    = strtol(argv[n], NULL, 16);
				cout << "setting east base to   " <<  hex << EAST_BASE <<dec  << endl;

			} else {
				usage();
				return 3;
			}
			continue;
		}

		if(!strcmp(argv[n],"-chip_base")) {
			n+=1;
			if(n< argc) {
				CHIP_BASE    = strtol(argv[n], NULL, 16);
				cout << "setting chip and spi bases to   "<<  hex  <<  CHIP_BASE <<dec << endl;

			} else {
				usage();
				return 3;
			}
			continue;
		}

		if(!strcmp(argv[n],"-force_stop_at_addr")) {
			n+=1;
			if(n< argc) {
				force_stop_at_addr    = strtol(argv[n], NULL, 16);
				cout << "..(GDG SERVER/BATCH MODE)......The simulation will be finished when detects the external access on the address    "<<  hex  <<  force_stop_at_addr <<dec << endl;

			} else {
				usage();
				return 3;
			}
			continue;
		}


		if(!strcmp(argv[n],"-ncycles")) {
			n+=1;
			if(n< argc) {

				cycles     = atoi(argv[n]);   //clock cycles to simulate
				cout << "setting number clock cycles to simulate " << cycles << endl;


			} else {
				usage();
				return 3;
			}
			continue;
		}

		if(!strcmp(argv[n],"-vcdlevel")) {
			n+=1;
			if(n< argc) {
				vcd  = atoi(argv[n]);   //VCD Dump Level(0=OFF)
				cout << "setting VCD Dump Level(0=OFF)" << vcd << endl;


			} else {
				usage();
				return 3;
			}
			continue;
		}

		if(!strcmp(argv[n],"-ptrace")) {
			n+=1;
			if(n< argc) {
				ptrace     = atoi(argv[n]);
				cout << "setting ptrace ON/OFF: " <<  ptrace << endl;

			} else {
				usage();
				return 3;
			}
			continue;
		}

		if(!strcmp(argv[n],"-vcdfile")) {
			n+=1;
			if(n< argc) {
				strcpy(vcdfile,argv[n]);
			} else {
				usage();
				return 3;
			}
			continue;
		}


		if(!strcmp(argv[n],"-core_0")) {
			n+=1;
			if(n< argc) {
				unsigned numCoreId = (std::strtol(argv[n], NULL, 10));
				assert(numCoreId<MAX_NCORES);
				fMapCoreCountToCoreNum[0]=numCoreId;

			} else {
				usage();
				return 3;
			}
			continue;
		}

		if(!strcmp(argv[n],"-core_1")) {
			n+=1;
			if(n< argc) {
				unsigned numCoreId = (std::strtol(argv[n], NULL, 10));
				assert(numCoreId<MAX_NCORES);
				fMapCoreCountToCoreNum[1]=numCoreId;

			} else {
				usage();
				return 3;
			}
			continue;
		}

		if(!strcmp(argv[n],"-core_2")) {
			n+=1;
			if(n< argc) {
				unsigned numCoreId = (std::strtol(argv[n], NULL, 10));
				assert(numCoreId<MAX_NCORES);
				fMapCoreCountToCoreNum[2]=numCoreId;

			} else {
				usage();
				return 3;
			}
			continue;
		}


		if(!strcmp(argv[n],"-core_3")) {
			n+=1;
			if(n< argc) {
				unsigned numCoreId = (std::strtol(argv[n], NULL, 10));
				assert(numCoreId<MAX_NCORES);
				fMapCoreCountToCoreNum[3]=numCoreId;

			} else {
				usage();
				return 3;
			}
			continue;
		}



		if(!strcmp(argv[n],"-northfile")) {
			n+=1;
			if(n< argc) {
				strcpy(northfile,argv[n]);
			} else {
				usage();
				return 3;
			}
			continue;
		}
		if(!strcmp(argv[n],"-southfile")) {
			n+=1;
			if(n< argc) {
				strcpy(southfile,argv[n]);
			} else {
				usage();
				return 3;
			}
			continue;
		}
		if(!strcmp(argv[n],"-westfile")) {
			n+=1;
			if(n< argc) {
				strcpy(westfile,argv[n]);
			} else {
				usage();
				return 3;
			}
			continue;
		}
		if(!strcmp(argv[n],"-eastfile")) {
			n+=1;
			if(n< argc) {
				strcpy(eastfile,argv[n]);
			} else {
				usage();
				return 3;
			}
			continue;
		}

		if(!strcmp(argv[n],"-spifile")) {
			n+=1;
			if(n< argc) {
				strcpy(spifile,argv[n]);
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



	//init core num to real core num map
	for(unsigned c = 0 ; c <  NCORES ; c++ ) {
		cerr << " core num " << c << " mapped to mesh core "   << fMapCoreCountToCoreNum[c] << endl;
	}
	cerr << "ROWs/COLs are "<< GET_NCORES_IN_ROW() << endl;

#ifndef DV_ONLY
	/**************************************************/
	/*Random Reset                                    */
	/**************************************************/
	Verilated::randReset(2);
#endif


	/**************************************************/
	/*Expected: retunted transaction and core write   */
	/*************************************************/
	map<sc_uint<32> , sc_uint<8> > *expectedDataFromDut = new map<sc_uint<32> , sc_uint<8> >();

	/**************************************************/
	/*Module Instantiation                            */
	/**************************************************/

	//Creating object, pointer, I don't know..whatever
#ifndef DV_ONLY
	top_wrap*      itop            = new top_wrap("iwtop");
#else
	dv_top*      itop            = new dv_top("iwtop");
#endif

	Monitor*  imon =0;
	if( !gdb_server) {
		imon            = new Monitor("imon");
	}

	Stimulus* istim_north     = new Stimulus("istim_north");
	Stimulus* istim_south     = new Stimulus("istim_south");
	Stimulus* istim_east      = new Stimulus("istim_east");
	Stimulus* istim_west      = new Stimulus("istim_west");
	Stimulus* istim_spi       = new Stimulus("istim_spi");


	if(!gdb_server) {
		//get monitor access to drivers&samples
		imon->SetStimulusRef(istim_north,istim_south,istim_east,istim_west,istim_spi);

		imon->SetSpiDriver(istim_spi);

		imon->SetDataExpected(expectedDataFromDut);
	}


	istim_north->SetDataExpected(expectedDataFromDut);
	istim_south->SetDataExpected(expectedDataFromDut);
	istim_east->SetDataExpected(expectedDataFromDut);
	istim_west->SetDataExpected(expectedDataFromDut);
	istim_spi->SetDataExpected(expectedDataFromDut);



	//TODO make for others
	istim_north->SetBaseAddress(NORTH_BASE);
	istim_south->SetBaseAddress(SOUTH_BASE);
	istim_east->SetBaseAddress(EAST_BASE);
	istim_west->SetBaseAddress(WEST_BASE);
	istim_spi->SetBaseAddress(CHIP_BASE);

	/**************************************************/
	/*Define Clock and Time Units                     */
	/**************************************************/
	sc_set_time_resolution(1,SC_NS);

	//SC CLOCK FORMAT
	//keyworkd name      reference    period   duty  offset
	sc_clock   cclk      ("cclk",     cclkp,   0.5,  5*cclkp,     true); //core clock

	/**************************************************/
	/*Signal Definitions                              */
	/**************************************************/
	//stop indicator from core
	sc_signal<bool>        dut_passed;
	sc_signal<bool>        dut_stopped;
	sc_signal<bool>        dut_failed;

	sc_signal<uint32_t>    x_id;
	sc_signal<uint32_t>    y_id;

	//Inputs to top
	sc_signal<bool>        stim_north_reset;
	sc_signal<bool>        stim_north_trace;
	sc_signal<bool>        stim_south_reset;
	sc_signal<bool>        stim_south_trace;
	sc_signal<bool>        stim_east_reset;
	sc_signal<bool>        stim_east_trace;
	sc_signal<bool>        stim_west_reset;
	sc_signal<bool>        stim_west_trace;
	sc_signal<bool>        stim_spi_reset;
	sc_signal<bool>        stim_spi_trace;

	//SPI
	sc_signal<bool>        stim_spi_wr_wait;
	sc_signal<bool>        stim_spi_rd_wait;
	sc_signal<bool>        stim_spi_write;
	sc_signal<bool>        stim_spi_access;
	sc_signal<uint32_t>    stim_spi_datamode;
	sc_signal<uint32_t>    stim_spi_ctrlmode;
	sc_signal<uint32_t>    stim_spi_dstaddr;
	sc_signal<uint32_t>    stim_spi_data;
	sc_signal<uint32_t>    stim_spi_srcaddr;
	sc_signal<bool>        dut_spi_write;
	sc_signal<bool>        dut_spi_access;
	sc_signal<uint32_t>    dut_spi_datamode;
	sc_signal<uint32_t>    dut_spi_ctrlmode;
	sc_signal<uint32_t>    dut_spi_dstaddr;
	sc_signal<uint32_t>    dut_spi_data;
	sc_signal<uint32_t>    dut_spi_srcaddr;
	sc_signal<bool>        dut_spi_wr_wait;
	sc_signal<bool>        dut_spi_rd_wait;

	//NORTH
	sc_signal<bool>        stim_north_wr_wait;
	sc_signal<bool>        stim_north_rd_wait;
	sc_signal<bool>        stim_north_write;
	sc_signal<bool>        stim_north_access;
	sc_signal<uint32_t>    stim_north_datamode;
	sc_signal<uint32_t>    stim_north_ctrlmode;
	sc_signal<uint32_t>    stim_north_dstaddr;
	sc_signal<uint32_t>    stim_north_data;
	sc_signal<uint32_t>    stim_north_srcaddr;
	sc_signal<bool>        dut_north_write;
	sc_signal<bool>        dut_north_access;
	sc_signal<uint32_t>    dut_north_datamode;
	sc_signal<uint32_t>    dut_north_ctrlmode;
	sc_signal<uint32_t>    dut_north_dstaddr;
	sc_signal<uint32_t>    dut_north_data;
	sc_signal<uint32_t>    dut_north_srcaddr;
	sc_signal<bool>        dut_north_wr_wait;
	sc_signal<bool>        dut_north_rd_wait;

	//SOUTH
	sc_signal<bool>        stim_south_wr_wait;
	sc_signal<bool>        stim_south_rd_wait;
	sc_signal<bool>        stim_south_write;
	sc_signal<bool>        stim_south_access;
	sc_signal<uint32_t>    stim_south_datamode;
	sc_signal<uint32_t>    stim_south_ctrlmode;
	sc_signal<uint32_t>    stim_south_dstaddr;
	sc_signal<uint32_t>    stim_south_data;
	sc_signal<uint32_t>    stim_south_srcaddr;
	sc_signal<bool>        dut_south_write;
	sc_signal<bool>        dut_south_access;
	sc_signal<uint32_t>    dut_south_datamode;
	sc_signal<uint32_t>    dut_south_ctrlmode;
	sc_signal<uint32_t>    dut_south_dstaddr;
	sc_signal<uint32_t>    dut_south_data;
	sc_signal<uint32_t>    dut_south_srcaddr;
	sc_signal<bool>        dut_south_wr_wait;
	sc_signal<bool>        dut_south_rd_wait;

	//EAST
	sc_signal<bool>        stim_east_wr_wait;
	sc_signal<bool>        stim_east_rd_wait;
	sc_signal<bool>        stim_east_write;
	sc_signal<bool>        stim_east_access;
	sc_signal<uint32_t>    stim_east_datamode;
	sc_signal<uint32_t>    stim_east_ctrlmode;
	sc_signal<uint32_t>    stim_east_dstaddr;
	sc_signal<uint32_t>    stim_east_data;
	sc_signal<uint32_t>    stim_east_srcaddr;
	sc_signal<bool>        dut_east_write;
	sc_signal<bool>        dut_east_access;
	sc_signal<uint32_t>    dut_east_datamode;
	sc_signal<uint32_t>    dut_east_ctrlmode;
	sc_signal<uint32_t>    dut_east_dstaddr;
	sc_signal<uint32_t>    dut_east_data;
	sc_signal<uint32_t>    dut_east_srcaddr;
	sc_signal<bool>        dut_east_wr_wait;
	sc_signal<bool>        dut_east_rd_wait;

	//WEST
	sc_signal<bool>        stim_west_wr_wait;
	sc_signal<bool>        stim_west_rd_wait;
	sc_signal<bool>        stim_west_write;
	sc_signal<bool>        stim_west_access;
	sc_signal<uint32_t>    stim_west_datamode;
	sc_signal<uint32_t>    stim_west_ctrlmode;
	sc_signal<uint32_t>    stim_west_dstaddr;
	sc_signal<uint32_t>    stim_west_data;
	sc_signal<uint32_t>    stim_west_srcaddr;
	sc_signal<bool>        dut_west_write;
	sc_signal<bool>        dut_west_access;
	sc_signal<uint32_t>    dut_west_datamode;
	sc_signal<uint32_t>    dut_west_ctrlmode;
	sc_signal<uint32_t>    dut_west_dstaddr;
	sc_signal<uint32_t>    dut_west_data;
	sc_signal<uint32_t>    dut_west_srcaddr;
	sc_signal<bool>        dut_west_wr_wait;
	sc_signal<bool>        dut_west_rd_wait;



	/**************************************************/
	/*DUT CONNECTIONS                                 */
	/**************************************************/

	itop->cclk(cclk);
	itop->reset(stim_north_reset);
	itop->itrace(stim_north_trace);
	itop->dut_passed(dut_passed);
	itop->dut_stopped(dut_stopped);
	itop->dut_failed(dut_failed);

	itop->x_id(x_id);
	itop->y_id(y_id);


	//NORTH
	itop->ext_north_data(stim_north_data);
	itop->ext_north_dstaddr(stim_north_dstaddr);
	itop->ext_north_srcaddr(stim_north_srcaddr);
	itop->ext_north_write(stim_north_write);
	itop->ext_north_access(stim_north_access);
	itop->ext_north_datamode(stim_north_datamode);
	itop->ext_north_ctrlmode(stim_north_ctrlmode);
	itop->ext_north_wr_wait(stim_north_wr_wait);
	itop->ext_north_rd_wait(stim_north_rd_wait);

	itop->dut_north_write(dut_north_write);
	itop->dut_north_access(dut_north_access);
	itop->dut_north_datamode(dut_north_datamode);
	itop->dut_north_ctrlmode(dut_north_ctrlmode);
	itop->dut_north_data(dut_north_data);
	itop->dut_north_srcaddr(dut_north_srcaddr);
	itop->dut_north_dstaddr(dut_north_dstaddr);
	itop->dut_north_wr_wait(dut_north_wr_wait);
	itop->dut_north_rd_wait(dut_north_rd_wait);

	//SOUTH
	itop->ext_south_data(stim_south_data);
	itop->ext_south_dstaddr(stim_south_dstaddr);
	itop->ext_south_srcaddr(stim_south_srcaddr);
	itop->ext_south_write(stim_south_write);
	itop->ext_south_access(stim_south_access);
	itop->ext_south_datamode(stim_south_datamode);
	itop->ext_south_ctrlmode(stim_south_ctrlmode);
	itop->ext_south_wr_wait(stim_south_wr_wait);
	itop->ext_south_rd_wait(stim_south_rd_wait);

	itop->dut_south_write(dut_south_write);
	itop->dut_south_access(dut_south_access);
	itop->dut_south_datamode(dut_south_datamode);
	itop->dut_south_ctrlmode(dut_south_ctrlmode);
	itop->dut_south_data(dut_south_data);
	itop->dut_south_srcaddr(dut_south_srcaddr);
	itop->dut_south_dstaddr(dut_south_dstaddr);
	itop->dut_south_wr_wait(dut_south_wr_wait);
	itop->dut_south_rd_wait(dut_south_rd_wait);

	//EAST
	itop->ext_east_data(stim_east_data);
	itop->ext_east_dstaddr(stim_east_dstaddr);
	itop->ext_east_srcaddr(stim_east_srcaddr);
	itop->ext_east_write(stim_east_write);
	itop->ext_east_access(stim_east_access);
	itop->ext_east_datamode(stim_east_datamode);
	itop->ext_east_ctrlmode(stim_east_ctrlmode);
	itop->ext_east_wr_wait(stim_east_wr_wait);
	itop->ext_east_rd_wait(stim_east_rd_wait);

	itop->dut_east_write(dut_east_write);
	itop->dut_east_access(dut_east_access);
	itop->dut_east_datamode(dut_east_datamode);
	itop->dut_east_ctrlmode(dut_east_ctrlmode);
	itop->dut_east_data(dut_east_data);
	itop->dut_east_srcaddr(dut_east_srcaddr);
	itop->dut_east_dstaddr(dut_east_dstaddr);
	itop->dut_east_wr_wait(dut_east_wr_wait);
	itop->dut_east_rd_wait(dut_east_rd_wait);

	//WEST
	itop->ext_west_data(stim_west_data);
	itop->ext_west_dstaddr(stim_west_dstaddr);
	itop->ext_west_srcaddr(stim_west_srcaddr);
	itop->ext_west_write(stim_west_write);
	itop->ext_west_access(stim_west_access);
	itop->ext_west_datamode(stim_west_datamode);
	itop->ext_west_ctrlmode(stim_west_ctrlmode);
	itop->ext_west_wr_wait(stim_west_wr_wait);
	itop->ext_west_rd_wait(stim_west_rd_wait);

	itop->dut_west_write(dut_west_write);
	itop->dut_west_access(dut_west_access);
	itop->dut_west_datamode(dut_west_datamode);
	itop->dut_west_ctrlmode(dut_west_ctrlmode);
	itop->dut_west_data(dut_west_data);
	itop->dut_west_srcaddr(dut_west_srcaddr);
	itop->dut_west_dstaddr(dut_west_dstaddr);
	itop->dut_west_wr_wait(dut_west_wr_wait);
	itop->dut_west_rd_wait(dut_west_rd_wait);

	//SPI
	itop->ext_spi_data(stim_spi_data);
	itop->ext_spi_dstaddr(stim_spi_dstaddr);
	itop->ext_spi_srcaddr(stim_spi_srcaddr);
	itop->ext_spi_write(stim_spi_write);
	itop->ext_spi_access(stim_spi_access);
	itop->ext_spi_datamode(stim_spi_datamode);
	itop->ext_spi_ctrlmode(stim_spi_ctrlmode);
	itop->ext_spi_wr_wait(stim_spi_wr_wait);
	itop->ext_spi_rd_wait(stim_spi_rd_wait);

	itop->dut_spi_write(dut_spi_write);
	itop->dut_spi_access(dut_spi_access);
	itop->dut_spi_datamode(dut_spi_datamode);
	itop->dut_spi_ctrlmode(dut_spi_ctrlmode);
	itop->dut_spi_data(dut_spi_data);
	itop->dut_spi_srcaddr(dut_spi_srcaddr);
	itop->dut_spi_dstaddr(dut_spi_dstaddr);
	itop->dut_spi_wr_wait(dut_spi_wr_wait);
	itop->dut_spi_rd_wait(dut_spi_rd_wait);


	/**************************************************/
	/*NORTH STIMULUS CONNECTIONS                      */
	/**************************************************/

	//Stimulus Inputs
	istim_north->clk(cclk);

	//Control Outputs
	istim_north->stim_reset(stim_north_reset);         //resets core
	istim_north->stim_trace(stim_north_trace);         //controls trace dumping during simulation

	//Chip Interface
	istim_north->dut_wr_wait(dut_north_wr_wait);
	istim_north->dut_rd_wait(dut_north_rd_wait);
	istim_north->stim_data(stim_north_data);
	istim_north->stim_dstaddr(stim_north_dstaddr);
	istim_north->stim_srcaddr(stim_north_srcaddr);
	istim_north->stim_write(stim_north_write);
	istim_north->stim_access(stim_north_access);
	istim_north->stim_datamode(stim_north_datamode);
	istim_north->stim_ctrlmode(stim_north_ctrlmode);

	istim_north->dut_write(dut_north_write);
	istim_north->dut_access(dut_north_access);
	istim_north->dut_datamode(dut_north_datamode);
	istim_north->dut_ctrlmode(dut_north_ctrlmode);
	istim_north->dut_dstaddr(dut_north_dstaddr);
	istim_north->dut_data(dut_north_data);
	istim_north->dut_srcaddr(dut_north_srcaddr);
	istim_north->stim_wr_wait(stim_north_wr_wait);
	istim_north->stim_rd_wait(stim_north_rd_wait);


	/**************************************************/
	/*SOUTH STIMULUS CONNECTIONS                      */
	/**************************************************/

	//Stimulus Inputs
	istim_south->clk(cclk);

	//Control Outputs
	istim_south->stim_reset(stim_south_reset);         //resets core
	istim_south->stim_trace(stim_south_trace);         //controls trace dumping during simulation

	//Chip Interface
	istim_south->dut_wr_wait(dut_south_wr_wait);
	istim_south->dut_rd_wait(dut_south_rd_wait);
	istim_south->stim_data(stim_south_data);
	istim_south->stim_dstaddr(stim_south_dstaddr);
	istim_south->stim_srcaddr(stim_south_srcaddr);
	istim_south->stim_write(stim_south_write);
	istim_south->stim_access(stim_south_access);
	istim_south->stim_datamode(stim_south_datamode);
	istim_south->stim_ctrlmode(stim_south_ctrlmode);


	istim_south->dut_write(dut_south_write);
	istim_south->dut_access(dut_south_access);
	istim_south->dut_datamode(dut_south_datamode);
	istim_south->dut_ctrlmode(dut_south_ctrlmode);
	istim_south->dut_dstaddr(dut_south_dstaddr);
	istim_south->dut_data(dut_south_data);
	istim_south->dut_srcaddr(dut_south_srcaddr);
	istim_south->stim_wr_wait(stim_south_wr_wait);
	istim_south->stim_rd_wait(stim_south_rd_wait);

	/**************************************************/
	/*EAST STIMULUS CONNECTIONS                      */
	/**************************************************/

	//Stimulus Inputs
	istim_east->clk(cclk);

	//Control Outputs
	istim_east->stim_reset(stim_east_reset);         //resets core
	istim_east->stim_trace(stim_east_trace);         //controls trace dumping during simulation

	//Chip Interface
	istim_east->dut_wr_wait(dut_east_wr_wait);
	istim_east->dut_rd_wait(dut_east_rd_wait);
	istim_east->stim_data(stim_east_data);
	istim_east->stim_dstaddr(stim_east_dstaddr);
	istim_east->stim_srcaddr(stim_east_srcaddr);
	istim_east->stim_write(stim_east_write);
	istim_east->stim_access(stim_east_access);
	istim_east->stim_datamode(stim_east_datamode);
	istim_east->stim_ctrlmode(stim_east_ctrlmode);

	istim_east->dut_write(dut_east_write);
	istim_east->dut_access(dut_east_access);
	istim_east->dut_datamode(dut_east_datamode);
	istim_east->dut_ctrlmode(dut_east_ctrlmode);
	istim_east->dut_dstaddr(dut_east_dstaddr);
	istim_east->dut_data(dut_east_data);
	istim_east->dut_srcaddr(dut_east_srcaddr);
	istim_east->stim_wr_wait(stim_east_wr_wait);
	istim_east->stim_rd_wait(stim_east_rd_wait);


	/**************************************************/
	/*WEST STIMULUS CONNECTIONS                       */
	/**************************************************/

	//Stimulus Inputs(3)
	istim_west->clk(cclk);
	istim_west->stim_reset(stim_west_reset);         //resets core
	istim_west->stim_trace(stim_west_trace);         //controls trace dumping during simulation

	//Chip Interface(9)
	istim_west->dut_wr_wait(dut_west_wr_wait);
	istim_west->dut_rd_wait(dut_west_rd_wait);
	istim_west->stim_data(stim_west_data);
	istim_west->stim_dstaddr(stim_west_dstaddr);
	istim_west->stim_srcaddr(stim_west_srcaddr);
	istim_west->stim_write(stim_west_write);
	istim_west->stim_access(stim_west_access);
	istim_west->stim_datamode(stim_west_datamode);
	istim_west->stim_ctrlmode(stim_west_ctrlmode);

	istim_west->dut_write(dut_west_write);
	istim_west->dut_access(dut_west_access);
	istim_west->dut_datamode(dut_west_datamode);
	istim_west->dut_ctrlmode(dut_west_ctrlmode);
	istim_west->dut_dstaddr(dut_west_dstaddr);
	istim_west->dut_data(dut_west_data);
	istim_west->dut_srcaddr(dut_west_srcaddr);
	istim_west->stim_wr_wait(stim_west_wr_wait);
	istim_west->stim_rd_wait(stim_west_rd_wait);

	/**************************************************/
	/*SPI STIMULUS CONNECTIONS                        */
	/**************************************************/

	//Stimulus Inputs(3)
	istim_spi->clk(cclk);
	istim_spi->stim_reset(stim_spi_reset);         //resets core
	istim_spi->stim_trace(stim_spi_trace);         //controls trace dumping during simulation

	//Chip Interface(9)
	istim_spi->dut_wr_wait(dut_spi_wr_wait);
	istim_spi->dut_rd_wait(dut_spi_rd_wait);
	istim_spi->stim_data(stim_spi_data);
	istim_spi->stim_dstaddr(stim_spi_dstaddr);
	istim_spi->stim_srcaddr(stim_spi_srcaddr);
	istim_spi->stim_write(stim_spi_write);
	istim_spi->stim_access(stim_spi_access);
	istim_spi->stim_datamode(stim_spi_datamode);
	istim_spi->stim_ctrlmode(stim_spi_ctrlmode);

	istim_spi->dut_write(dut_spi_write);
	istim_spi->dut_access(dut_spi_access);
	istim_spi->dut_datamode(dut_spi_datamode);
	istim_spi->dut_ctrlmode(dut_spi_ctrlmode);
	istim_spi->dut_dstaddr(dut_spi_dstaddr);
	istim_spi->dut_data(dut_spi_data);
	istim_spi->dut_srcaddr(dut_spi_srcaddr);
	istim_spi->stim_wr_wait(stim_spi_wr_wait);
	istim_spi->stim_rd_wait(stim_spi_rd_wait);


	/**************************************************/
	/*MONITOR FOR STOPPING TEST                       */
	/**************************************************/
	if(!gdb_server) {
		imon->clk(cclk);
		imon->dut_passed(dut_passed);
		imon->dut_stopped(dut_stopped);
		imon->dut_failed(dut_failed);
	}


	if(batch_mode) {
		extern void RegisterBreakISR();
		RegisterBreakISR();
	}

	if( gdb_server && !batch_mode) {

		cout << "Warning: The current design supports maximum " <<  NCORES << " cores " << endl;


		TNamedPipeMemServer* fTNamedPipeMemServer =  new TNamedPipeMemServer("target_control_front_end");
		//TOOD check if port is exclusive
		if(spi_gdb_server) {
			if(NCORES < 4) {
				cerr << "ERROR : the SPI driver can be chosen only for 4 (or more) cores set up" << endl;
				exit(9);
			}

			fTNamedPipeMemServer->BindSpiStimulusDriver(istim_spi);
		}
		if(north_gdb_server) {
			fTNamedPipeMemServer->BindHostStimulusDriver(istim_north);
		}
		if(south_gdb_server) {
			fTNamedPipeMemServer->BindHostStimulusDriver(istim_south);
		}
		if(west_gdb_server) {
			fTNamedPipeMemServer->BindHostStimulusDriver(istim_west);
		}
		if(east_gdb_server) {
			fTNamedPipeMemServer->BindHostStimulusDriver(istim_east);
		}

		fTNamedPipeMemServer->clk(cclk);
	}


	/**************************************************/
	/*Initialization                                  */
	/**************************************************/
	//set chip coordinates;
	x_id.write(0x8/*GET_EXT_X_FROM_BASE_ADDR(CHIP_BASE)*/);
	y_id.write(0x8/*GET_EXT_Y_FROM_BASE_ADDR(CHIP_BASE)*/);

	//Load Stimulus from Files
	std::cout << "MSG>>LOADING STIMULUS FROM FILES(spi,north,south,east,west): " << spifile
			<< ", " << northfile
			<< ", " << southfile
			<< ", " << eastfile
			<< ", " << westfile
			<< std::endl;


	unsigned preconfig_size = load_stimulus(preconfigfile, preconfig_array);
	unsigned  north_asize = load_stimulus(northfile, north_array);
	unsigned south_asize = load_stimulus(southfile, south_array);
	unsigned east_asize  = load_stimulus(eastfile,  east_array);
	unsigned west_asize  = load_stimulus(westfile,  west_array);
	unsigned spi_asize   = load_stimulus(spifile,   spi_array);
	std::cout << "MSG>>FINISHED LOADING STIMULUS FROM FILE" << std::endl;

	//Initiliazing stimulus
	std::cout << "MSG>>INITIALIZING SIMULATOR" << std::endl;
	istim_north -> initialize(north_array, ptrace, preconfig_size);
	istim_south -> initialize(south_array, ptrace , preconfig_size);
	istim_east  -> initialize(east_array,  ptrace, preconfig_size);

	//prepend the preconfig_array to west_array
	west_array.insert(west_array.begin(), preconfig_array.begin(),preconfig_array.end());
	istim_west  -> initialize(west_array,  ptrace, 0/*the pre config will be driven from west*/);


	istim_spi   -> initialize(spi_array,  ptrace, preconfig_size);


	sc_signal<bool>        wait_for_hloadr_sync_north_out;
	sc_signal<bool>        wait_for_hloadr_sync_;

	///dummy signals just to terminated the output ports
	sc_signal<bool>        wait_for_hloadr_sync_1;
	sc_signal<bool>        wait_for_hloadr_sync_2;
	sc_signal<bool>        wait_for_hloadr_sync_3;
	sc_signal<bool>        wait_for_hloadr_sync_4;

	istim_north -> wait_for_hloadr_sync_in(wait_for_hloadr_sync_);
	istim_south -> wait_for_hloadr_sync_in(wait_for_hloadr_sync_north_out);
	istim_east  -> wait_for_hloadr_sync_in(wait_for_hloadr_sync_north_out);
	istim_west  -> wait_for_hloadr_sync_in(wait_for_hloadr_sync_north_out);
	istim_spi   -> wait_for_hloadr_sync_in(wait_for_hloadr_sync_north_out);

	istim_north -> wait_for_hloadr_sync_out(wait_for_hloadr_sync_north_out);
	istim_south -> wait_for_hloadr_sync_out(wait_for_hloadr_sync_1);
	istim_east  -> wait_for_hloadr_sync_out(wait_for_hloadr_sync_2);
	istim_west  -> wait_for_hloadr_sync_out(wait_for_hloadr_sync_3);
	istim_spi   -> wait_for_hloadr_sync_out(wait_for_hloadr_sync_4);

	wait_for_hloadr_sync_.write(false);
	wait_for_hloadr_sync_north_out.write(true);

	if(!gdb_server) {
		imon        -> initialize(cycles);

	}
	std::cout << "MSG>>FINISHED INITIALIZING SIMULATOR" << std::endl;

	/**************************************************/
	/*Waveform Dumping                                */
	/**************************************************/

	// Before any evaluation, need to know to calculate those signals only used for tracing

	traceFileCtl = new TraceSupport(vcd,vcdfile,itop);

	//gdb will do this by itself through tsstart/tsstop command
	if(!gdb_server || always_record_wave) {
		std::cerr << "MSG>>DRIVE open the .vcd file" << endl;
		traceFileCtl->StartRecord();
	}


	/**************************************************/
	/*Starting Clocks                                 */
	/**************************************************/
	//Starting simulation
	//Arbitrarily long time.  Should never get there.
	//Should be stopped by stimulus
	//Catch all for error
	//sc_start(1000000,SC_NS);
	sc_start();
	//Closing


	/**************************************************/
	/*Closing Waveform Dump                           */
	/**************************************************/

	if(!gdb_server  || always_record_wave) {
		std::cerr << "MSG>>DRIVE closing the .vcd file" << endl;
		traceFileCtl->CloseRecord();
	}


	return(0);
}


/**************************************************/
/*Stimulus Loading Routine                        */
/**************************************************/ 
int load_stimulus(char *filename, vector<StimulusEntry> &stimulus_array){

	//Local variables
	FILE *sourceFile;
	int  lineno=0;
	char line[MAXLINE];      //String for reading line


	sourceFile = fopen(filename, "r");

	if(sourceFile==NULL) {
		std::cout << "WARNING>>CAN'T ACCESS FILE! ("  << filename  << ") --- the stimulus file will be ignored" << std::endl;
		//sc_stop();
	}
	else{
		//Parsing through file one line at a time
		while (fgets(line, MAXLINE, sourceFile)!=NULL){


			StimulusEntry tmp_a;
			// sscanf(line,"%x_%x_%x_%x_%x",&array[lineno][0],&array[lineno][1],&array[lineno][2],&array[lineno][3],&array[lineno][4]);

			unsigned int numDelim=0;
			for(unsigned k = 0 ; k< MAXLINE; k++) {
				if(line[k]== '\n') break;
				if(line[k]== '_') numDelim++;
			}


			if(numDelim==4) {
				sscanf(line,"%x_%x_%x_%x_%x",&(tmp_a.srcAddr),&(tmp_a.data),&(tmp_a.dstAddr),&(tmp_a.ctrlMode),&(tmp_a.dataMode));
			} else if(numDelim==5) {
				sscanf(line,"%x_%x_%x_%x_%x_%x",&(tmp_a.srcAddr),&(tmp_a.data),&(tmp_a.dstAddr),&(tmp_a.ctrlMode),&(tmp_a.dataMode),&(tmp_a.delay));
			} else {
				cerr << "ERROR>> wrong format in data file" << endl;
			}


			stimulus_array.push_back(tmp_a);

#if DEBUG
			cout << "MSG>>FILE="  << filename;
			cout << " LINE="      << dec << lineno;
			cout << *(stimulus_array.rbegin()) ;//last
#endif
			lineno++;
		}

		fclose(sourceFile);
	}



	return lineno;
}

