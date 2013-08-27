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

#include <scv.h>
#include <vector>
#include <set>
#include <iterator>
//from igen
#include <misc.h>
#include <common.h>

#include "host_tran.h"

#include <stdio.h>
#include <stdlib.h>

#define NUMBER_HOSTACCESS_REGION 1


#define ATDSP_DEFAULT_MEM_SIZE 0x100000 /* 1M from  ./sim/atdsp/sim-main.h:*/
#define CHIP_ID_DEFAULT 0x24

extern void _scv_pop_constraint();
static unsigned int NCORES = 1;

static unsigned int CHIP_ID = CHIP_ID_DEFAULT;
#define MAX_NCORES 64
#define MAX_HOSTS 4

#define CORE_START_MEM 0x1000

#define CORE_END_MEM_DEFAULT (0x8000 - 1 )

#define CORE_SLICE_SIZE  0x800
#define HOST_SLICE_SIZE  (1<<15)//CORE_SLICE_SIZE*2

#define ILAT_RADDR 0xf0428
#define ILAT_CL_RADDR 0xf0430

#define MESH_MULTICAST_RADDR 0xf0708
#define MESH_CONFIG_RADDR 0xf0700

bool internal_only; //core runs  from internal memory only
bool host_off; //host space is not used , no access from host as well
bool chip_ext_off;// chip doesn't go to external  ( host access )
bool host_master_off;// chip doesn't go to external  ( host access )
bool mc_off; //multi cast option is disable

bool external_prod_mode = false;


bool host_north_off = false;
bool host_south_off = false;
bool host_west_off  = false;
bool host_east_off  = false;

unsigned host_offset_val_north = 1;
unsigned host_offset_val_south = 1;
unsigned host_offset_val_west = 1;
unsigned host_offset_val_east = 1;


unsigned core_mem_end = CORE_END_MEM_DEFAULT;


//since the compiled design has holes, need map count core to core num
unsigned fMapCoreCountToCoreNum[MAX_NCORES];

/* generate master for slice */
class TMasterId  :
public scv_constraint_base
{

public:


	scv_smart_ptr<bool>         fIsOffChipGen;
	scv_bag<bool> 				fIsOffChipDist;


public:
	void SetMasterId(unsigned coreId) {
		fMasterId.write(coreId);
	}
	unsigned GetMasterId() {
		unsigned msId = fMasterId.read();
		return msId;
	}

private:
	scv_smart_ptr<sc_uint<32> > fMasterId;
	scv_smart_ptr<sc_uint<32> > fNCores;


public:
	SCV_CONSTRAINT_CTOR(TMasterId) {

		fNCores->disable_randomization();
		fNCores.write(NCORES);


		SCV_CONSTRAINT( fMasterId() < fNCores());
		//	SCV_CONSTRAINT(  fIsOffChipGen() !=  true ||   fMasterId() < MAX_HOSTS);
		//	SCV_CONSTRAINT(  fIsOffChipGen() !=  true );


		if(host_off==true) {
			fIsOffChipGen->keep_only(false);
		} else {

			fIsOffChipDist.push(true,50);
			fIsOffChipDist.push(false,50);
			fIsOffChipGen->set_mode(fIsOffChipDist);

		}
	}
};





enum EHostsDirections {
	NORTH=0,
			SOUTH=1,
			WEST=2,
			EAST=3,
			SPI=4
};


class TCoreMemoryMap {
private:

	vector< pair<sc_uint<32>, sc_uint<32> >  > fCoresRangeList[MAX_NCORES];
	vector< pair<sc_uint<32>, sc_uint<32> >  > fHostsRangeList[MAX_HOSTS];
	sc_uint<32>  							   fHostsStartAddrList[MAX_HOSTS];

	string     									fHostNum2Name[MAX_HOSTS];
	/*
	 * <31:20> mc_id <19:0> slice start addr
	 * <31:20>  is initialized by mc_id equal to <x+1,y>  <x-1,y> , <x,y-1> or <x,y+1>
	 *  bit <19,0>  slide start address will serve as mc_id ,a re initialized to zero
	 */
	sc_uint<32>				fHostsMulticastAddrList[MAX_HOSTS];//
	/* host id, the host will be master for MC transaction
	 * initialized by  MAX_HOSTS , after that each host set id  (e.g.NORTH)
	 */
	sc_uint<32> 			  fCoreMultiCastHostId[MAX_NCORES];//

	scv_smart_ptr<bool>         fIsExternal;
	//access the  internal space like external
	scv_smart_ptr<bool>         fIsMapInt2Ext;
	scv_bag<bool> 				fIsMapInt2ExtDist;


	//designate all hosts transaction to the same core
	scv_smart_ptr<bool> fCongestionMode;
	scv_bag<bool> fCongestionModeDist;
	scv_smart_ptr<unsigned long>fCongestionCoreId;

	//generate the host id ( N , S, W ,E ) -- can be master or slave
	//allowExternalAccessFromChip.write(true);
	scv_smart_ptr<sc_uint<2> > hostGenId;
	scv_bag<sc_uint<2> > hostGenIdDist;


	scv_smart_ptr<unsigned>         fDelayForHostMasterTran;
	scv_bag<unsigned> 				fDelayForHostMasterTranDist;


	map<sc_uint<32> , sc_uint<8> > fExpData;
	set<sc_uint<32> > fExpReadAddr;

	//multicast expected result for destination host only
	map<sc_uint<32> , sc_uint<8> > fMCExpData[MAX_HOSTS];


	scv_smart_ptr<bool> isGenExtendeHostSpace;


	THostBurstGen *fhostTranBurst;


