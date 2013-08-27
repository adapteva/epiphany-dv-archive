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



#ifndef LOAD_STORE_I_H
#define LOAD_STORE_I_H

extern unsigned int MAXLOOP_IT;
//each load store operation will allocate the unique buffer
#define LS_MIN_BUFFER_SIZE 16

enum ELS_type {

	IMM_OFFSET,
	POSTMOD_IMM,
	REG_OFFSET,
	POSTMOD_REG
};

template<>
class scv_extensions<ELS_type> : public scv_enum_base<ELS_type> {
public:

	SCV_ENUM_CTOR(ELS_type) {

		SCV_ENUM(IMM_OFFSET);
		SCV_ENUM(POSTMOD_IMM);
		SCV_ENUM(REG_OFFSET);
		SCV_ENUM(POSTMOD_REG);
	}
};

enum ELS_sign {
	ADD_OP,
	SUB_OP
};

template<>
class scv_extensions<ELS_sign> : public scv_enum_base<ELS_sign> {
public:
	SCV_ENUM_CTOR(ELS_sign) {

		SCV_ENUM(ADD_OP);
		SCV_ENUM(SUB_OP);
	}
};


enum ELS_immSize {
	LS_IMM3,
	LS_IMM11
};

template<>
class scv_extensions<ELS_immSize> : public scv_enum_base<ELS_immSize> {
public:
	SCV_ENUM_CTOR(ELS_immSize) {

		SCV_ENUM(LS_IMM3);
		SCV_ENUM(LS_IMM11);
	}
};








