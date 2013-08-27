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

#ifndef DMA_H
#define DMA_H


extern bool isGenInter;

/*
 DESCRIPTOR PLACEMENT IN MEMORY
 * (note that it takes 3 clock cycles to fetch a new descriptor)
 *
 *config
 *stride
 *count
 *stride_ext
 *src
 *dst
 *autodma0
 *autodma1
 *status
 */

#define DMA0_CONFIG   0x000F0500
#define DMA0_STRIDE   0x000F0504
#define DMA0_COUNT    0x000F0508
#define DMA0_SRCADDR  0x000F050C
#define DMA0_DSTADDR  0x000F0510
#define DMA0_AUTODMA0 0x000F0514
#define DMA0_AUTODMA1 0x000F0518
#define DMA0_STATUS   0x000F051C

#define DMA1_CONFIG   0x000F0520
#define DMA1_STRIDE   0x000F0524
#define DMA1_COUNT    0x000F0528
#define DMA1_SRCADDR  0x000F052C
#define DMA1_DSTADDR  0x000F0530
#define DMA1_AUTODMA0 0x000F0534
#define DMA1_AUTODMA1 0x000F0538
#define DMA1_STATUS   0x000F053C



#define MAX_ALLOWED_DMA_TRAN_SIZE_INGEN 1024

#define MAX_CHAIN_ADDRESS (1<<16)
#include <algorithm>


bool CompareRangeSize( pair<sc_uint<32> , sc_uint<32> > i, pair<sc_uint<32> , sc_uint<32> > j) {
	return ( i.second - i.first ) < ( j.second - j.first );
}

bool CompareRangeAddr( pair<sc_uint<32> , sc_uint<32> > i, pair<sc_uint<32> , sc_uint<32> > j) {
	return (  i.first ) < (  j.first );
}

struct TDecriptor {
	sc_uint<32>  fSrdAddr;
	sc_int<16> fStrideIn;
	sc_int<16> fStrideOut;

	sc_uint<32>  fDstAddr;
	ELS_AccessSize fAccessSize;
	sc_uint<16> fCountTranInner;
	sc_uint<16> fCountTranOuter;
	bool fIsISREn;

	bool fIsChained;
	bool fIsStartDMA;
	sc_uint<32> fChainPointer;//actually the chain pointer is 16 bits, used for ISR
	unsigned fMaxSizeInBytes;
	sc_uint<32> fDescLocation;
	sc_uint<32>  fISRAddress;
	sc_uint<32> fNextISRAddr;//to load the next IST address to IVT
	sc_uint<1>   fDMAnum;
	bool isAutoDMA;

	scv_smart_ptr<bool> fModifySign;

	sc_uint<2> fDMARoutCntrl;//TODO

	sc_uint<1> fDMAMessageMode;

	sc_uint<4> fStrideShift;//TODO

	//reg used for MOVTS operation
	unsigned fMOVTSreg;

	static const unsigned  NUM_FIELDS_IN_DESCRIPTOR = 6;//config,stride,count,stride_ext,addr src and dst
	static const unsigned  NUM_REGS_TO_INIT_TCB = 5;//config,stride,count,addr src and dst
	unsigned DecriptionSectionSize() {
		if(fDescLocation == DMA0_CONFIG || fDescLocation == DMA1_CONFIG) {
			return  NUM_REGS_TO_INIT_TCB*4/*bytes*/;
		} else {
			return  NUM_FIELDS_IN_DESCRIPTOR*4/*bytes*/;
		}

	}

	sc_uint<32> GetDecriptionSectionStartAddr() {
		return fDescLocation;
	}
	sc_uint<32> GetDecriptionSectionEndAddr() {
		return  fDescLocation+DecriptionSectionSize();
	}

	TDecriptor() {
		fIsChained=false;
		fChainPointer=0;
		fIsISREn=false;
		fISRAddress=0;
		fNextISRAddr=0;
		fIsStartDMA=false;

		fDMAMessageMode=0;


		fStrideOut=0x0badbeef;

		fDMARoutCntrl=0;
		fStrideShift=0;
		fCountTranOuter=0;

		fModifySign->next();
	}