	//"message" ISR
	scv_smart_ptr<bool>         fMessageMode;
	scv_bag<bool> 				fMessageModeDist;

public:
	TMasterId *ms;
public:
	TCoreMemoryMap() {


		ms = new TMasterId("TMasterId");
		fhostTranBurst = new THostBurstGen("Constrained tran");

		sc_uint<32> host_offset;
		//set x
		host_offset(25,23)= sc_uint<32>(CHIP_ID)(2,0);
		//set y
		host_offset(31,29) =  sc_uint<32>(CHIP_ID)(5,3);
		cerr << hex << "chip_full_offset 0x" << 	host_offset << dec << endl;


		for(unsigned j=0;j<NCORES;j++) {
			//cerr << "["<< j << hex << "]: 0x" << GetAddrFromCoreCount(j)	 << dec << endl;

		}

		//the chip can not be in the corner
		assert(sc_uint<32>(CHIP_ID)(2,0) != 0 && sc_uint<32>(CHIP_ID)(2,0) != 0x7 &&
				sc_uint<32>(CHIP_ID)(5,3) != 0 && sc_uint<32>(CHIP_ID)(5,3) != 0x7);

		if(mc_off!=true) {
			assert(sc_uint<32>(CHIP_ID)(2,0) != 1 && sc_uint<32>(CHIP_ID)(2,0) != 0x6 &&
					sc_uint<32>(CHIP_ID)(5,3) != 1 && sc_uint<32>(CHIP_ID)(5,3) != 0x6);
		}

		for(unsigned int k = 0 ; k < MAX_HOSTS; k++) {
			fHostsStartAddrList[k]=0;
			fHostsMulticastAddrList[k]=0;
		}
		for(unsigned int k = 0 ; k < NCORES; k++) {
			fCoreMultiCastHostId[k]=MAX_HOSTS;
		}
		//north
		fHostsStartAddrList[NORTH](25,23)=   sc_uint<32>(CHIP_ID)(2,0);//x
		fHostsStartAddrList[NORTH](31,29) =  sc_uint<32>(CHIP_ID)(5,3) - host_offset_val_north ;//y

		fHostsMulticastAddrList[NORTH](25,23)=   sc_uint<32>(CHIP_ID)(2,0);//x
		fHostsMulticastAddrList[NORTH](31,29) =  sc_uint<32>(CHIP_ID)(5,3) - 2 ;//y

		fHostNum2Name[NORTH]="north";

		//south
		fHostsStartAddrList[SOUTH](25,23)=   sc_uint<32>(CHIP_ID)(2,0);//x
		fHostsStartAddrList[SOUTH](31,29) =  sc_uint<32>(CHIP_ID)(5,3) + host_offset_val_south ;//y

		fHostsMulticastAddrList[SOUTH](25,23)=   sc_uint<32>(CHIP_ID)(2,0);//x
		fHostsMulticastAddrList[SOUTH](31,29) =  sc_uint<32>(CHIP_ID)(5,3) + 2 ;//y

		fHostNum2Name[SOUTH]="south";

		//west
		fHostsStartAddrList[WEST](25,23)=   sc_uint<32>(CHIP_ID)(2,0) - host_offset_val_west ;//x
		fHostsStartAddrList[WEST](31,29) =  sc_uint<32>(CHIP_ID)(5,3);//y

		fHostsMulticastAddrList[WEST](25,23)=   sc_uint<32>(CHIP_ID)(2,0) - 2 ;//x
		fHostsMulticastAddrList[WEST](31,29) =  sc_uint<32>(CHIP_ID)(5,3);//y

		fHostNum2Name[WEST]="west";

		//east
		fHostsStartAddrList[EAST](25,23)=   sc_uint<32>(CHIP_ID)(2,0) + host_offset_val_east ;//x
		fHostsStartAddrList[EAST](31,29) =  sc_uint<32>(CHIP_ID)(5,3);//y

		if(external_prod_mode) {
			fHostsStartAddrList[EAST](25,23)=   0;
			fHostsStartAddrList[EAST](31,29) =  8;
		}


		fHostsMulticastAddrList[EAST](25,23)=   sc_uint<32>(CHIP_ID)(2,0) + 2 ;//x
		fHostsMulticastAddrList[EAST](31,29) =  sc_uint<32>(CHIP_ID)(5,3);//y

		fHostNum2Name[EAST]="east";

		for(unsigned int k = 0 ; k < MAX_HOSTS; k++) {
			cerr << hex << " host_offset "<< fHostNum2Name[k] <<  " 0x" << fHostsStartAddrList[k] << dec << endl;
		}




		fCongestionModeDist.push(true,1);
		fCongestionModeDist.push(false,4);
		fCongestionMode->set_mode(fCongestionModeDist);

		//congestionHostMode->keep_only(true);
		if(internal_only == false ) {
			fCongestionMode->next();
		} else {
			fCongestionMode.write(false);
		}

		fCongestionCoreId->keep_only(0,NCORES-1);
		fCongestionCoreId->next();

		if(fCongestionMode.read() == true) {
			cerr << " congestion core ID : " << fCongestionCoreId.read() << endl;
		}


		fIsMapInt2ExtDist.push(true,1);
		fIsMapInt2ExtDist.push(false,7);
		fIsMapInt2Ext->set_mode(fIsMapInt2ExtDist);



		fDelayForHostMasterTranDist.add(0,80);
		fDelayForHostMasterTranDist.add(1,7);
		fDelayForHostMasterTranDist.add(2,3);
		fDelayForHostMasterTranDist.add(5,2);
		fDelayForHostMasterTranDist.add(6,2);
		fDelayForHostMasterTranDist.add(7,2);
		fDelayForHostMasterTranDist.add(8,2);
		fDelayForHostMasterTranDist.add(25,1);

		fDelayForHostMasterTran->set_mode(fDelayForHostMasterTranDist);


		//hostGenId->keep_only(0,MAX_HOSTS-1);


		// make some host to drive more transactions, used to rise probability to have the same slave multi core slice
		scv_smart_ptr<sc_uint<2> > drive_more_host;
		assert(MAX_HOSTS>0);

		//drive_more_host->keep_only(0,MAX_HOSTS-1);


		drive_more_host->next();
		for(unsigned j= 0; j <MAX_HOSTS ; j++) {
			if(j==drive_more_host.read()) {
				hostGenIdDist.add(j,60);
			} else {
				hostGenIdDist.add(j,20);
			}
		}


		if(host_north_off || host_south_off || host_west_off || host_east_off) {
			if(host_north_off) {
				hostGenId->keep_out(NORTH);
			}

			if(host_south_off) {
				hostGenId->keep_out(SOUTH);
			}

			if(host_west_off) {
				hostGenId->keep_out(WEST);
			}

			if(host_east_off) {
				hostGenId->keep_out(EAST);
			}
		} else {
			hostGenId->set_mode(hostGenIdDist);//hostGenId->keep_only(drive_more_host);
		}

		//if(NCORES>2) {


		//} else {
		//	 hostGenId->keep_only(WEST);
		//}





		//have the all hosts space available for transactions
		isGenExtendeHostSpace->next();

		if(external_prod_mode) {
			isGenExtendeHostSpace.write(false);
		}
		//isGenExtendeHostSpace.write(true);

		//generate "message" write transaction

		fMessageModeDist.add(true,20);
		fMessageModeDist.add(false,80);
		fMessageMode->set_mode(fMessageModeDist);


	}
	sc_uint<32> GetAddrFromCoreCount(unsigned nc_count) {

		unsigned ncore = fMapCoreCountToCoreNum[nc_count];

		sc_uint<32> addr=0;

		//host space
		addr(25,23)=   sc_uint<32>(CHIP_ID)(2,0);
		addr(31,29) =  sc_uint<32>(CHIP_ID)(5,3);

		addr(22,20)= sc_uint<32>(ncore)(2,0);//x
		addr(28,26)= sc_uint<32>(ncore)(5,3);//y

		return addr;

	}


