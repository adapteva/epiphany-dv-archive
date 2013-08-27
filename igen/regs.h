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

#ifndef REGS_H
#define REGS_H


#include <limits.h>

//------------------------------------------------------------------------------------------------
//template <int NREGS>
class TRegManager /*: public scv_constraint_base*/ {
private:
	// all registers
	vector <pair<unsigned,sc_uint<32> > > fRF;
	//used as mem pointer
	vector <pair<unsigned,sc_uint<32> > > fConstRegs;
	//modified reg as result of instruction, used for save/restore during ISR
	vector < unsigned > fModifiedRegs;

	scv_smart_ptr< sc_uint<32> > tmpVal;
	scv_smart_ptr<bool> useOnlyLow8BitsVal;//use only low 8 bits for generated reg value

	scv_smart_ptr<sc_uint<32> > fMemRegInd1,fMemRegInd2;
	scv_smart_ptr<sc_uint<32> > fDestReg;

	pair<unsigned, sc_uint<32> > fLoopCountReg;//value is used only init
	pair<unsigned, sc_uint<32> > fIntFrameReg;//value is used only init

	pair<bool,bool> fAZ,fAN, fAC, fAV, fAVS;//<valid,value>

public:
	TRegManager() {
		fMemRegInd1.write(0);
		fMemRegInd2.write(0);
	}

public:

	void InValidateAZ()  {fAZ.first=false;}
	void SetAZ(bool AZ) {
		fAZ.second=AZ;
		fAZ.first=true;
		cerr << "set AZ " << AZ << endl;
	}
	bool GetAZ(bool &AZ) {
		if (fAZ.first)  {
			AZ=fAZ.second;
			return true;
		} else {
			return false;
		}
	}

	void InValidateAN()  {fAN.first=false;}
	void SetAN(bool AN) {
		fAN.second=AN;
		fAN.first=true;
		cerr << "set AN " << AN << endl;
	}
	bool GetAN(bool &AN) {
		if (fAN.first)  {
			AN=fAN.second;
			return true;
		} else {
			return false;
		}
	}

	void InValidateAC()  {fAC.first=false;}
	void SetAC(bool AC) {
		fAC.second=AC;
		fAC.first=true;
		cerr << "set AC " << AC << endl;
	}
	bool GetAC(bool &AC) {
		if (fAC.first)  {
			AC=fAC.second;
			return true;
		} else {
			return false;
		}
	}

	void InValidateAV()  {fAV.first=false;}
	void SetAV(bool AV) {
		fAV.second=AV;
		fAV.first=true;
		cerr << "set AV " << AV << endl;
	}
	bool GetAV(bool &AV) {
		if (fAV.first)  {
			AV=fAV.second;
			return true;
		} else {
			return false;
		}
	}


	void InValidateAVS()  {fAVS.first=false;}
	void SetAVS(bool AVS) {
		fAVS.second=AVS;
		fAVS.first=true;
		cerr << "set AVS " << AVS << endl;
	}
	bool GetAVS(bool &AVS) {
		if (fAVS.first)  {
			AVS=fAVS.second;
			return true;
		} else {
			return false;
		}
	}

	void InvalidateAllFlags() {
		InValidateAZ();
		InValidateAN();
		InValidateAC();
		InValidateAV();
		InValidateAVS();
	}

	void Init() {

		//this->next();

		fRF.clear(); //TODO


		fMemRegInd1->keep_only(2,NREGS-2);
		fMemRegInd1->next();

		fLoopCountReg.first = (fMemRegInd1.read());
		fIntFrameReg.first = (fMemRegInd1.read()+1);

		fMemRegInd2.write(fMemRegInd1.read()+2);

		for( unsigned i = 0 ; i < NREGS; i++ ) {

			//fRF.push_back(pair<unsigned,sc_uint<32> >(i, (tmpVal.read()>>1) ) ) ;//help for align access gen
			if( i != GetLoopCountRegisterInd() ) {
				//tmpVal->keep_only(0,UINT_MAX);
			} else {
				cerr <<"setting loop reg" << i << endl;
			}
			tmpVal->next();
			if( i != GetLoopCountRegisterInd() ) {
				if( i != GetISRContextSaveRegInd() ) {
					useOnlyLow8BitsVal->next();
					if(useOnlyLow8BitsVal.read()) {
						SetReg(i, tmpVal.read()(3,0));
					} else {
						SetReg(i, tmpVal.read());
					}
				} else {
					//set by linker
				}
			} //loop
			else {

				if(isFPonly) {
					fLoopCountReg.second =0;
				} else {
					fLoopCountReg.second = (1+(MAXLOOP_IT)) ;
				}
			}

		}




	}

public:

	//record the index of  modified reg as result of some instruction. The called only by Set or Invalidate Reg
	void RecordChangedReg(unsigned int ind) {


		assert(ind < NREGS);

		//find if reg is already in vector
		vector <unsigned>::iterator it = find(fModifiedRegs.begin() , fModifiedRegs.end() , ind);
		if ( it == fModifiedRegs.end() ) {
			fModifiedRegs.push_back(ind )  ;
		}

		cerr << dec << "Record---REG  " << ind <<endl;
	}
	//print saved context
	void PrintInterruptSaveContext(unsigned kIntNum ,::std::ostream& s= ::std::cout) {
		for( vector <unsigned>::iterator it =fModifiedRegs.begin() ; it != fModifiedRegs.end() ; it++) {
			s << "str r" << dec << *it << " ,[r" << fIntFrameReg.first<< hex << ",+0x" << ( (it - fModifiedRegs.begin())   )<< dec <<"+"<< kIntNum*NREGS<< "] "  << endl;
		}

	}
	//print restored context
	void PrintInterruptRestoreContext(unsigned kIntNum ,::std::ostream& s= ::std::cout) {
		for( vector <unsigned>::iterator it =fModifiedRegs.begin() ; it != fModifiedRegs.end() ; it++) {
			s << "ldr r" << dec << *it << " ,[r" << fIntFrameReg.first<< hex << ",+0x" << ( (it - fModifiedRegs.begin())   )<< dec <<"+"<< kIntNum*NREGS<< "] "  << endl;
		}
	}

	//Create and set register that will be used as pointer in memory operation, can't be used as destination reg in any instruction
	void CreateSetMemPointerReg(sc_uint<32> val) {
		unsigned ind;
		if(fMemRegInd2.read() == NREGS) {

			assert(fMemRegInd1.read()>0);
			fMemRegInd1.write(fMemRegInd1.read()-1);
			ind = fMemRegInd1.read();
		} else {

			ind =fMemRegInd2.read();
			assert((fMemRegInd2.read()+1)<=NREGS);
			fMemRegInd2.write(fMemRegInd2.read()+1);
		}

		SetReg(ind,val);
		fConstRegs.push_back(pair<unsigned,sc_uint<32> >(ind,  val ));


	}
	unsigned GetISRContextSaveRegInd() {
		return fIntFrameReg.first;
	}

	unsigned GetLoopCountRegisterInd() {
		return fLoopCountReg.first;
	}

	bool ifRegIsMemPointer(unsigned reg) {
		return ( (fMemRegInd1.read() <=reg ) && (reg<= fMemRegInd2.read()) );
	}

	pair<unsigned,unsigned > GetMemPoitersRegs() {
		return pair<unsigned,unsigned >(fMemRegInd1.read(),fMemRegInd2.read());
	}
	/*
	unsigned  GenDestReg() {
		unsigned ind;

		fDestReg->keep_out(fMemRegInd1.read(),fMemRegInd2.read()-1);
		fDestReg->keep_only(0,NREGS-1);
		fDestReg->next();
		ind = fDestReg.read();

		cerr << "get NORMAL dst reg " <<dec << ind << endl;
		assert(ind < NREGS);

		return ind;
	}
	unsigned  GenDestRegDouble() {

		unsigned ind;

		fDestReg->keep_out((fMemRegInd1.read())/2,(fMemRegInd2.read())/2);
		fDestReg->keep_only(0,(NREGS/2)-1);
		fDestReg->next();
		ind = fDestReg.read();
		ind=ind*2;

		cerr << "get DOUBLE dst reg " <<dec << ind << endl;
		assert(ind < NREGS);
		return ind;
	}
	*/
	//generate index in RF
	bool GenValidReg(unsigned& ind) {
		if(fRF.empty()) {
			return false;
		} else {
			ind = fRF[rand() % fRF.size()].first;
			return true;
		}
	}