	void GetDMAConfig(unsigned& addr, sc_uint<32>& data) {

		data=0;

		addr=fDescLocation;

		/* Enable DMA 0
		 *
		 */
		data(0,0) = 1;
		if( !isAutoDMA) 		data(1,1) = 1;//master/slave mode
		else data(1,1) = 0;

		/* 1
		 * Chain enable. If set, at the end of transaction, you go to get another descriptor
		 */
		data(2,2) = (fIsChained)?1:0;


		/* start mode - 3
		 *  To launch the DMA transaction
		 */
		data(3,3) = (fIsStartDMA)?1:0;


		/*  DMA_IRQEN - 4
		 */
		data(4,4) = (fIsISREn)?1:0;



		/* DMA_DATASIZE 6.5
		 *        (data transaction size to generate)
		 *         00=byte,01=short,10=word,11=double)
		 */
		//size_format
		sc_uint<2> s_format;
		//check alignment
		switch ( fAccessSize ) {
		case BYTE_A:
			s_format = 0x0;
			break;
		case HALF_A:
			s_format = 0x1;
			break;
		case WORD_A:
			s_format = 0x2;
			break;
		case DOUBLE_A:
			s_format = 0x3;
			break;
		}
		data(6,5) = s_format;//transaction data-size,00=byte,01=short,10=word,11=double

		/*7     = RESERVED
		 **/
		data(7,7) = 0;


		/*9:8   = DMA_ROUTE-CTRL (RESERVED)
		 *        (controls routing algorithm for the mesh
		 *         not yet implemented)
		 */

		data(9,8) = fDMARoutCntrl;


		/*bit [10] in the dma-config register to indicate that we want to enable message mode*/
		if(fCountTranInner!= 0) {
			data(10,10) = fDMAMessageMode;
		}

		/*11:10 = RESERVED
		 */
		data(11,11) = 0;

		/*15:12 = DMA_STRIDESHIFT
		 *        (enables shift of 16 bit stride by 16 bits
		 *         [12],if 1, shift inner loop source stride << 16
		 *         [13],if 1, shift inner loop dest stride << 16
		 *         [14],if 1, shift outer loop source stride << 16
		 *         [15],if 1, shift outer loop dest stride << 16
		 *        )
		 */
		data(15,12) = fStrideShift;

		data(31,16) = fChainPointer(15,0);/*[31:16]=next pointer(for chaining) */

	}
	//DMA_COUNT
	void GetCount(unsigned& addr, sc_uint<32>& data) {
		addr=fDescLocation + 8;

		data(15,0)=fCountTranInner;
		data(31,16) = fCountTranOuter;
	}

	//DMA0_STRIDE
	void GetStrideIn(unsigned& addr, sc_uint<32>& data) {
		addr=fDescLocation + 4;
		data(15,0)= fStrideIn;
		data(31,16)=fStrideIn;

	}
	//DMA0_STRIDE
	void GetStrideOut(unsigned& addr, sc_uint<32>& data) {
		addr=fDescLocation + 0xc;
		data(15,0)=fStrideOut;
		data(31,16)=fStrideOut;
	}
	//DMA0_SRCADDR
	void GetSrcAddr(unsigned& addr, sc_uint<32>& data) {
		addr=fDescLocation + 0x10;
		data=fSrdAddr;
	}

	//DMA0_DSTADDR
	void GetDstAddr(unsigned& addr, sc_uint<32>& data) {
		addr=fDescLocation + 0x14;
		data=fDstAddr;
	}

	void StartFromDescriptor(ostream& s) {

		//need to write to DMA config
		unsigned d_addr;
		sc_uint<32> d_data;

		/*
		 * [5:0]-->(0..max)
	[11:6]-->(0,1)     larger values will blow up the run time

	Also, please refrain from generating a non-zero value for 5:0 and a
		zero value for 11:6.
		 */
		scv_smart_ptr<sc_uint<6> > dma_throttle_5_0;
		unsigned dma_throttle_max_5_0 = 25;
		dma_throttle_5_0->keep_only(0,dma_throttle_max_5_0);
		dma_throttle_5_0->next();


		scv_smart_ptr<sc_uint<6> > dma_throttle_11_6;
		dma_throttle_11_6->keep_only(0,1);
		if(dma_throttle_5_0.read()  != 0 ) {
			dma_throttle_11_6->keep_out(0);
		}
		dma_throttle_11_6->next();

		/*
		 *
	Then bits [15:4] have a special meaning and get written to the dma_throttle field.

	[9:4]=number of dma transactions to do before going into sleep mode
	[15:10]=number of clocks cycles (multipled by 16) to sleep before being active again.
		 */


		d_data(9,4) = dma_throttle_5_0.read();
		d_data(15,10) = dma_throttle_11_6.read();
		
		//start DMA
		d_data(3,3) = 1;
		//take the TCB from the memory location
		d_data(31,16) = fChainPointer;

		s <<dec  << "MOV R0" <<  ",%low(0x" << hex << d_data  << ");" << dec << endl;
		s <<dec  << "MOVT R0"   << ",%high(0x" << hex << d_data << ");"<< dec << endl;

		s<< "MOVTS dma" <<  fDMAnum << "config" << ", R0" << endl;
	}

