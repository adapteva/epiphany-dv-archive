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

#ifndef DATA_PROCESSING_INSTR_H
#define DATA_PROCESSING_INSTR_H

extern unsigned int MAXLOOP_IT;

enum EDPType {
	ADD_Reg , ADD_IMM3,  ADD_IMM11,
	SUB_Reg, SUB_IMM3 , SUB_IMM11 ,
	ASR_Reg, ASR_IMM,
	LSR_Reg, LSR_IMM,
	LSL_Reg, LSL_IMM ,
	ORR_,
	AND_,
	EOR_,

	BITR,

	FABS_,
	FIX_,
	FLOAT_,

	FADD_,
	FSUB_,
	FMUL_,

	FMADD_,
	FMSUB_//,

	//DV_MOV_LSL//fake instruction to raise probability for load store
};

template<>
class scv_extensions<EDPType> : public scv_enum_base<EDPType> {
public:

	SCV_ENUM_CTOR(EDPType) {
		SCV_ENUM(ADD_Reg);  SCV_ENUM(ADD_IMM3);    SCV_ENUM(ADD_IMM11);
		SCV_ENUM(SUB_Reg);  SCV_ENUM(SUB_IMM3);    SCV_ENUM(SUB_IMM11);
		SCV_ENUM(ASR_Reg);  SCV_ENUM(ASR_IMM);
		SCV_ENUM(LSR_Reg);  SCV_ENUM(LSR_IMM);
		SCV_ENUM(LSL_Reg);  SCV_ENUM(LSL_IMM);
		SCV_ENUM(ORR_);
		SCV_ENUM(AND_);
		SCV_ENUM(EOR_);

		SCV_ENUM(BITR);

		SCV_ENUM(FABS_);
		SCV_ENUM(FIX_);
		SCV_ENUM(FLOAT_);

		SCV_ENUM(FADD_);
		SCV_ENUM(FSUB_);
		SCV_ENUM(FMUL_);

		SCV_ENUM(FMADD_);
		SCV_ENUM(FMSUB_);

	//	SCV_ENUM(DV_MOV_LSL);
	}
};