	void GenerateMap() {


		cerr << hex << "setting chip id 0x" << CHIP_ID<< dec << endl;

		sc_uint<32> core_offset;
		for ( unsigned nc_count = 0 ; nc_count <  NCORES; nc_count++ ) {

			unsigned ncore = fMapCoreCountToCoreNum[nc_count];

			core_offset(22,20)= sc_uint<32>(ncore)(2,0);//x
			core_offset(28,26)= sc_uint<32>(ncore)(5,3);//y
			//cerr << dec << ncore << " 0x" << hex <<  core_offset << endl;
		}


		//generate mem for cores memory
		for ( unsigned nc_count = 0 ; nc_count <  NCORES; nc_count++ ) {

			unsigned ncore = fMapCoreCountToCoreNum[nc_count];

			if(NCORES ==1 ||  NCORES ==2) {
				mc_off=true;
			}

			sc_uint<32> coreMemAddrForPrint=0;
			coreMemAddrForPrint(25,23)= sc_uint<32>(CHIP_ID)(2,0);//x
			coreMemAddrForPrint(31,29)= sc_uint<32>(CHIP_ID)(5,3);//y

			coreMemAddrForPrint(22,20)= sc_uint<32>(ncore)(2,0);//x
			coreMemAddrForPrint(28,26)= sc_uint<32>(ncore)(5,3);//y

			cerr << "==============["<< nc_count << "]Distribute address space for core % " <<  ncore << hex << " " << coreMemAddrForPrint <<dec << endl;

			for( unsigned i = CORE_START_MEM ; i < core_mem_end; i = i +  CORE_SLICE_SIZE) {

				if( internal_only == true   ) { //each core run in own space only
					fIsExternal->keep_only(false);

				}

				fIsExternal->next();

				if(fCongestionMode.read()== true) {

					if(	nc_count != fCongestionCoreId.read()) {
						fIsExternal.write(false);


					} else {
						fIsExternal.write(true);
					}
				}

				if(fIsExternal.read() ) {
					ms->next();	//go for external
				} else {
					ms->SetMasterId(nc_count);// internal memory
					ms->fIsOffChipGen.write(false);
				}
				//cerr << ncore << "--- address 0x" << hex << i << dec << "  : " << ( ms->fIsOffChipGen.read() ? "H" : "C" ) << ms->fMasterId.read() << endl;

				pair<sc_uint<32>, sc_uint<32> > range;// = pair<sc_uint<32>, sc_uint<32> >(i,i+SLICE_SIZE-1);
				sc_uint<32> full_address_in_core = i;


				if(internal_only || ( nc_count == ms->GetMasterId() && !ms->fIsOffChipGen.read() )    ) {
					//internal access

					fIsMapInt2Ext->next();
					//remap internal to external , e.g 0 -->90000000
					if(fIsMapInt2Ext.read() && !internal_only) {
						full_address_in_core(25,23)= sc_uint<32>(CHIP_ID)(2,0);//x
						full_address_in_core(31,29)= sc_uint<32>(CHIP_ID)(5,3);//y

						full_address_in_core(22,20)= sc_uint<32>(ncore)(2,0);//x
						full_address_in_core(28,26)= sc_uint<32>(ncore)(5,3);//y
					}


				} else {
					full_address_in_core(25,23)= sc_uint<32>(CHIP_ID)(2,0);//x
					full_address_in_core(31,29)= sc_uint<32>(CHIP_ID)(5,3);//y

					full_address_in_core(22,20)= sc_uint<32>(ncore)(2,0);//x
					full_address_in_core(28,26)= sc_uint<32>(ncore)(5,3);//y
				}

				range  = pair<sc_uint<32>, sc_uint<32> >(full_address_in_core,full_address_in_core+CORE_SLICE_SIZE-1);

				if((!ms->fIsOffChipGen.read()) || internal_only || (chip_ext_off && host_master_off )  ) { // core access
					//core access
					if(!external_prod_mode) {
						fCoresRangeList[ms->GetMasterId()].push_back(range);
						cerr<<  dec  << "Core "<< ms->GetMasterId() <<  hex  << " " << range << endl;
					} else {
						fCoresRangeList[ncore].push_back(range);
						cerr<<  dec  << "Core "<< ncore <<  hex  << " " << range << endl;
					}

				}  else { // host access - master or slave

					scv_smart_ptr<bool>  allowExternalAccessFromChip;
					allowExternalAccessFromChip->next();

					if(chip_ext_off) {
						allowExternalAccessFromChip.write(false);
					}
					if(host_master_off) {
						allowExternalAccessFromChip.write(true);
					}

					hostGenId->next();
					if(NCORES<=2) {
						if(!external_prod_mode) {

							hostGenId.write(WEST);

							assert(hostGenId.read()==WEST);
						}
					} else {
						assert(hostGenId.read()<min((unsigned int)MAX_HOSTS,NCORES));
					}
					//						assert(hostSlaveId.read()==0);
					if(allowExternalAccessFromChip.read() ) {

						//host is slave
						//reallocate to host

						if(!external_prod_mode) {
							range.first(31,29) = fHostsStartAddrList[hostGenId.read()](31,29);
							range.first(25,23) = fHostsStartAddrList[hostGenId.read()](25,23);

							range.second(31,29) = fHostsStartAddrList[hostGenId.read()](31,29);
							range.second(25,23) = fHostsStartAddrList[hostGenId.read()](25,23);
						} else {

							assert(0);

							//PROG FIXME
							sc_uint<2> x_tmp=range.first(27,26);

							range.first(27,26) =0;
							range.second(27,26) =0;


							range.first(31,28) = 8;
							range.first(25,22) = 0;

							range.second(31,28) = 8;
							range.second(25,22) = 0;


							range.first(23,22) = x_tmp;
							range.second(23,22) = x_tmp;
						}

						fCoresRangeList[nc_count].push_back(range);

						cerr << "---Reuse for Core "<< nc_count<< hex <<  range << dec << endl;

					} else {

						//host master

						scv_smart_ptr<bool>  isTheFollowingSliceMC;
						if(mc_off == true ) {
							isTheFollowingSliceMC->keep_only(false);
						}
						isTheFollowingSliceMC->next();
						if(isTheFollowingSliceMC.read() == true)  {
							if(fCoreMultiCastHostId[nc_count] == MAX_HOSTS) {//core slave is not allocated to mc host
								sc_uint<32> mc_addr = fHostsMulticastAddrList[hostGenId.read()];
								if(
										mc_addr(19,0) == 0 /* the mc slice is not generated yet */ ||
										mc_addr(19,0) == (range.first)(19,0)/* the slice has the same mc range, can be added to mc  */
								) {
									//allocated slice mc group for the host
									fHostsMulticastAddrList[hostGenId.read()](19,0) = (range.first)(19,0);

									//core slave
									fCoreMultiCastHostId[nc_count]=hostGenId.read();

									//cerr << "MC --->" << hostGenId.read() << endl;
								}
							}

						}

						fHostsRangeList[hostGenId.read()].push_back(range);
						cerr << dec << "Host["<< hostGenId.read() <<"]" <<  hex  << " " << range << endl;
					}
				}
			}
		}



		if((isGenExtendeHostSpace.read() == true) && !internal_only && NCORES>2 && !chip_ext_off && !host_off) {
			for(unsigned h_id = 0 ; h_id < MAX_HOSTS ;h_id++ ) {
				cerr << "==============[0]Distribute remaining address space for Host %" <<  h_id << endl;
				//0 - already allocated
				for(unsigned int i = 1; i < 16; i++) {//number slices in extended host range

					if(i == 0x1e) {
						continue;
					}

					sc_uint<32> full_address_in_host=0;
					//ext
					full_address_in_host(25,23) = fHostsStartAddrList[h_id](25,23);
					full_address_in_host(31,29) = fHostsStartAddrList[h_id](31,29);
					//internal (x,y) to have all transactions unique
					full_address_in_host(22,20) = h_id;
					full_address_in_host(28,26) = h_id;

					//extended range --- not allocated previously
					full_address_in_host(19,15) = i;

					assert(full_address_in_host(19,15) != 0x1e);

					pair<sc_uint<32>, sc_uint<32> >  range  =
							pair<sc_uint<32>, sc_uint<32> >(full_address_in_host,full_address_in_host+HOST_SLICE_SIZE-1);


					scv_smart_ptr<bool> iSExtendedRangeAllocatedForCore;
					if(host_master_off) {
						iSExtendedRangeAllocatedForCore->keep_only(true);
					}
					iSExtendedRangeAllocatedForCore->next();
					if(iSExtendedRangeAllocatedForCore.read() == true) {//go for core
						scv_smart_ptr<unsigned> coreM;
						coreM->keep_only(0,NCORES-1);
						coreM->next();

						fCoresRangeList[coreM.read()].push_back(range);
						cerr<<  dec  << "Core "<< coreM.read() <<  hex  << " " << range << endl;

					} else {//go for host
						scv_smart_ptr<unsigned> hostM;
						hostM->keep_out(h_id);
						hostM->keep_only(0,MAX_HOSTS-1);
						hostM->next();

						fHostsRangeList[hostM.read()].push_back(range);
						cerr << dec << "Host["<< hostM.read() <<"]" <<  hex  << " " << range << endl;
					}

				}
			}
		}


		for(unsigned h_id = 0 ; h_id < MAX_HOSTS ;h_id++ ) {
			cerr << dec << " MC master ranges Host["<< h_id<<"]" <<  hex  << " <" << fHostsMulticastAddrList[h_id](31,20) << "," <<  fHostsMulticastAddrList[h_id](19,0) <<  "> for host " << fHostsStartAddrList[h_id] << dec  << endl;

		}
		for ( unsigned nc_count = 0 ; nc_count <  NCORES; nc_count++ ) {
			if(fCoreMultiCastHostId[nc_count]<MAX_HOSTS) { //Allocated
				cerr << dec << " MC core slave  " <<   nc_count << " ,host id : "  << fCoreMultiCastHostId[nc_count]<< dec  << endl;
			}
		}

		ofstream host_data_ilat("host_data_ilat.txt");

		for ( unsigned nc_count = 0 ; nc_count <  NCORES; nc_count++ ) {

			unsigned ncore = fMapCoreCountToCoreNum[nc_count];

			//Drive ILAT
			host_data_ilat << "00000000_00000001_";
			sc_uint<32> ilatStart= ILAT_RADDR;
			ilatStart(31,29)= sc_uint<32>(CHIP_ID)(5,3);//y
			ilatStart(25,23)= sc_uint<32>(CHIP_ID)(2,0);//x

			ilatStart(28,26)= sc_uint<32>(ncore)(5,3);//y
			ilatStart(22,20)= sc_uint<32>(ncore)(2,0);//x


			host_data_ilat << hex << long(ilatStart);
			host_data_ilat <<		"_0_9" <<endl;
		}

		host_data_ilat.close();



		//in case of mc write to core need to read data from core back for further comparison
		ofstream mc_signoff_tran[MAX_HOSTS];

		mc_signoff_tran[NORTH].open("north_mc_signoff_tran.txt",ios_base::out | ios_base::trunc);
		mc_signoff_tran[SOUTH].open("south_mc_signoff_tran.txt",ios_base::out | ios_base::trunc);
		mc_signoff_tran[WEST].open("west_mc_signoff_tran.txt",ios_base::out | ios_base::trunc);
		mc_signoff_tran[EAST].open("east_mc_signoff_tran.txt",ios_base::out | ios_base::trunc);

		ofstream mc_data_out_expected("mc_data_out_expected.txt",ios_base::out | ios_base::trunc);


		//for all hosts
		for(unsigned hostIdNum = 0 ; hostIdNum < MAX_HOSTS; hostIdNum++) {
			if(host_master_off) {
				break;
			}

			ofstream host_data_host_random;

			switch (hostIdNum) {
			case NORTH:
				host_data_host_random.open("host_data_random_north.txt",ios_base::out | ios_base::trunc);
				break;

			case SOUTH:
				host_data_host_random.open("host_data_random_south.txt",ios_base::out | ios_base::trunc);
				break;

			case WEST:
				host_data_host_random.open("host_data_random_west.txt",ios_base::out | ios_base::trunc);
				break;

			case EAST:
				host_data_host_random.open("host_data_random_east.txt",ios_base::out | ios_base::trunc);
				break;


			};

			//program core slaves for multi cast
			for ( unsigned nc_count = 0 ; nc_count <  NCORES; nc_count++ ) {
				if(fCoreMultiCastHostId[nc_count] == hostIdNum) {
					sc_uint<64> data=0;
					data(63,32) = fHostsStartAddrList[hostIdNum];//src tran
					data(11,0) =  fHostsMulticastAddrList[hostIdNum](31,20);
					//Address of the multicast configuration register (12 bits) used for address comparison (bits 31:20) instead of core's coordinates:
					PackTransactionToFile( host_data_host_random, data(63,32)/*srdAddr*/ ,
							GetAddrFromCoreCount(nc_count) + MESH_MULTICAST_RADDR,
							data,true, 0 /*ctrlMode*/,WORD_A , 0/*delay*/ );
				}
			}


			//generate host random accesses
			for ( unsigned k = 0 ; k < NUMBER_HOSTACCESS_REGION*fHostsRangeList[hostIdNum].size(); k++) {
				//

				//host_data_host_random << endl;

				unsigned r_ind = rand() % fHostsRangeList[hostIdNum].size();
				assert(r_ind<fHostsRangeList[hostIdNum].size());
				sc_uint <32> sliceStart = fHostsRangeList[hostIdNum].operator [](r_ind).first;



				//generate is burst Write or Read, while some transactions in the burst can be not the same as burst type !!!
				scv_smart_ptr<bool > isWrite;
				scv_bag<bool > isWriteDist;

				isWriteDist.add(true,66);
				isWriteDist.add(false,33);
				isWrite->set_mode(isWriteDist);

				isWrite->next();

//				isWrite.write(true);//FIXME

				bool isMultiCast=false;
				if(isWrite.read() == true) {//this is a write

					//can be multicast
					if(fHostsMulticastAddrList[hostIdNum](19,0) ==  sliceStart(19,0)) {
						//the slice should be multi cast
						isMultiCast=true;
					}

				}


				fhostTranBurst->ctrlMode.write(REGULAR_CTRL_MODE);
				if(isMultiCast) {

					fhostTranBurst->ctrlMode.write(MULTI_CAST_CTRL_MODE);
				}

				fhostTranBurst->addr_min.write(sliceStart);
				fhostTranBurst->addr_max.write( fHostsRangeList[hostIdNum].operator [](r_ind).second);

				fhostTranBurst->next();


				sc_uint<2>  dataSize =fhostTranBurst->dataSize.read();

				unsigned burst_length = (fhostTranBurst->length.read()) >> dataSize;


				//host_data_host_random << " ------- " << burst_length << " dsize "<< dataSize <<endl;


				//generate burst
				for(unsigned i = 0  ; i <  burst_length; i++) {

					//the burst will mix read and write
					if(fhostTranBurst->ctrlMode.read() !=MULTI_CAST_CTRL_MODE) {
						if(!fhostTranBurst->isWriteOrReadBurst.read() ) {
							isWrite->next();
						}
					}


					sc_uint<32> dstAddrFromH =(( fhostTranBurst->addr.read() >> dataSize) << dataSize);

					if(!fhostTranBurst->same_addr_burst.read()) {
						dstAddrFromH+= i * (1<<dataSize);
					}

					//check if  tran in allowed range
					assert(dstAddrFromH < fHostsRangeList[hostIdNum].operator [](r_ind).second && dstAddrFromH >=fHostsRangeList[hostIdNum].operator [](r_ind).first);

					//cout << ( isWrite.read() ? "W" : "R")  << " " << hex << dstAddrFromH<< dec << endl;

					sc_uint<32> srcAddrHost = dstAddrFromH;
					srcAddrHost(31,29)=fHostsStartAddrList[hostIdNum](31,29);
					srcAddrHost(25,23)=fHostsStartAddrList[hostIdNum](25,23);

					scv_smart_ptr<sc_uint<64> > data;
					data->next();

					fDelayForHostMasterTran->next();


					if(fhostTranBurst->ctrlMode.read() == MULTI_CAST_CTRL_MODE) {
						//replace the destination address id by 11 bits multicast address for destination address
						dstAddrFromH(31,20) = fHostsMulticastAddrList[hostIdNum](31,20);

						assert(  isWrite.read() ==true && dataSize != DOUBLE_A);
					}

					unsigned CtrlMode = fhostTranBurst->ctrlMode.read();
					//no multicast, write and designated to core


					if(CtrlMode != MULTI_CAST_CTRL_MODE && isWrite.read() == true &&
							dstAddrFromH(31,29) ==  sc_uint<32>(CHIP_ID)(5,3)
							&& dstAddrFromH(25,23) ==  sc_uint<32>(CHIP_ID)(2,0)) {


						fMessageMode.write(false);
						if(NCORES<=2) {
							if(hostIdNum == WEST) {
								fMessageMode->next();
							}

						}
						if(NCORES==4) {
							/*north -->  00
							 * south --> 01
							 * west -->  10
							 * east -->  11
							 */
							if(
									(hostIdNum == NORTH) && ( dstAddrFromH(28,26) == 0) && ( dstAddrFromH(22,20) == 0) ||
									(hostIdNum == SOUTH) && ( dstAddrFromH(28,26) == 0) && ( dstAddrFromH(22,20) == 1) ||
									(hostIdNum == WEST) && ( dstAddrFromH(28,26) == 1) && ( dstAddrFromH(22,20) == 0) ||
									(hostIdNum == EAST) && ( dstAddrFromH(28,26) == 1) && ( dstAddrFromH(22,20) == 1)
							) {
								fMessageMode->next();
							}


						}

						//TODO
						fMessageMode.write(false);
						if(fMessageMode.read() == true) {
							CtrlMode=MODIFY_READ_MODE;//change to message mode
						}

					}


					//print out transaction
					PackTransactionToFile( host_data_host_random,srcAddrHost,dstAddrFromH,data.read(),isWrite.read(),
							CtrlMode,	dataSize , fDelayForHostMasterTran.read() );


					//generate expected result for regular

					if(fhostTranBurst->ctrlMode.read()==REGULAR_CTRL_MODE) {
						if(isWrite.read()) {

							for ( unsigned i = 0 ; i < (1 << (dataSize)) ; i++) {

								fExpData[dstAddrFromH+i] = 	(data.read())( (i+1)*8-1,i*8  );

								// in end of test every write became read CHIP(31,26) ->> HOST(31,26)
								fExpData[srcAddrHost+i]= (data.read())( (i+1)*8-1,i*8  );

							}
						} else {
							//read
							for ( unsigned i = 0 ; i < (1 << (dataSize)) ; i++) {
								fExpReadAddr.insert(srcAddrHost+i);
								if(fExpData.find(dstAddrFromH+i) != fExpData.end()) {
									fExpData[srcAddrHost+i] = fExpData[dstAddrFromH+i] ;

								}
							}
						}
					}// END REGULAR_CTRL_MODE


					//generate expected result for multicast and build singoff
					if(fhostTranBurst->ctrlMode.read()==MULTI_CAST_CTRL_MODE) {
						assert(isWrite.read()==true);//should be write

						//go for all hosts

						for(unsigned mcTragetHostId = 0 ; mcTragetHostId < MAX_HOSTS; mcTragetHostId++) {
							extern bool isHostMCTarget(sc_uint<12> , sc_uint<12> , sc_uint<12> );
							if(isHostMCTarget(dstAddrFromH(31,20),srcAddrHost(31,20),fHostsStartAddrList[mcTragetHostId](31,20)) ) {
								//format : <full mc dst address as seen in host> <11 bits of target host> <data>
								mc_data_out_expected << hex << dstAddrFromH << " " <<
										fHostsStartAddrList[mcTragetHostId](31,20) << " " << data.read()((8<<(dataSize))-1,0) << " " << srcAddrHost(31,20) << dec << endl;

							}

						}

						// go for all cores cores
						for ( unsigned nc_count = 0 ; nc_count <  NCORES; nc_count++ ) {
							if(fCoreMultiCastHostId[nc_count] == hostIdNum) {

								sc_uint<32> dstAddrFromH_mc=dstAddrFromH;
								sc_uint<32> srcAddrHost_mc=srcAddrHost;

								sc_uint<32> coreDstBase = GetAddrFromCoreCount(nc_count);
								//write core corresponding multi core slave
								dstAddrFromH_mc(31,20) = coreDstBase(31,20);

								//same as core
								srcAddrHost_mc(28,26) =  dstAddrFromH(28,26);
								srcAddrHost_mc(22,20) =  dstAddrFromH(22,20);

								//sign transaction
								PackTransactionToFile(mc_signoff_tran[hostIdNum] ,srcAddrHost_mc,dstAddrFromH_mc,data.read(),false/*read*/,
										REGULAR_CTRL_MODE	,	dataSize , 0/*delay*/ );


								//set expected data to core
								for ( unsigned i = 0 ; i < (1 << (dataSize)) ; i++) {

									fExpData[dstAddrFromH_mc+i] = 	(data.read())( (i+1)*8-1,i*8  );

									//sign off will make all write to read in the end of test
									fExpData[srcAddrHost_mc+i] = (data.read())( (i+1)*8-1,i*8  );
								}


							}
						}

					}//END MULTI_CAST_CTRL_MODE


				}

			}
			host_data_host_random.close();
		}


		for(unsigned hostIdNum = 0 ; hostIdNum < MAX_HOSTS; hostIdNum++) {

			mc_signoff_tran[hostIdNum].close();

		}
		mc_data_out_expected.close();





		ofstream genMem;
		//print out for generator
		for ( unsigned nc_count = 0 ; nc_count <  NCORES; nc_count++ ) {

			unsigned ncore = fMapCoreCountToCoreNum[nc_count];

			char fileName[100];
			sprintf(fileName,"mem_ranges_%d.txt",ncore);
			genMem.open(fileName);
			if(genMem.bad() ) {
				cerr << "ERROR , can't open file" << endl;
				exit(8);
			}

			//igenMem << "Core " << j << endl;
			genMem << hex << showbase;

			for(unsigned i = 0; i < fCoresRangeList[nc_count].size() ; i++ ) {

				genMem << (fCoresRangeList[nc_count])[i] << endl;

			}

			genMem.close();
		}



		//print out for simulator
		for ( unsigned nc_count = 0 ; nc_count <  NCORES; nc_count++ ) {

			unsigned ncore = fMapCoreCountToCoreNum[nc_count];

			char fileName[100];
			sprintf(fileName,"mem_sim_%d.txt",ncore);
			genMem.open(fileName);
			if(genMem.bad() ) {
				cerr << "ERROR , can't open file" << endl;
				exit(8);
			}


			genMem << hex << showbase;
			for(unsigned i = 0; i < fCoresRangeList[nc_count].size() ; i++ ) {
				if((fCoresRangeList[nc_count])[i].first >=  ATDSP_DEFAULT_MEM_SIZE) {
					genMem  << " --memory-region " << (fCoresRangeList[nc_count])[i].first << ","<< ((fCoresRangeList[nc_count])[i].second ) - ( (fCoresRangeList[nc_count])[i].first ) ;
				}
			}

			genMem.close();
		}


		//expected result



		ofstream hostExpectedData("host_expected_data_out.txt");
		for( map<sc_uint<32> , sc_uint<8> >::iterator it =   fExpData.begin(); it !=  fExpData.end(); it++ ) {
			if( (   fHostsStartAddrList[NORTH](31,29) == it->first(31,29) &&  fHostsStartAddrList[NORTH](25,23) == it->first(25,23) ) ||
					(	fHostsStartAddrList[SOUTH](31,29) == it->first(31,29) && fHostsStartAddrList[SOUTH](25,23) == it->first(25,23) ) ||
					(	fHostsStartAddrList[WEST](31,29) == it->first(31,29)  && fHostsStartAddrList[WEST](25,23) == it->first(25,23)  ) ||
					(	fHostsStartAddrList[EAST](31,29) == it->first(31,29)  && fHostsStartAddrList[EAST](25,23) == it->first(25,23)  )
			) {

				//only address in the host
				hostExpectedData<< hex << it->first << " " << it->second << dec << endl;
			}
		}

		hostExpectedData.close();


		ofstream host_address_read("host_address_read.txt");
		host_address_read << hex;
		copy(fExpReadAddr.begin(), fExpReadAddr.end(), ostream_iterator<sc_uint<32> >(host_address_read, "\n"));
		host_address_read.close();

	}