	void PrintDMATCB(ostream& s,  TDecriptor& nextTCB) {
		s << "/*" << endl;
		if (isAutoDMA) {
			s << "DMA slave( source address will be ignored) ";
		}
		else {
			s << "DMA master "<< hex << fSrdAddr ;
		}

		s  << hex << "-->" << fDstAddr << dec <<endl;

		s << fDMAnum << " "<< "TCB pointer " <<hex << fDescLocation << dec << endl;
		s << " next DMA loc**" << hex << fChainPointer<< dec << endl;
		if(fIsChained) s << "CHAIN >>>"<< hex << fChainPointer<< dec << endl;
		if(fIsChained) {
			assert( fChainPointer(31,16) == 0);//only internal
		}

		if(fDMAMessageMode && (fCountTranInner!= 0)) s << "[10] bit is ON" << endl;
		if(fIsISREn) s << "INTr >>>"<< hex << fISRAddress<< dec <<endl;
		if(fIsISREn) s << "next DMA IVT >>>"<< hex << fNextISRAddr << dec <<endl;
		s << "Count Inner Loop " << hex << fCountTranInner << " ";
		s << "Count Outer Loop " << hex << fCountTranOuter <<endl;
		s << "StrideIn " << hex << fStrideIn << " ";
		s << "StrideOut " << hex << fStrideOut << " ";
		s << "Modify sign ";
		if(fModifySign.read()) s << "+";
		else s << "-";
		s << "MAX BYTES SIZE in region "<< dec << fMaxSizeInBytes <<endl;
		//count
		switch ( fAccessSize ) {
		case BYTE_A:
			s << "BYTE_A" << endl;
			break;
		case HALF_A:
			s << "HALF_A" << endl;
			break;
		case WORD_A:
			s << "WORD_A" << endl;
			break;
		case DOUBLE_A:
			s << "DOUBLE_A" << endl;
			break;
		}
		s << "*/" << endl;

		if(fIsISREn) {
			s << ".section L_" << hex <<  fISRAddress<< dec << ",\"a\",@progbits;" << endl;
			s<< ".L_" <<   hex << fISRAddress << dec << ":"<< endl;

			// if no valid chain -- need to reprogram DMA
			if(!fIsChained) {
				unsigned addr;
				sc_uint<32> data;

				s << " //prepare chain the next chain @address: " << hex << fChainPointer << dec <<endl;

				StartFromDescriptor(s);

			}


			if(fNextISRAddr != fISRAddress) {
				s << "//set DMA next IVT " << endl;
				s << "//load address of the dma " << endl;

				s <<dec  << "MOV R0" <<  ",%low(.L_" << hex <<   fNextISRAddr << ");" << dec << endl;
				s <<dec  << "MOVT R0"   << ",%high(.L_" << hex << fNextISRAddr   << ");"<< dec << endl;

				s << "//r0 >> next DMA ISR address" << endl;
				s << "str R0,[r" << fMOVTSreg << ",2+( (DMA_INTERRUPT_BUFFER"<< fDMAnum <<"-ISR_SAVE_RESTORE_SPACE)>>2 )];" <<endl;

				s << "//end of ivt setting" << endl;
			}


			s << "//restore reg 0 " << endl;
			s << "ldr R0,[r" << fMOVTSreg << ",1+( (DMA_INTERRUPT_BUFFER"<<fDMAnum <<"-ISR_SAVE_RESTORE_SPACE)>>2 )];" <<endl;

			s << "RTI;/*end of 0x"<< hex << fISRAddress << dec  << " */" << endl;

		}

		s << endl;
	}
};


class TDmaEngine {
protected:
	TMemManager *fMemManager;
	TRegManager *fRegsManager;

	vector<TDecriptor> fDescrList;

	//used to generate the value
	scv_smart_ptr<ELS_AccessSize> fAccessSize;
	scv_smart_ptr<bool> fDirOut;

	scv_smart_ptr<bool> fIsChain;
	scv_smart_ptr<bool> fIsInterrupt;
	scv_bag<bool> I_C_Dist;



	unsigned fDMAEngineNumber;





public:
	TDmaEngine(unsigned num) {
		//choose the DMA engine number
		fDMAEngineNumber=num;

		I_C_Dist.add(true,66);
		I_C_Dist.add(false,33);
		fIsChain->set_mode(I_C_Dist);
		//fIsInterrupt->set_mode(I_C_Dist);
	}


	unsigned GetNumTCBs() {
		return fDescrList.size();
	}

	void BindMemManager(TMemManager *MemManager) {fMemManager=MemManager;}
	void BindRegsManager(TRegManager *RegsManager) {fRegsManager=RegsManager;}