class TDPIntr
:       public TInstr_If ,
public scv_constraint_base
{

	//implement the Instruction interface
public:

	virtual void PrintMe(::std::ostream& s= ::std::cout ) {


		switch (fDPtype.read()) {
		case ADD_Reg:
			s << "ADD";
			if(isForced32bitsOpcode) {
				s <<".l";
			}
			s << " R" << fRdInd.read() << ",R" << fRnInd.read() << ",R" << fRmInd.read() << ";" << endl;
			break;

		case ADD_IMM3:
			s << "ADD";
			if(isForced32bitsOpcode) {
				s <<".l";
			}
			s<< " R" << fRdInd.read() << ",R" << fRnInd.read() << ",#" << fSimm3.read() << ";" << endl;

			break;
		case ADD_IMM11:
			s << "ADD";
			if(isForced32bitsOpcode) {
				s <<".l";
			}
			s << " R" << fRdInd.read() << ",R" << fRnInd.read() << ",#" << fSimm11.read() << ";" << endl;

			break;

		case SUB_Reg:
			s << "SUB";
			if(isForced32bitsOpcode) {
				s <<".l";
			}
			s << " R" << fRdInd.read() << ",R" << fRnInd.read() << ",R" << fRmInd.read() << ";" << endl;
			break;

		case SUB_IMM3:
			s << "SUB";
			if(isForced32bitsOpcode) {
				s <<".l";
			}
			s<<" R" << fRdInd.read() << ",R" << fRnInd.read() << ",#" << fSimm3.read() << ";" << endl;
			break;

		case SUB_IMM11:
			s << "SUB";
			if(isForced32bitsOpcode) {
				s <<".l";
			}
			s<<" R" << fRdInd.read() << ",R" << fRnInd.read() << ",#" << fSimm11.read() << ";" << endl;
			break;

		case ASR_Reg:
			s << "ASR";
			if(isForced32bitsOpcode) {
				s <<".l";
			}
			s << " R" << fRdInd.read()  << ",R" << fRnInd.read()  << ",R" << fRmInd.read()  << ";" << endl;
			break;

		case ASR_IMM:
			s << "ASR";
			if(isForced32bitsOpcode) {
				s <<".l";
			}
			s << " R" << fRdInd.read() << ",R" << fRnInd.read() << ",#" << fUimm5.read() << ";" << endl;
			break;

		case LSR_Reg:
			s << "LSR";
			if(isForced32bitsOpcode) {
				s <<".l";
			}
			s<<" R" << fRdInd.read()  << ",R" << fRnInd.read()  << ",R" << fRmInd.read()  << ";" << endl;
			break;

		case LSR_IMM:
			s << "LSR";
			if(isForced32bitsOpcode) {
				s <<".l";
			}
			s<<" R" << fRdInd.read() << ",R" << fRnInd.read() << ",#" << fUimm5.read() << ";" << endl;
			break;

		case LSL_Reg:
			s << "LSL";
			if(isForced32bitsOpcode) {
				s <<".l";
			}
			s<<" R" << fRdInd.read()  << ",R" << fRnInd.read()  << ",R" << fRmInd.read()  << ";" << endl;
			break;

		case LSL_IMM:
			s << "LSL";
			if(isForced32bitsOpcode) {
				s <<".l";
			}
			s<<" R" << fRdInd.read() << ",R" << fRnInd.read() << ",#" << fUimm5.read() << ";" << endl;
			break;

		case ORR_:
			s << "ORR";
			if(isForced32bitsOpcode) {
				s <<".l";
			}
			s<<" R" << fRdInd.read() << ",R" << fRnInd.read() << ",R" << fRmInd.read() << ";" << endl;
			break;

		case AND_:
			s << "AND";
			if(isForced32bitsOpcode) {
				s <<".l";
			}
			s<<" R" << fRdInd.read() << ",R" << fRnInd.read() << ",R" << fRmInd.read() << ";" << endl;
			break;

		case EOR_:
			s << "EOR";
			if(isForced32bitsOpcode) {
				s <<".l";
			}
			s<<" R" << fRdInd.read() << ",R" << fRnInd.read() << ",R" << fRmInd.read() << ";" << endl;
			break;

		case BITR:
			s << "BITR";
			if(isForced32bitsOpcode) {
				s <<".l";
			}
			s<<" R" << fRdInd.read() << ",R" << fRnInd.read() << ";" << endl;
			break;



		case FADD_:
			s << "FADD";
			if(isForced32bitsOpcode) {
				s <<".l";
			}
			s<<" R" << fRdInd.read() << ",R" << fRnInd.read() << ",R" << fRmInd.read() << ";" << endl;
			break;

		case FSUB_:
			s << "FSUB";
			if(isForced32bitsOpcode) {
				s <<".l";
			}
			s<<" R" << fRdInd.read() << ",R" << fRnInd.read() << ",R" << fRmInd.read() << ";" << endl;
			break;

		case FMUL_:
			s << "FMUL";
			if(isForced32bitsOpcode) {
				s <<".l";
			}
			s<<" R" << fRdInd.read() << ",R" << fRnInd.read() << ",R" << fRmInd.read() << ";" << endl;
			break;


		case FABS_:
			s << "FABS";
			if(isForced32bitsOpcode) {
				s <<".l";
			}
			s<<" R" << fRdInd.read() << ",R" << fRnInd.read()  << ";" << endl;
			break;

		case FIX_:
			s << "FIX";
			if(isForced32bitsOpcode) {
				s <<".l";
			}
			s<<" R" << fRdInd.read() << ",R" << fRnInd.read()  << ";" << endl;
			break;

		case FLOAT_:
			s << "FLOAT";
			if(isForced32bitsOpcode) {
				s <<".l";
			}
			s<<" R" << fRdInd.read() << ",R" << fRnInd.read()  << ";" << endl;
			break;


		case FMADD_:
			s << "FMADD";
			if(isForced32bitsOpcode) {
				s <<".l";
			}
			s<<" R" << fRdInd.read() << ",R" << fRnInd.read() << ",R" << fRmInd.read() << ";" << endl;
			break;

		case FMSUB_:
			s << "FMSUB";
			if(isForced32bitsOpcode) {
				s <<".l";
			}
			s<<" R" << fRdInd.read() << ",R" << fRnInd.read() << ",R" << fRmInd.read() << ";" << endl;
			break;

		}
	}


	//contraint part
public:

	scv_smart_ptr<EDPType> fDPtype;
	//scv_bag<EDPType> fDPtypeDist;//not used


	scv_smart_ptr<unsigned> fRdInd;
	scv_smart_ptr<unsigned> fRmInd;
	scv_smart_ptr<unsigned> fRnInd;
	scv_smart_ptr<sc_int<11> > fSimm11;
	scv_smart_ptr<sc_int<3> > fSimm3;
	scv_smart_ptr<sc_uint<5> > fUimm5;

	//scv_smart_ptr<sc_uint<16> > fUimm16;

public:
	SCV_CONSTRAINT_CTOR(TDPIntr) {


		SCV_CONSTRAINT(fDPtype() != FADD_ || fRdInd() != 14 );
		SCV_CONSTRAINT(fDPtype() != FSUB_ || fRdInd() != 14 );
		SCV_CONSTRAINT(fDPtype() != FMUL_ || fRdInd() != 14 );
		SCV_CONSTRAINT(fDPtype() != FADD_ || fRdInd() != 14 );
		SCV_CONSTRAINT(fDPtype() != FMADD_ || fRdInd() != 14 );
		SCV_CONSTRAINT(fDPtype() != FMSUB_ || fRdInd() != 14 );
		SCV_CONSTRAINT(fDPtype() != FABS_ || fRdInd() != 14 );
		SCV_CONSTRAINT(fDPtype() != FIX_ || fRdInd() != 14 );
		SCV_CONSTRAINT(fDPtype() != FLOAT_ || fRdInd() != 14 );

		if(!isGenFP) {
			SCV_CONSTRAINT(fDPtype() != FADD_);
			SCV_CONSTRAINT(fDPtype() != FSUB_);
			SCV_CONSTRAINT(fDPtype() != FMUL_);
			SCV_CONSTRAINT(fDPtype() != FMADD_);
			SCV_CONSTRAINT(fDPtype() != FMSUB_);

			SCV_CONSTRAINT(fDPtype() != FABS_);
			SCV_CONSTRAINT(fDPtype() != FIX_);
			SCV_CONSTRAINT(fDPtype() != FLOAT_);
		}

		if(isIntegerModeOn==true) {
			SCV_CONSTRAINT(fDPtype() != FABS_);
			SCV_CONSTRAINT(fDPtype() != FIX_);
			SCV_CONSTRAINT(fDPtype() != FLOAT_);
		}

	//	SCV_SOFT_CONSTRAINT(fDPtype() == ASR_Reg);


//		SCV_CONSTRAINT(fRdInd() < NREGS);
		SCV_CONSTRAINT(fRmInd() < NREGS);
		SCV_CONSTRAINT(fRnInd() < NREGS);

		//TODO !!!!
//		SCV_CONSTRAINT(fDPtype() != LSR_Reg);
//		SCV_CONSTRAINT(fDPtype() != LSL_Reg);


		/*
		fDPtypeDist.add(ADD_Reg , 10);
		fDPtypeDist.add(ADD_IMM3,10);
		fDPtypeDist.add(ADD_IMM11,10);

		fDPtypeDist.add(SUB_Reg,10);
		fDPtypeDist.add(SUB_IMM3 , 10);
		fDPtypeDist.add(SUB_IMM11 ,10);

		fDPtypeDist.add(ASR_Reg, 10);
		fDPtypeDist.add(ASR_IMM,10);

		fDPtypeDist.add(LSR_Reg, 10);
		fDPtypeDist.add(LSR_IMM,10);

		fDPtypeDist.add(LSL_Reg,10);
		fDPtypeDist.add(LSL_IMM ,10);

		fDPtypeDist.add(ORR_,10);

		fDPtypeDist.add(AND_,10);

		fDPtypeDist.add(EOR_,10);

		fDPtypeDist.add(FADD_,10);
		fDPtypeDist.add(FSUB_,10);
		fDPtypeDist.add(FMUL_,10);
		fDPtypeDist.add(FMADD_,10);
		fDPtypeDist.add(FMSUB_,10);

		fDPtypeDist.add(DV_MOV_LSL	,100);

		fDPtype->set_mode(fDPtypeDist);
*/

	}


public:
	virtual void GenerateMe() {
		this->next();
	}

	void next() {

		if( fRegsManager ) {

			fRdInd->keep_only(0,NREGS-1);
			fRdInd->keep_out(fRegsManager->GetMemPoitersRegs().first, fRegsManager->GetMemPoitersRegs().second);

		}

		scv_constraint_base::next();








		if( fRegsManager ) { //TODO bind reg to static object --- !!!

			bool isToInvalidate = true;
			fRegsManager->InvalidateAllFlags();


			if((fDPtype.read() == EOR_ ) && ( MAXLOOP_IT ==0 || (fRdInd.read() != fRnInd.read()) && (fRdInd.read() != fRmInd.read()))) {
				sc_uint<32> val1,val2;
				if( fRegsManager->GetReg(fRnInd.read(),val1) && fRegsManager->GetReg(fRmInd.read(),val2)) {
					fRegsManager->SetReg(fRdInd.read(),(val1^val2));
					cerr << "XOR _set" << endl;
					isToInvalidate = false;


					fRegsManager->SetAZ(sc_uint<32>(val1^val2)==0);
					fRegsManager->SetAN(sc_uint<32>(val1^val2)[31]);

					fRegsManager->SetAC(0);
					fRegsManager->SetAV(0);
				}
			}
			if((fDPtype.read() == ORR_ ) && ( MAXLOOP_IT ==0 ||(fRdInd.read() != fRnInd.read()) && (fRdInd.read() != fRmInd.read()))) {
				sc_uint<32> val1,val2;
				if( fRegsManager->GetReg(fRnInd.read(),val1) && fRegsManager->GetReg(fRmInd.read(),val2)) {
					fRegsManager->SetReg(fRdInd.read(),(val1|val2));
					cerr << "ORR _set" << endl;
					isToInvalidate = false;


					fRegsManager->SetAZ(sc_uint<32>(val1|val2)==0);
					fRegsManager->SetAN(sc_uint<32>(val1|val2)[31]);

					fRegsManager->SetAC(0);
					fRegsManager->SetAV(0);
				}
			}

			if((fDPtype.read() == AND_ ) && ( MAXLOOP_IT ==0 || (fRdInd.read() != fRnInd.read()) && (fRdInd.read() != fRmInd.read()))) {
				sc_uint<32> val1,val2;
				if( fRegsManager->GetReg(fRnInd.read(),val1) && fRegsManager->GetReg(fRmInd.read(),val2)) {
					fRegsManager->SetReg(fRdInd.read(),(val1&val2));
					cerr << "AND _set" << endl;
					isToInvalidate = false;


					fRegsManager->SetAZ(sc_uint<32>(val1&val2)==0);
					fRegsManager->SetAN(sc_uint<32>(val1&val2)[31]);

					fRegsManager->SetAC(0);
					fRegsManager->SetAV(0);

				}
			}
			if((fDPtype.read() == ADD_Reg ) && ( MAXLOOP_IT ==0 ||(fRdInd.read() != fRnInd.read()) && (fRdInd.read() != fRmInd.read()))) {
				sc_uint<32> val1,val2;
				if( fRegsManager->GetReg(fRnInd.read(),val1) && fRegsManager->GetReg(fRmInd.read(),val2)) {
					fRegsManager->SetReg(fRdInd.read(),(val1+val2));
					cerr << "ADD _set" << endl;
					isToInvalidate = false;


					fRegsManager->SetAZ(sc_uint<32>(val1+val2)==0);
					fRegsManager->SetAN(sc_uint<32>(val1+val2)[31]);
				}
			}
			if((fDPtype.read() == ADD_IMM3 ) && ( MAXLOOP_IT ==0 ||(fRdInd.read() != fRnInd.read()))) {
				sc_uint<32> val1;
				if( fRegsManager->GetReg(fRnInd.read(),val1) ) {
					fRegsManager->SetReg(fRdInd.read(),(val1+(fSimm3.read())));
					cerr << "ADD_imm3 _set" << endl;
					isToInvalidate = false;



					fRegsManager->SetAZ(sc_uint<32>(val1+(fSimm3.read()))== 0);
					fRegsManager->SetAN(sc_uint<32>(val1+(fSimm3.read()))[31]);
				}
			}
			if((fDPtype.read() == ADD_IMM11 ) && ( MAXLOOP_IT ==0 ||(fRdInd.read() != fRnInd.read()))) {
				sc_uint<32> val1;
				if( fRegsManager->GetReg(fRnInd.read(),val1) ) {
					fRegsManager->SetReg(fRdInd.read(),(val1+(fSimm11.read())));
					cerr << "ADD_imm11 _set" << endl;
					isToInvalidate = false;


					fRegsManager->SetAZ(sc_uint<32>(val1+(fSimm11.read()))==0);
					fRegsManager->SetAN(sc_uint<32>(val1+(fSimm11.read()))[31]);
				}
			}
			if((fDPtype.read() == SUB_Reg ) && ( MAXLOOP_IT ==0 ||(fRdInd.read() != fRnInd.read()) && (fRdInd.read() != fRmInd.read()))) {
				sc_uint<32> val1,val2;
				if( fRegsManager->GetReg(fRnInd.read(),val1) && fRegsManager->GetReg(fRmInd.read(),val2)) {
					fRegsManager->SetReg(fRdInd.read(),(val1-val2));
					cerr << "SUB _set" << endl;
					isToInvalidate = false;


					fRegsManager->SetAZ(sc_uint<32>(val1-val2)==0);
					fRegsManager->SetAN(sc_uint<32>(val1-val2)[31]);
				}
			}
			if((fDPtype.read() == SUB_IMM3 ) && ( MAXLOOP_IT ==0 ||(fRdInd.read() != fRnInd.read()))) {
				sc_uint<32> val1;
				if( fRegsManager->GetReg(fRnInd.read(),val1) ) {
					fRegsManager->SetReg(fRdInd.read(),(val1-(fSimm3.read())));
					cerr << "SUB_imm3 _set" << endl;
					isToInvalidate = false;


					fRegsManager->SetAZ(sc_uint<32>(val1-(fSimm3.read()))==0);
					fRegsManager->SetAN(sc_uint<32>(val1-(fSimm3.read()))[31]);
				}
			}
			if((fDPtype.read() == SUB_IMM11 ) && ( MAXLOOP_IT ==0 ||(fRdInd.read() != fRnInd.read()))) {
				sc_uint<32> val1;
				if( fRegsManager->GetReg(fRnInd.read(),val1) ) {
					fRegsManager->SetReg(fRdInd.read(),(val1-(fSimm11.read())));
					cerr << "SUB_imm11 _set" << endl;
					isToInvalidate = false;


					fRegsManager->SetAZ(sc_uint<32>(val1-(fSimm11.read()))==0);
					fRegsManager->SetAN(sc_uint<32>(val1-(fSimm11.read()))[31]);
				}
			}
			if((fDPtype.read() == LSR_Reg ) && ( MAXLOOP_IT ==0 ||(fRdInd.read() != fRnInd.read()) && (fRdInd.read() != fRmInd.read()))) {
				sc_uint<32> val1,val2;
				if( fRegsManager->GetReg(fRnInd.read(),val1) && fRegsManager->GetReg(fRmInd.read(),val2)) {
					fRegsManager->SetReg(fRdInd.read(),(val1>>(val2(4,0))));
					cerr << "LSR_r _set" << endl;
					isToInvalidate = false;


					fRegsManager->SetAZ(sc_uint<32>(val1>>(val2(4,0)))==0);
					fRegsManager->SetAN(sc_uint<32>(val1>>(val2(4,0)))[31]);

					fRegsManager->SetAC(0);
					fRegsManager->SetAV(0);

				}
			}
			if((fDPtype.read() == LSL_Reg ) && ( MAXLOOP_IT ==0 ||(fRdInd.read() != fRnInd.read()) && (fRdInd.read() != fRmInd.read()))) {
				sc_uint<32> val1,val2;
				if( fRegsManager->GetReg(fRnInd.read(),val1) && fRegsManager->GetReg(fRmInd.read(),val2)) {
					fRegsManager->SetReg(fRdInd.read(),(val1<<val2(4,0)));
					cerr << "LSL_r _set" << endl;
					isToInvalidate = false;


					fRegsManager->SetAZ(sc_uint<32>(val1<<val2(4,0))==0);
					fRegsManager->SetAN(sc_uint<32>(val1<<val2(4,0))[31]);


					fRegsManager->SetAC(0);
					fRegsManager->SetAV(0);

				}
			}
			if((fDPtype.read() == ASR_Reg ) && ( MAXLOOP_IT ==0 ||(fRdInd.read() != fRnInd.read()) && (fRdInd.read() != fRmInd.read()))) {
				sc_uint<32> val1,val2;

				if( fRegsManager->GetReg(fRnInd.read(),val1) && fRegsManager->GetReg(fRmInd.read(),val2)) {

					sc_uint<32>res_val=(val1>>val2(4,0));
					for(unsigned n=0;n<val2(4,0);n++) {
						res_val[31-n]=val1[31];
					}
					fRegsManager->SetReg(fRdInd.read(),res_val);
					cerr << "ASR_r _set" << endl;
					isToInvalidate = false;


					fRegsManager->SetAZ(res_val==0);
					fRegsManager->SetAN(res_val[31]);


					fRegsManager->SetAC(0);
					fRegsManager->SetAV(0);

				}
			}

			if((fDPtype.read() == ASR_IMM ) && ( MAXLOOP_IT ==0 ||(fRdInd.read() != fRnInd.read()))) {
				sc_uint<32> val1;

				if( fRegsManager->GetReg(fRnInd.read(),val1) ) {

					sc_uint<32>res_val=(val1>>(fUimm5.read()));
					for(unsigned n=0;n<(fUimm5.read());n++) {
						res_val[31-n]=val1[31];
					}
					fRegsManager->SetReg(fRdInd.read(),res_val);
					cerr << "ASR_imm _set" << endl;
					isToInvalidate = false;


					fRegsManager->SetAZ(res_val==0);
					fRegsManager->SetAN(res_val[31]);

					fRegsManager->SetAC(0);
					fRegsManager->SetAV(0);

				}
			}


			if((fDPtype.read() == LSR_IMM ) && ( MAXLOOP_IT ==0 ||(fRdInd.read() != fRnInd.read()))) {
				sc_uint<32> val1;
				if( fRegsManager->GetReg(fRnInd.read(),val1) ) {
					fRegsManager->SetReg(fRdInd.read(),(val1>>(fUimm5.read())));
					cerr << "LSR_imm _set" << endl;
					isToInvalidate = false;


					fRegsManager->SetAZ(sc_uint<32>(val1>>(fUimm5.read()))==0);
					fRegsManager->SetAN(sc_uint<32>(val1>>(fUimm5.read()))[31]);


					fRegsManager->SetAC(0);
					fRegsManager->SetAV(0);

				}
			}

			if((fDPtype.read() == LSL_IMM ) && ( MAXLOOP_IT ==0 ||(fRdInd.read() != fRnInd.read()))) {
				sc_uint<32> val1;
				if( fRegsManager->GetReg(fRnInd.read(),val1) ) {
					fRegsManager->SetReg(fRdInd.read(),(val1<<(fUimm5.read())));
					cerr << "LSL_imm _set" << endl;
					isToInvalidate = false;


					fRegsManager->SetAZ(sc_uint<32>(val1<<(fUimm5.read()))==0);
					fRegsManager->SetAN(sc_uint<32> (val1<<(fUimm5.read()))[31]);


					fRegsManager->SetAC(0);
					fRegsManager->SetAV(0);

				}
			}

			if((fDPtype.read() == BITR ) && ( MAXLOOP_IT ==0 ||(fRdInd.read() != fRnInd.read()))) {
				sc_uint<32> val1;
				if( fRegsManager->GetReg(fRnInd.read(),val1) ) {
					for( unsigned i = 0 ; i < 16 ; i++ ) {
						bool tmp = val1.operator [](i);
						val1[i] = val1[31 -i];
						val1[31 -i] = tmp;
					}
					fRegsManager->SetReg(fRdInd.read(),(val1));
					cerr << "BITR _set" << endl;
					isToInvalidate = false;


					fRegsManager->SetAZ((val1) ==0);
					fRegsManager->SetAN((val1) [31]);

					fRegsManager->SetAC(0);
					fRegsManager->SetAV(0);

				}
			}

			if(isToInvalidate  ) {
				fRegsManager->InvalidateReg(fRdInd.read());
			}

			PrintMe(cerr);

		}

	}

};


#endif