	void SetReg( unsigned ind, sc_uint<32> val) {

//		assert((  (fMemRegInd1.read() >ind ) || (ind>= fMemRegInd2.read()) ));

		assert(ind < NREGS);

		//find if reg is already in vector
		vector <pair<unsigned,sc_uint<32> > >::iterator it;
		for ( it  = fRF.begin() ; it != fRF.end(); it++ ) {
			if ( it->first == ind ) {
				it->second = val;
				break;
			}
		}
		if ( it == fRF.end() ) {
			fRF.push_back(pair<unsigned,sc_uint<32> >(ind,  val ) ) ;
		}

		cerr << dec << "SetREG  " << ind << " = " <<hex  << val  <<dec << endl;
		RecordChangedReg(ind);

	}

	bool GetReg( unsigned ind, sc_uint<32>& val) {

		bool retSt = false;//not found

		assert(ind < NREGS);
		//find if reg is already in vector
		vector <pair<unsigned,sc_uint<32> > >::iterator it;
		for ( it  = fRF.begin() ; it != fRF.end(); it++ ) {
			if ( it->first == ind ) {
				val = it->second;
				retSt = true;
				break;
			}
		}
		//if (retSt ) cerr << " GetREG  " << ind << " = " << val  << endl;
		//else cerr << "not found " << ind << endl;

		return retSt;
	}


	//used by start of subroutine
	void InvalidateAllRegs() {
		fRF.clear();
		cerr << "Invalidate All regs " <<  endl;
		//copy const
		vector <pair<unsigned,sc_uint<32> > >::iterator it;
		for ( it  = fConstRegs.begin() ; it != fConstRegs.end(); it++ ) {
			fRF.push_back(*it);

		}

		//reset the value -- assuming - -calling in start of service
		fModifiedRegs.clear();

		//invalidate flags
		InvalidateAllFlags();


	}

	void InvalidateReg(unsigned ind) {
		assert(ind < NREGS);

		assert(( (fMemRegInd1.read() >ind ) || (ind>= fMemRegInd2.read()) ));

		vector <pair<unsigned,sc_uint<32> > >::iterator it;
		for ( it  = fRF.begin() ; it != fRF.end(); it++ ) {
			if ( it->first == ind ) {
				break;
			}
		}
		if( it != fRF.end() ) {
			fRF.erase(it);
		}
		cerr << "InvalidateReg " << ind << endl;
		RecordChangedReg(ind);

	}
	unsigned GetValidArraySize() {
		return  fRF.size();
	}

	void PrintReg(unsigned ind, sc_uint<32> val ,::std::ostream& s= ::std::cout) {
		s <<dec  << "MOV R" << ind <<  ",%low(0x"<<hex << val << ")" << dec << endl;
		s <<dec  << "MOVT R" << ind << ",%high(0x"<<hex << val << ")"<< dec << endl;
	}

	void PrintMe(::std::ostream& s= ::std::cout ) {
		vector <pair<unsigned,sc_uint<32> > >::iterator it;
		for ( it  = fRF.begin() ; it != fRF.end(); it++ ) {

			s <<dec  << "MOV R" << it->first <<  ",%low(0x" << hex << it->second  << ")" << dec << endl;
			if( it->second  >= (1<<16) )
				s <<dec  << "MOVT R" << it->first << ",%high(0x" << hex <<  it->second << ")"<< dec << endl;

			//s << "MOV R" << it->first << hex << ",#0x" << it->second  << ";" <<dec <<endl;
		}
		s <<dec  << "MOV R" << fLoopCountReg.first <<  ",%low(0x" << hex <<fLoopCountReg.second  << ");/*for loop*/" << dec << endl;

		s <<dec  << "MOV R" << fIntFrameReg.first <<  ",%low(ISR_SAVE_RESTORE_SPACE)" << dec << endl;
		s <<dec  << "MOVT R" << fIntFrameReg.first << ",%high(ISR_SAVE_RESTORE_SPACE)"<< dec << endl;

	}

	//save all register to ISR SAVE/RESTORE space
	void DumpAllRegsToMem(::std::ostream& s, unsigned byte_offset) {
		vector <pair<unsigned,sc_uint<32> > >::iterator it;
		for( unsigned i = 0 ; i < (NREGS/2) ; i++ ) {
			s << "strd r" << i*2 << " ,[r" << GetISRContextSaveRegInd() << " , "<< i  +  byte_offset/8 << " ]" << endl;
		}
	}

};

#endif