	virtual void PrintMe(::std::ostream& s= ::std::cout ) {

		s << ".section L_DMA_INIT_CONFIG"<< fDMAEngineNumber << ",\"a\",@progbits     ;take care of fisrt transfer if any" << endl;
		s << ".L_DMA_INIT_CONFIG" << fDMAEngineNumber << ":" << endl;

		if(! fDescrList.empty()) {
			TDecriptor dummyDscr;
			dummyDscr.fChainPointer = fDescrList[0].fDescLocation;			
			dummyDscr.fDMAnum=fDMAEngineNumber;

			dummyDscr.StartFromDescriptor(s);

		}
		s << "RTS" << endl;

		unsigned n = fDMAEngineNumber;

		s << ".section L_DMA_GLOBAL"<< n <<",\"a\",@progbits     ;" <<endl;


		s << ".global "<< ".L_DMA_GLOBAL" << n << ";" << endl;
		s << ".L_DMA_GLOBAL" << n << ":"<<  endl;

		s << "//store reg 0 " << endl;
		s << "str R0,[r" << fRegsManager->GetISRContextSaveRegInd() << ",1+( (DMA_INTERRUPT_BUFFER"<<fDMAEngineNumber <<"-ISR_SAVE_RESTORE_SPACE)>>2 )];" <<endl;
		//		s << "GIEN" << endl;//TODO

		//load to R0 address of  DMA ISR
		s << "ldr R0,[r" << fRegsManager->GetISRContextSaveRegInd() << ",2+( (DMA_INTERRUPT_BUFFER"<<fDMAEngineNumber<<"-ISR_SAVE_RESTORE_SPACE)>>2 )];" <<endl;
		s << "JR R0" << endl; // jump to ISR



		bool isr_dma_set = false;

		for(vector<TDecriptor>::iterator it = fDescrList.begin() ; it != fDescrList.end(); it++) {

			if(it->fIsISREn && !isr_dma_set) {
				s << ".set ISR_DMA"<< n << ", .L_"<< hex  << it->fISRAddress << dec << endl;
				isr_dma_set=true;
			}

			TDecriptor nextD = *it;
			if(it+1 !=fDescrList.end() ) {
				nextD = *(it+1);
			}
			it->PrintDMATCB(s,nextD);

		}

		//no ISR in DMA
		s << "//no DMA ISR setting to dummy" << endl;
		if(! isr_dma_set) s << ".set ISR_DMA" << n << ", L_DMA_GLOBAL" << n << endl;


	}