class TLSInstr
:       public TInstr_If ,
public scv_constraint_base
{
	//implement the Instruction interface
public:


	virtual sc_uint<32> GetInstructionSize() {
		if( !validInstr) {return 0;}
		else{	return 4 ;}
	}

	virtual void PrintMe(::std::ostream& s= ::std::cout ) {


		s<< dec;
		//if( fIsRegInitNeeded ) {
		//	s<< "MOV R" <<  fRdInd.read() << ",#0x" << hex << fPreInitVal.read()<< ";/*set reg*/" << dec <<endl;
		//}

		if(!validInstr) {
			//s <<hex << "/*nop;*//*load/store generation fails " << fMemAddr << "*/" << dec << endl;

		}//generation fails !!!
		else {


			if(isReplaceLoadByTestandSet && fAccessSize.read()==WORD_A ) {
				s << "TESTSET";

			} else {

				if (fLoS.read()  == true ) { //load
					s << "LDR";
				} else { //store
					s << "STR" ;
				}
			}
			switch ( fAccessSize.read() ) {
			case BYTE_A:
				s << "B";
				break;
			case HALF_A:
				s << "H";
				break;
			case WORD_A:
				s << "";
				break;
			case DOUBLE_A:
				s << "D";
				break;
			}

			if(isForced32bitsOpcode) {
				s <<".l";
			}

			s << " R" << fRdInd.read() ;

			s << ",[R" << fRnInd.read() ;

			if( fType.read() == POSTMOD_IMM || fType.read() == POSTMOD_REG ) {
				s << "]";
			}
			s << ",";

			if( fType.read() == IMM_OFFSET || fType.read() == POSTMOD_IMM ) {
				s << "#";
			}


			if( fSign.read() == ADD_OP ) {
				s << "+";
			}

			if( fSign.read() == SUB_OP) {
				s << "-";
			}
			if( fType.read() == REG_OFFSET || fType.read() == POSTMOD_REG ) {
				s << "R" << fRmInd.read();
			}

			if( fType.read() ==  IMM_OFFSET || fType.read() == POSTMOD_IMM ) {

				s << dec << "(" << printedImm << ")" ;

			}

			if( fType.read() == IMM_OFFSET || fType.read() == REG_OFFSET ) {
				s << "]";
			}


			s << hex << ";/*" << fMemAddr << "*/"<< dec <<endl;


		}


	}

	//Constraint part

public:
	scv_smart_ptr<bool> fLoS;
	scv_smart_ptr<ELS_type> fType;
	scv_smart_ptr<ELS_sign> fSign;
	scv_bag<ELS_AccessSize> fAccessSizeDist;
	scv_smart_ptr<ELS_AccessSize> fAccessSize;
	scv_smart_ptr<unsigned> fRdInd;

	scv_smart_ptr<unsigned> fRnInd;
	scv_smart_ptr<unsigned> fRmInd;

	scv_smart_ptr<sc_uint<3>  > fUimm3;
	scv_smart_ptr<sc_uint<11> > fUimm11;

	// address range used by imm extension mode
	pair<sc_uint<32>,sc_uint<32> >  genRange;

	scv_smart_ptr<sc_uint<16> > fPreInitVal;//used to init reg if no point reg available

	scv_smart_ptr<bool> isSafeLoadOp;// used to generate the load from memory which is not protected, this means the memory is not removed from space

	scv_smart_ptr<bool> isGoRegOffset;
	sc_int<32>  printedImm;

	scv_smart_ptr<bool> isConvert2TestAndSet;// convert the POST_MOD load word to testaset
	bool isReplaceLoadByTestandSet;


public:
	SCV_CONSTRAINT_CTOR(TLSInstr) {

		SCV_CONSTRAINT(fRnInd() < NREGS);
		SCV_CONSTRAINT(fRmInd() < NREGS);

		isReplaceLoadByTestandSet=false;
		//isConvert2TestAndSet->keep_only(false);
		isConvert2TestAndSet->next();


		if(isGenDMA || isGenInter ) {
			isSafeLoadOp->keep_only(true);
		}


		extern bool isFPonly;
		if(isFPonly) {
			SCV_CONSTRAINT(fAccessSize() == DOUBLE_A);

			//fAccessSizeDist.add(BYTE_A,1);
			//fAccessSizeDist.add(HALF_A,1);
			//fAccessSizeDist.add(WORD_A,1);
			//fAccessSizeDist.add(DOUBLE_A,10000000);

		} else {
			fAccessSizeDist.add(BYTE_A,10);
			fAccessSizeDist.add(HALF_A,10);
			if(isConvert2TestAndSet.read() == true) {
				fAccessSizeDist.add(WORD_A,500);
			} else {
				fAccessSizeDist.add(WORD_A,10);
			}
			fAccessSizeDist.add(DOUBLE_A,10);

			fAccessSize->set_mode(fAccessSizeDist);
		}



		//SCV_SOFT_CONSTRAINT(fLoS()  == false ) ;
	}

protected:
	sc_uint<32> fMemAddr;
	//sometime no more registers ... reinit
	bool fIsRegInitNeeded;
	//some time the igen't can allocate the load store instruction  TODO make some ...
	bool validInstr;

public:
	virtual void GenerateMe() {
		this->next();
	}



	void next() {


		fIsRegInitNeeded = false;
		validInstr = true;
		//cerr << "fRnInd --- " <<  fRnInd.read() << endl;
		fRnInd->enable_randomization();
		fRmInd->enable_randomization();
		//fSign->enable_randomization();
		//cerr << "fRnInd ---2 " <<  fRnInd.read() << endl;


		fSign->enable_randomization();

		pair<sc_uint<32>  ,sc_uint<32> > rangeToCheck;

		//memory address should be in valid memory range
		//first find Rn regsiter for postmodify and after that try the index mode
		//if no register ( or register pair Rn,Rm) found go for IMM_OFFSET only

		fType->disable_randomization();


		unsigned ind,ind2;
		sc_uint<32> val,val2;
		bool canBePostMod = false;
		unsigned int i = 0 ;


		if( fMemManager && fRegsManager ) { //TODO bind reg to static object --- !!!
			//checking the first index register
			//go over register file

			cerr << "LS gen start" << endl;

			for( i = 0 ; i < NREGS ; i++ ) {
				if ( fRegsManager->GetReg(i,val) ) {
					rangeToCheck =   pair<sc_uint<32>  ,sc_uint<32> >(val, val+LS_MIN_BUFFER_SIZE);
					//if register can be the mem address @ postmodify
					if( (rangeToCheck.first< rangeToCheck.second) && fMemManager->GetRangeIndexForLS(ind,rangeToCheck) ) {
						//get ind
						canBePostMod = true;
						ind = i;
						break;
					}

				}
			}


			//if found at least one reg as candidate for pos mod operation
			if(canBePostMod) { //check for index option Rm -/+ Rn
				cerr  << " LS can be post modify" << endl;

				isGoRegOffset->next();
				if(isGoRegOffset.read()) {
					for( i = 0 ; i < NREGS ; i++ ) {
						if ( fRegsManager->GetReg(i,val2) ) {

							//check for ADD_OP
							rangeToCheck = pair<sc_uint<32>  ,sc_uint<32> >(val+val2, val+val2+LS_MIN_BUFFER_SIZE);
							//if register can be the mem address @ postmodify
							if( (rangeToCheck.first< rangeToCheck.second) &&  fMemManager->GetRangeIndexForLS(ind2,rangeToCheck) ) {

								fSign->disable_randomization();
								*fSign =ADD_OP;
								ind2 = i;
								//get ind
								break;

							}

							//check for SUB_OP
							rangeToCheck = pair<sc_uint<32>  ,sc_uint<32> >(val-val2, val-val2+LS_MIN_BUFFER_SIZE);
							//if register can be the mem address @ postmodify
							if( (rangeToCheck.first< rangeToCheck.second) && fMemManager->GetRangeIndexForLS(ind2,rangeToCheck) ) {

								fSign->disable_randomization();
								*fSign = SUB_OP;
								ind2 = i;
								//get ind
								break;

							}

						}//if ( fRegsManager->GetReg(i,val2) )
					}
				}



				//if found Rm
				if ( i < NREGS && isGoRegOffset.read() ) {


					//should be mem = Rn+/-Rm
					*fType = REG_OFFSET;
					fRnInd->disable_randomization(); *fRnInd = ind;
					fRmInd->disable_randomization(); *fRmInd = ind2;

					fMemAddr=((fSign.read() == ADD_OP)? (val + val2) : (val - val2));

					cerr << "got Rn,Rm " << ind << "," << ind2 << "=" << hex << val << "," << val2 << "="<< fMemAddr<< dec <<endl;

				} else {

					//check if is not dedicated memory pointer
					if( fRegsManager->ifRegIsMemPointer(ind) ) {
						cerr << "R"<< ind <<" is  dedicated memory pointer , can't use as post modify" << endl;

						fType.write(IMM_OFFSET);

					} else {

						//should be  mem = Rn
						fType->enable_randomization();
						fType->keep_out(REG_OFFSET);
						fType->keep_out(IMM_OFFSET);
						fType->next();

						fType->disable_randomization();


						assert((fType.read() == POSTMOD_REG) || (fType.read() == POSTMOD_IMM));
						fRnInd->disable_randomization(); *fRnInd = ind;


						fMemAddr=val;
						if(fType.read() == POSTMOD_REG)  cerr << "POSTMOD_REG" << endl;
						if(fType.read() == POSTMOD_IMM)  cerr << "POSTMOD_IMM" << endl;
						cerr << "LS:  got Rn " << ind  << "="<< hex  << fMemAddr << dec << endl;
					}

				}


				if(fType.read() != IMM_OFFSET ) {
					//update the memory address

					pair<sc_uint<32>,sc_uint<32> > rangeTodelete =
							pair<sc_uint<32>  ,sc_uint<32> >(fMemAddr,fMemAddr +LS_MIN_BUFFER_SIZE);
					assert( fMemManager->GetRangeIndexForLS(ind,rangeTodelete) );

					fMemManager->RemoveUpdateRangeLS(ind,rangeTodelete);
					fMemManager->AddLoadStoreSection(rangeTodelete);

					isSafeLoadOp->disable_randomization();
					isSafeLoadOp.write(true);

					//constrain access size
					if((fMemAddr %8) != 0) {
						fAccessSize->keep_out(DOUBLE_A);
					}

					if((fMemAddr %4) != 0) {
						fAccessSize->keep_out(WORD_A);
					}
					if((fMemAddr %2) != 0) {
						fAccessSize->keep_out(HALF_A);
					}
				}

			} else {//should go only for imm option

				*fType = IMM_OFFSET;
				cerr << "LS: ( no Rn or Rn+-Rm ) should go for ... IMM .... " << endl;
				//fSign->disable_randomization();
			}
		}


		scv_constraint_base::next();



		if( fRegsManager ) {

			sc_uint<32> tmpImm;
			if( fType.read() == IMM_OFFSET ) {
				//if ( fRegsManager->GetValidArraySize() == 0) 	fIsRegInitNeeded = true;
				//if(fIsRegInitNeeded) {
				//	fRegsManager->SetReg(fRdInd.read(),((fPreInitVal.read() ) )	);

				//}
				// no more registers
				if ( fRegsManager->GetValidArraySize() == 0) 	{
					validInstr=false;
				}

				if( validInstr && fRegsManager->GenValidReg(ind) ) {
					assert( fRegsManager->GetReg(ind,val));

					//generate range
					pair<sc_uint<32>,sc_uint<32> >  genRange;

					//fMemManager->GetGenRangebyLoadStore(genRange,LS_MIN_BUFFER_SIZE,LS_MIN_BUFFER_SIZE);
					//fMemAddr = genRange.first;
					switch ( fAccessSize.read() ) {
					case BYTE_A:
						cerr << "B";
						break;
					case HALF_A:
						cerr  << "H";
						break;
					case WORD_A:
						cerr << "W";
						break;
					case DOUBLE_A:
						cerr << "D";
						break;
					}
					cerr <<endl;

					unsigned immShift;
					//try to find the register as pointer
					for( i = 0 ; i < NREGS ; i++ ) {
						pair<sc_uint<32>,sc_uint<32> > rangeToCheck;


						if(fRegsManager->GetReg(i,val) ) {
							cerr << "LS: check for reg  [" <<dec  << i <<"]" << hex << val << endl;

							unsigned k = 0;
							for ( k=0 ; k < 15 ; k++ ) {
								if (fSign.read() == ADD_OP) fMemAddr = val + ((rand() %  (1<<(14-k)) ) ) ;
								if (fSign.read() == SUB_OP) fMemAddr = val - ((rand() %  (1<<(14-k)) ) );

								if((fAccessSize.read() ==  DOUBLE_A ))   {

									fMemAddr = ((fMemAddr>>3)<<3 )+8;
									rangeToCheck=pair<sc_uint<32>  ,sc_uint<32> >(fMemAddr,fMemAddr +LS_MIN_BUFFER_SIZE);
									if( (rangeToCheck.first<rangeToCheck.second) &&   ((val%8)==0) && ( fMemManager->GetRangeIndexForLS(ind,rangeToCheck) )&& (abs(long(fMemAddr - val)) < (1<<14) )   ) {
										immShift=3;break;
									}
								}
								if((fAccessSize.read() ==  WORD_A ))  {
									fMemAddr = ((fMemAddr>>2)<<2 )+4;
									rangeToCheck=pair<sc_uint<32>  ,sc_uint<32> >(fMemAddr,fMemAddr +LS_MIN_BUFFER_SIZE);
									if( (rangeToCheck.first<rangeToCheck.second ) &&  ((val%4)==0)  && ( fMemManager->GetRangeIndexForLS(ind,rangeToCheck) )&&   (abs(long(fMemAddr - val)) < (1<<13) )  ) {
										immShift=2;break;
									}
								}

								if((fAccessSize.read() ==  HALF_A ))   {
									fMemAddr =( (fMemAddr>>1)<<1 )+2;
									rangeToCheck=pair<sc_uint<32>  ,sc_uint<32> >(fMemAddr,fMemAddr +LS_MIN_BUFFER_SIZE);
									if((rangeToCheck.first<rangeToCheck.second ) && ((val%2)==0)	&& ( fMemManager->GetRangeIndexForLS(ind,rangeToCheck) )&&	(abs(long(fMemAddr - val)) < (1<<12) )   ){
										immShift=1;break;
									}
								}

								if((fAccessSize.read() ==  BYTE_A ))  {
									rangeToCheck=pair<sc_uint<32>  ,sc_uint<32> >(fMemAddr,fMemAddr +LS_MIN_BUFFER_SIZE);
									if( (rangeToCheck.first<rangeToCheck.second )&& ( fMemManager->GetRangeIndexForLS(ind,rangeToCheck) ) && (abs(long(fMemAddr - val)) < (1<<11) )        ){
										immShift=0;break;
									}
								}

							}
							//check if found
							if(k < 15) break;

						}


					}
					if ( i ==  NREGS) {
						pair<sc_uint<32>,sc_uint<32> > lastTryRange;

						cerr << "LS: no good register found ... ...  skip LS generation(not valid instruction)" << endl;
						fMemAddr=0xffffffff;

						validInstr=false;

					} else {
						fRnInd.write(i);

						tmpImm = (fSign.read() == ADD_OP) ? (fMemAddr-val):(val -fMemAddr);

						cerr << dec << "IMM access" << "Ind reg [" << i << "] = " << hex<< val ;
						cerr << hex << " tmp IMM " << tmpImm << "  addr in mem " << fMemAddr<< ((fSign.read() == ADD_OP) ? "+":"-" ) << dec << endl;
						//update the memory address

						pair<sc_uint<32>,sc_uint<32> > rangeTodelete =
								pair<sc_uint<32>  ,sc_uint<32> >(fMemAddr,fMemAddr +LS_MIN_BUFFER_SIZE);
						assert( fMemManager->GetRangeIndexForLS(ind,rangeTodelete) );

						if( fLoS.read() && !isSafeLoadOp.read()) {

							fMemManager->AddLoadMultiUseRange(rangeTodelete);
						} else {
							fMemManager->RemoveUpdateRangeLS(ind,rangeTodelete);

							fMemManager->AddLoadStoreSection(rangeTodelete);
						}


						printedImm = tmpImm;
						printedImm = printedImm>>immShift;
						tmpImm = tmpImm>>immShift;


						fUimm11.write(tmpImm(10,0));
						cerr << "Shifted Imm " <<hex << fUimm11.read() << dec << endl;



					}



				} else {
					cerr << "LS no more [index] registers available ..  skip LS generation(not valid instruction)" << endl;
					validInstr=false;
					///assert(0);
				}
			}


			/////////////////
			//validate
			if(validInstr && (fType.read() ==  IMM_OFFSET) ) {
				sc_uint<32> regVal;
				assert(fRegsManager->GetReg(fRnInd.read(),regVal));



				cerr << "Printed " << hex << printedImm << dec << " " << printedImm << endl;
				if(printedImm<0 ) {
					if ( fSign.read() == ADD_OP)
						fSign.write(SUB_OP);
					else
						fSign.write(ADD_OP);
					printedImm = printedImm * (-1);
				}


				if( fSign.read() == ADD_OP ) {
					switch ( fAccessSize.read() ) {
					case BYTE_A:
						assert(fMemAddr == sc_uint<32>(regVal + sc_uint<32>(printedImm)));
						break;
					case HALF_A:
						assert(fMemAddr == sc_uint<32>(regVal + sc_uint<32>((printedImm<<1))));
						break;
					case WORD_A:
						assert(fMemAddr == sc_uint<32>(regVal + sc_uint<32>((printedImm<<2))));
						break;
					case DOUBLE_A:
						assert(fMemAddr == sc_uint<32>(regVal + sc_uint<32>((printedImm<<3))));
						break;
					}
				}



				if( fSign.read() == SUB_OP) {
					switch ( fAccessSize.read() ) {
					case BYTE_A:

						assert(fMemAddr == sc_uint<32>(regVal - sc_uint<32>(printedImm)));
						break;
					case HALF_A:

						assert(fMemAddr == sc_uint<32>(regVal - sc_uint<32>((printedImm<<1))));
						break;
					case WORD_A:
						assert(fMemAddr == sc_uint<32>(regVal - sc_uint<32>((printedImm<<2))));
						break;
					case DOUBLE_A:
						assert(fMemAddr == sc_uint<32>(regVal - sc_uint<32>((printedImm<<3))));
						break;
					}
				}

			}

			if(validInstr) {
				if( fType.read() == POSTMOD_IMM || fType.read() == POSTMOD_REG ) {

					fUimm11->next();
					printedImm = fUimm11.read();
					cerr << dec << printedImm << endl;
					assert(printedImm>=0);

					unsigned shiftPost;
					switch ( fAccessSize.read() ) {
					case BYTE_A:
						shiftPost=0;
						break;
					case HALF_A:
						shiftPost=1;
						break;
					case WORD_A:
						shiftPost=2;
						break;
					case DOUBLE_A:
						shiftPost=3;
						break;
					}

					sc_uint<32> val_post;
					assert(fRegsManager->GetReg(fRnInd.read(),val_post));

					sc_uint<32> val_post2 = (fUimm11.read()) << shiftPost;
					assert((fUimm11.read() << shiftPost) == (printedImm << shiftPost));
					if(MAXLOOP_IT ==0 && fRegsManager->GetReg(fRnInd.read(),val_post) &&
							(  (fType.read() == POSTMOD_IMM) || (fType.read() == POSTMOD_REG && fRegsManager->GetReg(fRmInd.read(),val_post2)) ) ) {
						if( fSign.read() == ADD_OP ) {
							val_post = val_post + val_post2;
						} else {
							val_post = val_post - val_post2;
						}


						if(fType.read() ==POSTMOD_IMM) cerr << "LS_POSTMOD_IMM ADD/SUB _set" << endl;
						if(fType.read() == POSTMOD_REG) cerr << "LS_POSTMOD_REG ADD/SUB _set" << endl;
						fRegsManager->SetReg(fRnInd.read() ,  val_post);
					} else {
						fRegsManager->InvalidateReg(((fRnInd.read())));
					}

				}

				if(fAccessSize.read() == DOUBLE_A ) {
					fRdInd->keep_only(0,(NREGS-1)/2);
					fRdInd->keep_out((fRegsManager->GetMemPoitersRegs().first)/2, (fRegsManager->GetMemPoitersRegs().second)/2);

					if(fType.read() == POSTMOD_REG || fType.read() ==POSTMOD_IMM) {//prevent STRB R31,[R31]+R24;
						fRdInd->keep_out(fRnInd.read()/2);
					}
				} else {
					fRdInd->keep_only(0,NREGS-1);
					fRdInd->keep_out(fRegsManager->GetMemPoitersRegs().first, fRegsManager->GetMemPoitersRegs().second);

					if(fType.read() == POSTMOD_REG || fType.read() ==POSTMOD_IMM) {
						fRdInd->keep_out(fRnInd.read());
					}
				}

				fRdInd->next();

				//cerr << "LS: fRdInd->next()" << fRdInd.read() << endl;

				if(fLoS.read() == true && isSafeLoadOp.read()) {
					sc_uint<8> dataToMem = (sc_uint<32>(rand()))(7,0);

					scv_smart_ptr<bool> setMemToZero;
					setMemToZero.write(false);
					//in case of testset instruction the "interesting" mem value is zero
					if(isConvert2TestAndSet.read() ) {
						setMemToZero->next();
					}
					if(setMemToZero.read()) {
						dataToMem=0;
					}

					for ( unsigned m = 0 ; m < 8 ; m++) {
						//if(fAccessSize.read() == DOUBLE_A && m == 4)

						fMemManager->SetMem(fMemAddr+m,dataToMem);
					}


				}

				if(fAccessSize.read() == DOUBLE_A ) {
					fRdInd.write(fRdInd.read()*2);
				}

				if(fLoS.read() == true ) {//invalidate regs

					if( !isSafeLoadOp.read()) {
						fRegsManager->InvalidateReg(fRdInd.read());

						if(fAccessSize.read() == DOUBLE_A ) {
							fRegsManager->InvalidateReg(1+(fRdInd.read()));
						}

						cerr << "WR allowed" << endl;



					} else {
						//cerr << "NO WR allowed" << endl;

						sc_uint<32> reg_val;
						for ( unsigned m = 0 ; m < 4 ; m++) {
							if(fAccessSize.read() == BYTE_A && m == 1) break;
							if(fAccessSize.read() == HALF_A && m == 2) break;
							reg_val(m*8+7,m*8) = fMemManager->GetMem(fMemAddr+m);
						}

						fRegsManager->SetReg(fRdInd.read(),reg_val);
						if(isConvert2TestAndSet.read()  && fAccessSize.read() == WORD_A && fType.read() == REG_OFFSET&&
								fMemAddr>EXT_MEM_START && sc_uint<32>(chip_id)(7,4) == fMemAddr(31,28)  && sc_uint<32>(chip_id)(3,0) == fMemAddr(25,22)) {
							cerr << "TESTASET REG_OFFSET" << endl;
							isReplaceLoadByTestandSet=true;
							fRegsManager->InvalidateReg(fRdInd.read());
						}

						if(fAccessSize.read() == DOUBLE_A ) {

							for ( unsigned m = 0 ; m < 4 ; m++) {
								reg_val(m*8+7,m*8) = fMemManager->GetMem(fMemAddr+4+m);
							}

							fRegsManager->SetReg(1+(fRdInd.read()),reg_val);
						}
					}
				}
			}

			PrintMe(cerr);
		}

	}
};

#endif
