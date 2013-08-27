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


#ifndef BRANCH_I_H
#define BRANCH_I_H



class TBranchInstr 
:       public TInstr_If ,
public scv_constraint_base
{

	//implement the Instruction interface
public:
	virtual sc_uint<32> GetNextPC() { return fMemAddr; }//return target address of jump
	virtual sc_uint<32> GetInstructionSize() {
		if(isSetRegisterSet) return 12;
		else return 4 ;
	}

	virtual void PrintMe(::std::ostream& s= ::std::cout ) {
		if(isSetRegisterSet) {
			assert( fRjumpInd.read()!=14);

			s <<dec  << "MOV R" << fRjumpInd.read() <<  ",%low(0x" << hex << fMemAddr << ")" << dec << endl;
			s <<dec  << "MOVT R" << fRjumpInd.read() << ",%high(0x" << hex << fMemAddr << ")"<< dec << endl;
		}

		if(jumpToEndOfTest) {
			s <<dec  << "MOV R0"  <<  ",%low(END_OF_TEST);" << dec << endl;
			s <<dec  << "MOVT R0"  << ",%high(END_OF_TEST);"<< dec << endl;
			s << "JR R0; " <<  endl;
			s << ";;;;//no more memoty for jump -- finishing test " << endl;

		} else {

			if ( fLabelOrReg.read() ) { //Label
				s << "B";
			} else { // reg jump
				s << "J";
			}
			if( fIsCond.read() && fLabelOrReg.read() ) {

				PrintCondS::PrintCond(fCondType.read(),s);
			}



			if ( fIsLinked.read() ) { //Linked
				if ( !fLabelOrReg.read() ) 	s << "A";
				s << "L";
			}


			if ( fLabelOrReg.read() ) { //Label
				//if()
				s << " L_" << hex << fMemAddr << dec << ";/*pc->>*/" << endl;
			} else { // reg jump
				s << "R R"  << fRjumpInd.read();
				s << hex << ";/*" << fMemAddr << "pc->>*/"<< dec<<endl;
			}

		}

	}

	//Constraint part

public:


	scv_smart_ptr<bool> fLabelOrReg;
	scv_bag<bool> fLabelOrRegDist;

	scv_smart_ptr<bool> fIsCond;
	scv_bag<bool> 		fIsCondDist;

	scv_smart_ptr<bool> fIsLinked;
	scv_smart_ptr<ECondCode> fCondType;
	scv_smart_ptr<unsigned> fRjumpInd;

	bool jumpToEndOfTest;
	bool isSetRegisterSet;


public:
	SCV_CONSTRAINT_CTOR(TBranchInstr) {

		SCV_CONSTRAINT(fRjumpInd() < NREGS);
		SCV_CONSTRAINT(fRjumpInd() != 14);

		// cond only for label based instr
		SCV_CONSTRAINT(if_then(fLabelOrReg()  == false ,  fIsCond() != true ));
		SCV_CONSTRAINT(if_then(fIsLinked()  == true ,  fIsCond() != true ));

		SCV_CONSTRAINT(fCondType() != Unconditional);
		SCV_CONSTRAINT(fCondType() != Branch_Link);

		jumpToEndOfTest=false;
		isSetRegisterSet=false;

		fIsCondDist.add(true,99);
		fIsCondDist.add(false,01);
		fIsCond->set_mode(fIsCondDist);

	}

protected:

	sc_uint<32> fMemAddr;
public:

	virtual void GenerateMe() {
		this->next();
	}



	void next() {

		jumpToEndOfTest=false;
		fMemAddr=0;
		isSetRegisterSet=false;
		fRjumpInd->enable_randomization();

		fLabelOrReg->enable_randomization();
		fLabelOrReg->next();



		if( fMemManager && fRegsManager ) cerr << " Start Gen Jump" << endl;

		if(fLabelOrReg.read() == false) {
			//go over register file
			unsigned ind;
			sc_uint<32> val;
			if( fMemManager && fRegsManager ) {

				for(unsigned i = 0 ; i < NREGS ; i++ ) {
					if ( fRegsManager->GetReg(i,val) ) {
						//Initialize by reg val
						pair<sc_uint<32>  ,sc_uint<32> > rangeToCheck =
								pair<sc_uint<32>  ,sc_uint<32> >(val, val+MAX_SUB_ROUTINE_SIZE);

						//check if register can be jump target
						if((i!=14) && (( (rangeToCheck.first) %2 ) == 0) && (rangeToCheck.first < rangeToCheck.second) && fMemManager->GetRangeIndexForFetch(ind,rangeToCheck) ) {
							fRjumpInd->disable_randomization();
							*fRjumpInd=i;
							//cerr << "jump to R " << i << endl;
							fMemAddr=rangeToCheck.first;
							//get ind
							fMemManager->RemoveUpdateRangeFetch(ind,rangeToCheck);
							//found
							break;
						} else { //just to print for sure
							cerr << "SQ: tried reg " << i << " with value " << hex << val <<dec  << endl;
						}
					}
				}

			}

			if( ind < NREGS) {
				cerr << "SQ: found register as jump target: " <<  fRjumpInd.read() <<endl;
				//found register as jump target
				//*fLabelOrReg = false;//can be reg
			} else {
				//have to generate the label
				fLabelOrReg.write( true) ;//should be label
			}


		}



		fLabelOrReg->disable_randomization();

		scv_constraint_base::next();

		if( fRegsManager  && fMemManager && fLabelOrReg.read() ) {

			pair<sc_uint<32>,sc_uint<32> >  genRange;
			if( fMemManager->GetGenRangebyJump( genRange, MAX_SUB_ROUTINE_SIZE, MAX_SUB_ROUTINE_SIZE) ) {

				fMemAddr = genRange.first;

				//make alignment
				if( (fMemAddr %2) != 0 ) {  fMemAddr++ ;}

				if(abs(int(fMemAddr - fCpc)) < (1<<20)/*to be on the safe side*/) {
					cerr << "gen label jump address " << hex << fMemAddr << dec;
				} else {
					cerr << hex  << "have to jump by register, pc(0x"<< fCpc <<") since no sufficient bits in immm for addr: "  << fMemAddr << dec << endl;
					isSetRegisterSet=true;

					fRjumpInd->keep_only(0,NREGS-1);
					fRjumpInd->keep_out(fRegsManager->GetMemPoitersRegs().first, fRegsManager->GetMemPoitersRegs().second);
					fRjumpInd->keep_out(14);

					fRjumpInd->next();


					fRegsManager->SetReg(fRjumpInd.read(),fMemAddr);
					*fLabelOrReg = false;//have to be reg
				}


			} else {
				jumpToEndOfTest=true;
				fMemAddr=0xffffffff;//fake
				cerr << "no more instruction memory" << endl;
				//exit(9);
			}
		}


		if( fRegsManager && fMemManager) {

			if(fRegsManager->ifRegIsMemPointer(14) ) {
				fIsLinked.write(false);
			}

			if(fIsLinked.read()) {
				fRegsManager->InvalidateReg(14/*link register*/);

			}

			PrintMe(cerr);
		}


	}
};
#endif