	void GenerateDescrList(vector<TSubRoutine *>& currSubRList/*to ISR*/) {



		vector< pair<sc_uint<32> , sc_uint<32> > > unLIst;
		unLIst.insert (unLIst.end(),fMemManager->fRangeListForFetch.begin() , fMemManager->fRangeListForFetch.end() );
		unLIst.insert (unLIst.end(),fMemManager->fRangeListForLS.begin() , fMemManager->fRangeListForLS.end() );

		fMemManager->fRangeListForLS.erase(fMemManager->fRangeListForLS.begin() , fMemManager->fRangeListForLS.end() );
		fMemManager->fRangeListForFetch.erase(fMemManager->fRangeListForFetch.begin() , fMemManager->fRangeListForFetch.end() );


		for( vector< pair<sc_uint<32> , sc_uint<32> > >::iterator it = unLIst.begin() ; it  != unLIst.end() ; it++ ) {
			if((it-unLIst.begin()) %2) {
				fMemManager->fRangeListForLS.push_back(*it);
			} else {
				fMemManager->fRangeListForFetch.push_back(*it);
			}
		}

		//cerr << " UNIFY LIST " << unLIst.size() << endl;
		//fMemManager->PrintRange(unLIst, cerr);

		//sort the lists according to size
		sort(fMemManager->fRangeListForLS.begin() , fMemManager->fRangeListForLS.end() , CompareRangeSize);
		cerr << "DMA fRangeListForLS" << endl;
		fMemManager->PrintRange(fMemManager->fRangeListForLS, cerr);

		//sort the lists according to start
		sort(fMemManager->fRangeListForFetch.begin() , fMemManager->fRangeListForFetch.end() , CompareRangeAddr);
		cerr << "DMA fRangeListForFetch" << endl;
		fMemManager->PrintRange(fMemManager->fRangeListForFetch, cerr);



		pair<sc_uint<32> , sc_uint<32> > int_range,ext_range,ext_range_2;

		if(!(!fMemManager->fRangeListForLS.empty() && !fMemManager->fRangeListForFetch.empty())) {
			cerr << "[--DMA] no ANY SPACE  found" << endl;
		}

		bool endOfTCBgen = false;
		unsigned u=0;
		while (!endOfTCBgen && !fMemManager->fRangeListForLS.empty() && !fMemManager->fRangeListForFetch.empty()) {

			bool foundDMArangeInt=false;
			bool foundDMArangeExt=false;
			bool foundDMArangeExt_2=false;

			cerr << "Allocation for DMA: " <<  u++ << endl;
			fMemManager->DumpInitialRangesList(cerr);

			for(vector< pair<sc_uint<32> , sc_uint<32> > > ::iterator it = fMemManager->fRangeListForLS.begin() ; it !=  fMemManager->fRangeListForLS.end() ; it++ ) {
				if( it->first < EXT_MEM_START) {
					int_range = *it;
					fMemManager->fRangeListForLS.erase(it);
					foundDMArangeInt=true;
					break;
				}
			}
			for(vector< pair<sc_uint<32> , sc_uint<32> > > ::iterator it = fMemManager->fRangeListForLS.begin() ; it !=  fMemManager->fRangeListForLS.end() ; it++ ) {
				if( it->first >= EXT_MEM_START) {
					ext_range = *it;
					fMemManager->fRangeListForLS.erase(it);
					foundDMArangeExt=true;
					break;
				}
			}

			//ext2ext is allowed as well
			if(!foundDMArangeInt && foundDMArangeExt) {
				for(vector< pair<sc_uint<32> , sc_uint<32> > > ::iterator it = fMemManager->fRangeListForLS.begin() ; it !=  fMemManager->fRangeListForLS.end() ; it++ ) {
					if( it->first >= EXT_MEM_START && ( ( it->first(31,29) != ext_range.first (31,29)) || ( it->first(25,23) != ext_range.first (25,23))) /*not same host or chip*/) {
						ext_range_2 = *it;
						fMemManager->fRangeListForLS.erase(it);
						foundDMArangeExt_2=true;
						break;
					}
				}
			}

			if(!((foundDMArangeInt&& foundDMArangeExt) || (foundDMArangeExt&&foundDMArangeExt_2)) ) {

				cerr << "[--DMA] no proper range  found %%fRangeListForLS " << fMemManager->fRangeListForLS.size() << " %%fRangeListForFetch " << fMemManager->fRangeListForFetch.size()<<  endl;
			}

			if((foundDMArangeInt&& foundDMArangeExt) || (foundDMArangeExt&&foundDMArangeExt_2)) {

				pair<sc_uint<32> , sc_uint<32> > range_1,range_2;


				if(foundDMArangeInt&& foundDMArangeExt) {
					range_1=int_range;
					range_2=ext_range;

				} else {
					if(foundDMArangeExt&&foundDMArangeExt_2) {
						range_1=ext_range_2;
						range_2=ext_range;
					} else {
						assert(0); //should be found
					}
				}


				cerr << hex  << "DMA 1--- --" << range_1 << "size" << range_1.second-range_1.first<< dec << endl;
				cerr << hex << "DMA 2--- --" <<  range_2 << "size" << range_2.second-range_2.first<< dec << endl;

				TDecriptor tmpDescr;

				tmpDescr.fDMAnum = fDMAEngineNumber;
				tmpDescr.fMOVTSreg = fRegsManager->GetISRContextSaveRegInd();


				//choose direction
				fDirOut->next();//relevant only for in<->ext

				if( fDirOut.read() ) { // int ->> ext
					tmpDescr.fSrdAddr = range_1.first;
					tmpDescr.fDstAddr = range_2.first;
				} else { //ext --> int
					tmpDescr.fSrdAddr = range_2.first;
					tmpDescr.fDstAddr = range_1.first;
				}
				assert(range_2.second >= range_2.first);
				assert(range_1.second >= range_1.first);


				scv_smart_ptr<bool> fIsAutoDMA;
				//choose Auto DMA
				if(fDirOut.read() || !foundDMArangeInt || !isAutoDMAOn) {
					fIsAutoDMA.write(false);
				} else {

					fIsAutoDMA->next();

					//fIsAutoDMA.write(true);///////////////////////////////////////
				}
				if(this->fDMAEngineNumber==1) {
					fIsAutoDMA.write(false);//TODO TODO
				}
				//fIsAutoDMA.write(false);//TODO TODO

				tmpDescr.isAutoDMA = fIsAutoDMA.read();


				scv_smart_ptr<bool> IsDMAMessageMode;
				if(fDirOut.read() || !foundDMArangeInt || fIsAutoDMA.read()) {
					//the message mode is relevant for transfer with destination of internal memory
					IsDMAMessageMode->keep_only(false);
				}
				IsDMAMessageMode->next();
				tmpDescr.fDMAMessageMode=IsDMAMessageMode.read();


				//reuse
				if(tmpDescr.fDMAMessageMode) {
					if( fDMAEngineNumber == 0 ) {
						//can't reuse (DMA 1 can overwrite the mem ranges !!!) : allocated region will be removed from generation
						if(tmpDescr.isAutoDMA) {
							if(fDirOut.read() ) {
								fMemManager->fRangeListForLS.push_back(range_1);
							} else {
								fMemManager->fRangeListForLS.push_back(range_2);
							}
						}
					} else {
						//DMA 1 can reuse the LS mem ranges
						fMemManager->fRangeListForLS.push_back(range_2);
						fMemManager->fRangeListForLS.push_back(range_1);
					}
				}

				fAccessSize->next();

				tmpDescr.fAccessSize = fAccessSize.read();

				//check alignment
				switch ( tmpDescr.fAccessSize ) {
				case BYTE_A:

					break;
				case HALF_A:
					tmpDescr.fSrdAddr=(((tmpDescr.fSrdAddr)>>1)<<1)+2;
					tmpDescr.fDstAddr=(((tmpDescr.fDstAddr)>>1)<<1)+2;
					tmpDescr.fMaxSizeInBytes = ((tmpDescr.fMaxSizeInBytes >> 1) << 1);
					break;
				case WORD_A:
					tmpDescr.fSrdAddr=(((tmpDescr.fSrdAddr)>>2)<<2)+4;
					tmpDescr.fDstAddr=(((tmpDescr.fDstAddr)>>2)<<2)+4;
					tmpDescr.fMaxSizeInBytes = ((tmpDescr.fMaxSizeInBytes >> 2) << 2);

					break;
				case DOUBLE_A:
					tmpDescr.fSrdAddr=(((tmpDescr.fSrdAddr)>>3)<<3)+8;
					tmpDescr.fDstAddr=(((tmpDescr.fDstAddr)>>3)<<3)+8;
					tmpDescr.fMaxSizeInBytes = ((tmpDescr.fMaxSizeInBytes >> 3) << 3);
					break;
				}

				cerr << hex << " tmpDescr.fSrdAddr " << tmpDescr.fSrdAddr << " tmpDescr.fDstAddr " << tmpDescr.fDstAddr << dec << endl;


				if (((range_2.second - range_2.first) >=8) && ((range_1.second - range_1.first) >=8)) {
					tmpDescr.fMaxSizeInBytes = (min(  (range_2.second - range_2.first) , (range_1.second - range_1.first)  ))  - 8 /*transaction can 8 bytes */;

					if((tmpDescr.fMaxSizeInBytes)>MAX_ALLOWED_DMA_TRAN_SIZE_INGEN) {
						tmpDescr.fMaxSizeInBytes=MAX_ALLOWED_DMA_TRAN_SIZE_INGEN;
					}

				} else {
					tmpDescr.fMaxSizeInBytes = 0;
					assert(0);// ????
				}

				//find region to store DMA reg
				if((fMemManager->fRangeListForFetch.empty()) || (fMemManager->fRangeListForFetch.empty()) ) {
					cerr << "[--DMA] no more space  ( location) : the channel will be not allocated" << endl;
					endOfTCBgen=true;
					// the DMA will be not generated
				} else {


					vector< pair<sc_uint<32> , sc_uint<32> > > ::iterator it_f = fMemManager->fRangeListForFetch.begin();
					tmpDescr.fDescLocation = MAX_CHAIN_ADDRESS;
					for(it_f = fMemManager->fRangeListForFetch.begin() ; it_f !=  fMemManager->fRangeListForFetch.end() ; it_f++ ) {
						if( it_f->first < MAX_CHAIN_ADDRESS ) {
							tmpDescr.fDescLocation = (((it_f->first)>>3)<<3)+8;
							fMemManager->fRangeListForFetch.erase(it_f);
							break;
						}
					}

					//set up chain for prev
					cerr << "DMA --- location " << hex << (tmpDescr).fDescLocation  << dec<<endl;
					//fill chain filed

					//the dec location should be in the internal space
					if(tmpDescr.fDescLocation >= MAX_CHAIN_ADDRESS) {
						endOfTCBgen=true;
						cerr << "[--DMA] tmpDescr.fDescLocation >= MAX_CHAIN_ADDRESS" << endl;
					}

					if(! fDescrList.empty() && tmpDescr.fDescLocation < MAX_CHAIN_ADDRESS) {
						//always assign the prev DMA TCB

						if(!isGenInter && fDMAEngineNumber==1) {//Maximize DMA gen
							fIsChain.write(true);
						} else {
							fIsChain->next();
						}

						fDescrList.rbegin()->fIsChained = fIsChain.read();


						//fill interrupt
						if(fMemManager->fRangeListForFetch.empty() ) {
							fIsInterrupt.write(false);

							cerr << "[--DMA] (fRangeListForFetch.empty())DMA no more space  : LAST TCB" << endl;

						} else {

							if(isGenInter && !fIsChain.read() /*chain and ISR mutually exclusive */) {
								if(fDMAEngineNumber==0) {
									fIsInterrupt->next();
								} else {
									fIsInterrupt.write(true);
								}
							} else {
								fIsInterrupt.write(false);
							}

							cerr << " fIsInterrupt->next() " << fIsInterrupt.read() << " , CHAIN "<< fIsChain.read() << endl;

							fDescrList.rbegin()->fIsISREn = fIsInterrupt.read();// allocated the ISR
							if(fDescrList.rbegin()->fIsISREn) {
								fDescrList.rbegin()->fISRAddress = 2 + (((fMemManager->fRangeListForFetch.rbegin()->first)>>1)<<1);

								fMemManager->fRangeListForFetch.erase((fMemManager->fRangeListForFetch.end()-1));

								TSubRoutine*iSubRoutine = new TSubRoutine();
								iSubRoutine->fStartAddress = fDescrList.rbegin()->fISRAddress;
								currSubRList.push_back(iSubRoutine);
							}

						} // if(fMemManager->fRangeListForFetch.empty() )fill interrupt

						endOfTCBgen = (!fIsInterrupt.read()  &&  !fIsChain.read());
						if(endOfTCBgen) {
							cerr << "[--DMA] no ISR or no Chain" << endl;
						}

					}

					//add to descriptor list
					if( !endOfTCBgen ) {
						//if not first TCB
						if(!fDescrList.empty()) {
							assert(fDescrList.rbegin()->fIsChained == true || fDescrList.rbegin()->fIsISREn == true);
							//link to prev
							fDescrList.rbegin()->fChainPointer = tmpDescr.fDescLocation;
						}
						assert(tmpDescr.fDescLocation< MAX_CHAIN_ADDRESS);
						fDescrList.push_back(tmpDescr);
					}
				}

			} else {
				endOfTCBgen=true;
			}
		}


		vector<TDecriptor>::iterator last_isr_tcb = fDescrList.begin() ;
		//go over DMA list and make relevant field updates ( e.g count, stride and links
		for(vector<TDecriptor>::iterator d_it = fDescrList.begin() ; d_it != fDescrList.end() ; d_it++) {

			//allocate data
			fMemManager->AddLoadStoreSection(pair<sc_uint<32>,sc_uint<32> >(d_it->fSrdAddr,d_it->fSrdAddr+d_it->fMaxSizeInBytes));
			fMemManager->AddLoadStoreSection(pair<sc_uint<32>,sc_uint<32> >(d_it->fDstAddr,d_it->fDstAddr+d_it->fMaxSizeInBytes));
			//allocate descriptors
			fMemManager->AddLoadStoreSection(pair<sc_uint<32>,sc_uint<32> >(d_it->GetDecriptionSectionStartAddr(),d_it->GetDecriptionSectionEndAddr()));

			if(d_it->fIsISREn) {

				last_isr_tcb->fNextISRAddr = d_it->fISRAddress;

				last_isr_tcb = d_it;//me
			}

			scv_smart_ptr<bool> isOutLoopGen;
			isOutLoopGen->next();


			//find the first dimension : 2 numbers with product less then fMaxSizeInBytes
			scv_smart_ptr<unsigned> val;
			if(8<(d_it->fMaxSizeInBytes)/2) {
				val->keep_only(8,(d_it->fMaxSizeInBytes)/2);
			} else {
				val->keep_only(0,(d_it->fMaxSizeInBytes)/2);
			}
			unsigned x1 = val.read();
			unsigned x2 = 0;
			if(x1==0) x1=1;
			x2=(d_it->fMaxSizeInBytes) / x1;

			assert(x1*x2<=d_it->fMaxSizeInBytes);

			//now split again for second dimension
			unsigned y1 = 0;
			unsigned y2 = 0;
			unsigned y;


			//make all calculation in bytes
			if(isOutLoopGen.read() && (d_it->fChainPointer !=0)/*next descriptor has been allocated*/ ) {
				//make 2D

				d_it->fCountTranOuter=min(x1,x2);
				y = max(x1,x2);
				assert(y);
				y1 = ((unsigned int) rand()) % (y);
				y2 = y - y1;
				d_it->fStrideOut=min(y1,y2);

				assert(max(y1,y2));

				d_it->fStrideIn=   ((unsigned int) rand()) % (max(y1,y2))  ;
				d_it->fCountTranInner=0;
				if( d_it->fStrideIn) d_it->fCountTranInner=   (max(y1,y2)) / d_it->fStrideIn;

			} else {

				d_it->fStrideOut=0;
				d_it->fCountTranOuter=1;//at least one transaction

				d_it->fStrideIn=x2;
				d_it->fCountTranInner=x1;

			}





			cerr << "SHIFT " << ( d_it->fAccessSize ) <<endl;;

			//adjust according to access size
			//d_it->fCountTranInner=d_it->fCountTranInner>>( d_it->fAccessSize );
			d_it->fStrideIn=(d_it->fStrideIn >>( d_it->fAccessSize ))<<( d_it->fAccessSize );
			//d_it->fCountTranOuter=d_it->fCountTranOuter>>( d_it->fAccessSize );
			d_it->fStrideOut=(d_it->fStrideOut >>( d_it->fAccessSize ))<<( d_it->fAccessSize );


			cerr << "[modO coun0] [modIN countIn] "<< hex << d_it->fStrideOut  << " " << d_it->fCountTranOuter << " " << d_it->fStrideIn <<  "  " << d_it->fCountTranInner << endl;

			if(d_it->fModifySign.read() == false) {
				//change direction
				d_it->fSrdAddr= d_it->fSrdAddr + ((d_it->fMaxSizeInBytes >>( d_it->fAccessSize ))<<( d_it->fAccessSize )) - (1 << d_it->fAccessSize);
				d_it->fDstAddr= d_it->fDstAddr + ((d_it->fMaxSizeInBytes >>( d_it->fAccessSize ))<<( d_it->fAccessSize ))-  (1 << d_it->fAccessSize);


				d_it->fStrideIn = -d_it->fStrideIn;
				d_it->fStrideOut = -d_it->fStrideOut;
			}
			cerr << "[modO coun0] [modIN countIn] "<< hex << d_it->fStrideOut  << " " << d_it->fCountTranOuter << " " << d_it->fStrideIn <<  "  " << d_it->fCountTranInner << endl;

			//fill descriptor location
			unsigned addr;
			sc_uint<32> data;

			//CONFIG
			d_it->GetDMAConfig(addr,data);//the first config will be allocated by program
			for( unsigned j = 0 ; j < 4 ; j++ ) {
				fMemManager->SetMem(addr+j,  data(8*(j+1)-1,8*j));
			}
			//DMA_STRIDE IN
			d_it->GetStrideIn(addr,data);
			for( unsigned j = 0 ; j < 4 ; j++ ) {
				fMemManager->SetMem(addr+j,  data(8*(j+1)-1,8*j));
			}
			//DMA_COUNT
			d_it->GetCount(addr,data);
			for( unsigned j = 0 ; j < 4 ; j++ ) {
				fMemManager->SetMem(addr+j,  data(8*(j+1)-1,8*j));
			}
			//STRIDE OUT
			d_it->GetStrideOut(addr,data);
			for( unsigned j = 0 ; j < 4 ; j++ ) {
				fMemManager->SetMem(addr+j,  data(8*(j+1)-1,8*j));
			}
			//DMA_SRCADDR
			d_it->GetSrcAddr(addr,data);
			for( unsigned j = 0 ; j < 4 ; j++ ) {
				fMemManager->SetMem(addr+j,  data(8*(j+1)-1,8*j));
			}
			//DMA_DSTADDR
			d_it->GetDstAddr(addr,data);
			for( unsigned j = 0 ; j < 4 ; j++ ) {
				fMemManager->SetMem(addr+j,  data(8*(j+1)-1,8*j));
			}



		}

	}