	void PackTransactionToFile(ofstream& host_data_host_random, sc_uint<32> srcAddrHost, sc_uint<32> dstAddrH,
			sc_uint<64> data, bool isWrite, sc_uint<4> controlMode,sc_uint<2>  dataSize , unsigned delay ) {

		/*
		 * Note: Format is <srcaddr>_<data>_<dstaddr>_<ctrlmode>_<type>
 Leave the ctrlmode as 0 for now.  The type is compried of
b3:2=datamode, b1=read,b0=write.
Datamode is, 11=64bit,10=32bit,01=16bit,00=byte transaction
		 */


		if(!isWrite || controlMode == MULTI_CAST_CTRL_MODE) {
			host_data_host_random <<  hex << long( srcAddrHost ) ;

			//reset the data for read
			if(!isWrite ) {
				data(31,0)=0;
			}
		} else {
			//host_data_host_random <<  "00000000" ;
			host_data_host_random.width(8);
			host_data_host_random.fill('0');
			host_data_host_random  << hex << right <<  long(data(63,32) );

		}
		host_data_host_random << "_";

		host_data_host_random.width(8);
		host_data_host_random.fill('0');
		host_data_host_random  << hex << right <<  long(data(31,0) );


		host_data_host_random << "_";


		host_data_host_random.width(8);
		host_data_host_random.fill('0');
		host_data_host_random  << hex << right <<  long(dstAddrH );

		//host_data_host_random.width(1);
		//host_data_host_random.fill('0');
		host_data_host_random << "_" << controlMode << "_";


		host_data_host_random <<  long(((dataSize) << 2) + ( isWrite ? 1 : 2  ));

		host_data_host_random << "_" << hex << delay << dec;

		host_data_host_random << endl;
	}
};