	void SimulateDMA(::std::ostream& s= ::std::cout) {
		for(vector<TDecriptor>::iterator d_it = fDescrList.begin() ; d_it != fDescrList.end() ; d_it++) {

			//check last tcb
			if(d_it+1==fDescrList.end()) {
				assert(d_it->fIsChained==false);
				assert(d_it->fIsISREn==false);
			} else {
				assert( (d_it->fIsChained==true) ||  (d_it->fIsISREn==true) );
			}


			sc_uint<32> modifyIn=  abs(d_it->fStrideIn);
			sc_uint<32> modifyOut= abs(d_it->fStrideOut);

			cerr << " modifyIn " <<hex << modifyIn <<endl;
			cerr << " modifyOUT " <<hex << modifyOut <<endl;
			unsigned nBytesInTran = 0;

			switch ( d_it->fAccessSize ) {
			case BYTE_A:
				nBytesInTran=1;
				break;
			case HALF_A:
				nBytesInTran=2;

				break;
			case WORD_A:
				nBytesInTran=4;

				break;
			case DOUBLE_A:
				nBytesInTran=8;

				break;
			}
			cerr << " TCB " <<  d_it - fDescrList.begin()  << endl;

			sc_uint<32> srcAddr =  d_it->fSrdAddr;
			sc_uint<32> dstAddr =  d_it->fDstAddr;

			for( unsigned j=0; j < d_it->fCountTranOuter ; j++ ) {

				cerr << "IT " << j << endl;

				for( unsigned i = 0 ; i < d_it->fCountTranInner ; i++ ) {


					if(d_it->isAutoDMA) {
						cerr << " [" << hex <<   ((fDMAEngineNumber==0)?(DMA0_AUTODMA0):(DMA1_AUTODMA0) )  << "  " << dstAddr<<"]" << dec ;
					} else {
						cerr << " [" << hex << srcAddr << "  " << dstAddr<<"]" << dec ;
					}
					for( unsigned n = 0; n < nBytesInTran; n++) {
						if(d_it->isAutoDMA) {
							s << hex << "0x" <<  ((fDMAEngineNumber==0)?(DMA0_AUTODMA0):(DMA1_AUTODMA0) )        << " 0x" << dstAddr+n<< " "<< (d_it->fAccessSize)<< dec << endl;
						} else {
							s << hex << "0x" << srcAddr+n << " 0x" << dstAddr+n<< " "<< (d_it->fAccessSize)<< dec << endl;
						}
					}

					if(d_it->fModifySign.read() ) {
						assert(d_it->fSrdAddr <= srcAddr && srcAddr <= d_it->fSrdAddr + d_it->fMaxSizeInBytes );
						assert(d_it->fDstAddr <= dstAddr && dstAddr <= d_it->fDstAddr + d_it->fMaxSizeInBytes );
					} else {
						assert(d_it->fSrdAddr >= srcAddr && srcAddr >= d_it->fSrdAddr - d_it->fMaxSizeInBytes );
						assert(d_it->fDstAddr >= dstAddr && dstAddr >= d_it->fDstAddr - d_it->fMaxSizeInBytes );
					}

					//add modify in
					if(d_it->fModifySign.read() ) {
						srcAddr+= modifyIn ;
						dstAddr+= modifyIn ;
					} else {
						srcAddr-=modifyIn ;
						dstAddr-= modifyIn ;
					}

				}
				//add modify out
				if(d_it->fModifySign.read() ){
					srcAddr   += modifyOut;
					dstAddr  +=  modifyOut;
				} else {
					srcAddr   -= modifyOut;
					dstAddr  -=  modifyOut;
				}

				if(d_it->fModifySign.read() ) {
					srcAddr-= modifyIn ;
					dstAddr-= modifyIn ;
				} else {
					srcAddr+=modifyIn ;
					dstAddr+= modifyIn ;
				}
			}
			cerr << endl;

			if(d_it->fIsChained || d_it->fIsISREn ) {
				//continue simulation

			} else {
				break;
			}

		}
	}

};


#endif