void usage() {
	cerr << "Usage " << "mgen   -seed <seed> -c <num cores> [  -host_off ] [ -internal_only] [ -chip_ext_off] [-core_mem_end_address <hex> ]"   << endl;
}



int sc_main (int argc, char** argv)
{
	unsigned long long seed=0;
	internal_only = false;
	host_off = false;
	chip_ext_off=false;
	host_master_off=false;
	mc_off=false;

	//init core num to real core num map
	for(unsigned c = 0 ; c <  MAX_NCORES ; c++ ) {
		fMapCoreCountToCoreNum[c]=c;
	}


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
		//host_offset_val

		if(!strcmp(argv[n],"-host_offset_val_north")) {
			n+=1;
			if(n< argc) {
				host_offset_val_north = (std::atoi(argv[n]));

			} else {
				usage();
				return 3;
			}
			continue;
		}
		if(!strcmp(argv[n],"-host_offset_val_south")) {
			n+=1;
			if(n< argc) {
				host_offset_val_south = (std::atoi(argv[n]));

			} else {
				usage();
				return 3;
			}
			continue;
		}
		if(!strcmp(argv[n],"-host_offset_val_west")) {
			n+=1;
			if(n< argc) {
				host_offset_val_west = (std::atoi(argv[n]));

			} else {
				usage();
				return 3;
			}
			continue;
		}
		if(!strcmp(argv[n],"-host_offset_val_east")) {
			n+=1;
			if(n< argc) {
				host_offset_val_east = (std::atoi(argv[n]));

			} else {
				usage();
				return 3;
			}
			continue;
		}


		if(!strcmp(argv[n],"-c")) {
			n+=1;
			if(n< argc) {
				NCORES = (std::atoi(argv[n]));

			} else {
				usage();
				return 3;
			}
			continue;
		}
		if(!strcmp(argv[n],"-internal_only")) {

			internal_only = true;
			continue;
		}
		if(!strcmp(argv[n],"-host_off")) {

			host_off = true;
			continue;
		}
		if(!strcmp(argv[n],"-chip_ext_off")) {

			chip_ext_off = true;
			continue;
		}
		if(!strcmp(argv[n],"-mc_off")) {


			mc_off = true;
			continue;
		}
		if(!strcmp(argv[n],"-host_master_off")) {

			host_master_off = true;
			continue;
		}


		if(!strcmp(argv[n],"-host_north_off")) {

			host_north_off = true;
			continue;
		}

		if(!strcmp(argv[n],"-host_south_off")) {

			host_south_off = true;
			continue;
		}
		if(!strcmp(argv[n],"-host_west_off")) {

			host_west_off = true;
			continue;
		}
		if(!strcmp(argv[n],"-host_east_off")) {

			host_east_off = true;
			continue;
		}
		if(!strcmp(argv[n],"-external_prod_mode")) {

			assert(0);
			external_prod_mode = true;
			continue;
		}

		if(!strcmp(argv[n],"-id")) {
			n+=1;
			if(n< argc) {
				CHIP_ID = (std::strtol(argv[n], NULL, 16));

			} else {
				usage();
				return 3;
			}
			continue;
		}

		if(!strcmp(argv[n],"-core_mem_end_address")) {
			n+=1;
			if(n< argc) {

				core_mem_end = (std::strtol(argv[n], NULL, 16));

				cerr << "setting the internal memory range {"<< hex << "0x"<< CORE_START_MEM << " ... 0x" << core_mem_end << dec <<  "}" <<endl;

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

				cerr << "core_0 "  << fMapCoreCountToCoreNum[0] << endl;

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

				cerr << "core_1 "  << fMapCoreCountToCoreNum[1] << endl;

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

				cerr << "core_2 "  << fMapCoreCountToCoreNum[2] << endl;

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

				cerr << "core_3 "  << fMapCoreCountToCoreNum[3] << endl;

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

	//for 4x4 when use only 2x2
//	if(NCORES == 4) {
//		fMapCoreCountToCoreNum[2]=8;
//		fMapCoreCountToCoreNum[3]=9;
//	}




	scv_random::set_global_seed(seed);
	cerr << "Setting seed " << seed <<endl;

	if(mc_off != true) {
		//don't use the random chip id
		CHIP_ID=CHIP_ID_DEFAULT;
	}


	TCoreMemoryMap c;
	c.GenerateMap();

	return 0;
}


/* get mc is address 12 bits and driving host 12 bits is and check if multicast transaction will reach the host */
bool isHostMCTarget(sc_uint<12> mcId, sc_uint<12> drivingHostId, sc_uint<12> dstTargetId) {


	//TODO coordinate is calculated for 6 most significant bits
	sc_uint<3> x_mc_id = mcId(5,3);
	sc_uint<3> y_mc_id = mcId(11,9);


	sc_uint<3> x_drivingHost = drivingHostId(5,3);
	sc_uint<3> y_drivingHost = drivingHostId(11,9);


	sc_uint<3> x_dstTargetId = dstTargetId(5,3);
	sc_uint<3> y_dstTargetId = dstTargetId(11,9);

	return
	//from north to south
	(y_mc_id < y_drivingHost && y_dstTargetId > y_drivingHost && x_dstTargetId == x_drivingHost) ||
	//from sourth to north
	(y_mc_id > y_drivingHost && y_dstTargetId < y_drivingHost && x_dstTargetId == x_drivingHost) ||
	//from east to north-west or south-west or west
	(x_mc_id < x_drivingHost && x_dstTargetId  > x_drivingHost) ||
	//from west to  north-east or south-east or east
	(x_mc_id > x_drivingHost && x_dstTargetId  < x_drivingHost);

}
