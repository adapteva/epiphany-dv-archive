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

#ifndef FP_MACRO_INSTR_H_
#define FP_MACRO_INSTR_H_


/*
 *
 *
 *
 *
 */
enum E_Fp_NumberType {

	NAN_,
	Infinity_,
	Normal_,
	Denormal_,
	Zero_,

};


template<>
class scv_extensions<E_Fp_NumberType> : public scv_enum_base<E_Fp_NumberType> {
public:

	SCV_ENUM_CTOR(E_Fp_NumberType) {

		SCV_ENUM(NAN_);
		SCV_ENUM(Infinity_);
		SCV_ENUM(Normal_);
		SCV_ENUM(Denormal_);
		SCV_ENUM(Zero_);
	}
};

/*SIGN
EXP[7:0]
MANTISSA[22:0]
 */


class TFP_Base : public scv_constraint_base  {
protected :
	scv_smart_ptr<sc_uint<1>  > sign;
	scv_smart_ptr<sc_uint<8>  >exp;
	scv_smart_ptr<sc_uint<23> > mantisa;
public:

	SCV_CONSTRAINT_CTOR(TFP_Base) {

	}
public:
	sc_uint<32> GetFpNumber() {
		sc_uint<32> res;
		res(22,0) = mantisa.read();
		res(30,23) = exp.read();
		res(31,31) = sign.read();
		return res;
	}
};

/*NAN X 255 Nonzero Undefined */
class  TNan_FP
:       public TFP_Base {

public:
	SCV_CONSTRAINT_CTOR(TNan_FP) {

		//use the base constraint
		SCV_BASE_CONSTRAINT(TFP_Base);
		SCV_CONSTRAINT( exp() == 255 && mantisa() != 0);
	}
};


/*Infinity S 255 Zero (-1)S Infinity*/
class  TInf_FP
:       public TFP_Base  {

public:
	SCV_CONSTRAINT_CTOR(TInf_FP) {

		//use the base constraint
		SCV_BASE_CONSTRAINT(TFP_Base);

		SCV_CONSTRAINT( exp() == 255 && mantisa() == 0);
	}
};

/*Denormal S 0 Any (-1)S Zero  */
class  TDenormal_FP
:       public TFP_Base  {

public:
	SCV_CONSTRAINT_CTOR(TDenormal_FP) {

		//use the base constraint
		SCV_BASE_CONSTRAINT(TFP_Base);

		SCV_CONSTRAINT( exp() == 0 && mantisa() != 0);
	}
};

class  TZero_FP
:       public TFP_Base  {

public:
	SCV_CONSTRAINT_CTOR(TZero_FP) {

		//use the base constraint
		SCV_BASE_CONSTRAINT(TFP_Base);

		SCV_CONSTRAINT( exp() == 0 && mantisa() == 0);
	}
};
/*S 1<=e<=254 Any */

class  TNormal_FP
:       public TFP_Base         {

private :
	scv_bag<pair <sc_uint<8>, sc_uint<8> > > fNormalNumberDist;


public:
	SCV_CONSTRAINT_CTOR(TNormal_FP) {

		fNormalNumberDist.add( pair<sc_uint<8>,sc_uint<8> >(1,5),  20);
		fNormalNumberDist.add( pair<sc_uint<8>,sc_uint<8> >(6,15),  20);
		fNormalNumberDist.add( pair<sc_uint<8>,sc_uint<8> >(16,239),  20);
		fNormalNumberDist.add( pair<sc_uint<8>,sc_uint<8> >(240,249),  20);
		fNormalNumberDist.add( pair<sc_uint<8>,sc_uint<8> >(250,254),  20);

		exp->set_mode(fNormalNumberDist);

		//use the base constraint
		SCV_BASE_CONSTRAINT(TFP_Base);

		SCV_CONSTRAINT( exp() > 0 &&  exp() < 255 );
	}
};



//template <int NREGS>
class TFPMegaInstr
:       public TInstr_If
        {
	static const unsigned NUM_INSTR_IN_MACRO_PATTERN = 5;
	//implement the Instruction interface
public:


	TFP_Base *CreateFpNumber(scv_smart_ptr<E_Fp_NumberType>&opType) {
		opType->next();

		TFP_Base *res=0;

		switch (opType.read()) {
		case NAN_:
			res=new TNan_FP("nan");
			cerr << " nan " << endl;
			break;
		case Infinity_:
			res=new TInf_FP("inf");
			cerr << "inf " << endl;
			break;

		case Normal_:
			res=new TNormal_FP("normal");
			cerr << " normal " << endl;
			break;

		case Denormal_:
			res=new TDenormal_FP("denormal");
			cerr << " denormal " << endl;
			break;

		case Zero_:
			res=new TZero_FP("zero");
			cerr << " zero " << endl;
			break;
		}

		res->next();

		return res;
	}

	virtual sc_uint<32> GetInstructionSize() {
		assert(fMovRegIntsrRd);

		assert(fMovTRegIntsrRd);


		assert(fMovRegIntsrRm);

		assert(fMovTRegIntsrRm);


		assert(fMovRegIntsrRn);

		assert(fMovTRegIntsrRn);


		assert(fpInstr);


		assert(fLSinstr);

		return

		2 + /*movfs r1, status*/
		fMovRegIntsrRd->GetInstructionSize()+
		fMovTRegIntsrRd->GetInstructionSize()+
		fMovRegIntsrRm->GetInstructionSize()+
		fMovTRegIntsrRm->GetInstructionSize()+
		fMovRegIntsrRn->GetInstructionSize()+
		fMovTRegIntsrRn->GetInstructionSize()+
		fpInstr->GetInstructionSize()+
		fLSinstr->GetInstructionSize();

	}

	virtual void PrintMe(::std::ostream& s= ::std::cout ) {

		if(fDPtype.read() == FMADD_ || fDPtype.read() == FMSUB_) {
			assert(fMovRegIntsrRd);
			fMovRegIntsrRd->PrintMe(s);
			assert(fMovTRegIntsrRd);
			fMovTRegIntsrRd->PrintMe(s);
		}

		assert(fMovRegIntsrRm);
		fMovRegIntsrRm->PrintMe(s);
		assert(fMovTRegIntsrRm);
		fMovTRegIntsrRm->PrintMe(s);

		assert(fMovRegIntsrRn);
		fMovRegIntsrRn->PrintMe(s);
		assert(fMovTRegIntsrRn);
		fMovTRegIntsrRn->PrintMe(s);

		assert(fpInstr);
		fpInstr->PrintMe(s);

		//TODO Floating point
		if(isIntegerModeOn==true) {
			s << "movfs r1, status" << endl;
		}

		assert(fLSinstr);
		fLSinstr->PrintMe(s);


	}


	//Constraint part
public:
	scv_smart_ptr<E_Fp_NumberType> op1Type;
	scv_smart_ptr<E_Fp_NumberType> op2Type;
	scv_smart_ptr<E_Fp_NumberType> op3Type;

	scv_bag<E_Fp_NumberType> opTypeDist;


	scv_smart_ptr<EDPType> fDPtype;
	scv_bag<EDPType> fDPtypeDist;


	TMoveRegRegular *fMovRegIntsrRd;
	TMoveRegRegular *fMovRegIntsrRm;
	TMoveRegRegular *fMovRegIntsrRn;

	TMoveRegRegular *fMovTRegIntsrRd;
	TMoveRegRegular *fMovTRegIntsrRm;
	TMoveRegRegular *fMovTRegIntsrRn;
	TDPIntr *fpInstr;
	TLSInstr *fLSinstr;


public:

	void SetConstraints() {
		if(isIntegerModeOn==true) {
			fDPtype->keep_out(FABS_);
			fDPtype->keep_out(FIX_);
			fDPtype->keep_out(FLOAT_);

		} else {
			fDPtypeDist.add(FABS_,10);
			fDPtypeDist.add(FIX_,200);
			fDPtypeDist.add(FLOAT_,200);
		}

		fDPtypeDist.add(FADD_,100);
		fDPtypeDist.add(FSUB_,100);
		fDPtypeDist.add(FMUL_,100);
		fDPtypeDist.add(FMADD_,500);
		fDPtypeDist.add(FMSUB_,500);

		fDPtype->set_mode(fDPtypeDist);

		opTypeDist.add(NAN_,2);
		opTypeDist.add(Infinity_,1);
		opTypeDist.add(Normal_,95);
		opTypeDist.add(Denormal_,1);
		opTypeDist.add(Zero_,1);


		op1Type->set_mode(opTypeDist);
		op2Type->set_mode(opTypeDist);
		op3Type->set_mode(opTypeDist);

	}

	TFPMegaInstr(const char*) {

		fpInstr=0;
		fLSinstr=0;
		fMovRegIntsrRd=0;
		fMovRegIntsrRm=0;
		fMovRegIntsrRn=0;
		fMovTRegIntsrRd=0;
		fMovTRegIntsrRm=0;
		fMovTRegIntsrRn=0;


		SetConstraints() ;

	}
	TFPMegaInstr() {

		fpInstr=0;
		fLSinstr=0;
		fMovRegIntsrRd=0;
		fMovRegIntsrRm=0;
		fMovRegIntsrRn=0;
		fMovTRegIntsrRd=0;
		fMovTRegIntsrRm=0;
		fMovTRegIntsrRn=0;
		SetConstraints() ;



	}
	~TFPMegaInstr() {
		delete fpInstr;

	}

public:

	virtual void GenerateMe() {
		this->next();
	}


	//overwrite the next method -- just update the dest reg in RF

	void next() {


		if( fRegsManager && fMemManager) {


			TFP_Base *op1=CreateFpNumber(op1Type);
			TFP_Base *op2=CreateFpNumber(op2Type);
			TFP_Base *op3=CreateFpNumber(op3Type);

			cerr << " NUMBER op1 " << hex << op1->GetFpNumber() << endl;
			cerr << " NUMBER op2 " << hex << op2->GetFpNumber() << endl;
			cerr << " NUMBER op3 " << hex << op3->GetFpNumber() << endl;

			fDPtype->next();


			//PREPARE REGS
			if(fMovRegIntsrRd)delete fMovRegIntsrRd;//clean up from prev iteration

			fMovRegIntsrRd= new TMoveRegRegular("movetoregforfpD");
			fMovRegIntsrRd->BindRegsManager(fRegsManager);
			fMovRegIntsrRd->BindMemManager(fMemManager);
			fMovRegIntsrRd->fRMovetype->keep_only(MOV_IMM16);
			//place result always in R0
			fMovRegIntsrRd->fRdInd->keep_only(0);

			//if(fDPtype.read() == FMADD_ || fDPtype.read() == FMSUB_)

			fMovRegIntsrRd->GenerateMe();
			assert(fMovRegIntsrRd->GetInstructionSize());
			fMovRegIntsrRd->fUimm16.write(op1->GetFpNumber()(15,0));
			fRegsManager->InvalidateReg(fMovRegIntsrRd->fRdInd.read());

			if(fMovTRegIntsrRd)delete fMovTRegIntsrRd;//clean up from prev iteration

			fMovTRegIntsrRd= new TMoveRegRegular("movetoregforfpD");
			fMovTRegIntsrRd->BindRegsManager(fRegsManager);
			fMovTRegIntsrRd->BindMemManager(fMemManager);
			fMovTRegIntsrRd->fRMovetype->keep_only(MOVT_IMM16);
			fMovTRegIntsrRd->fRdInd->keep_only(fMovRegIntsrRd->fRdInd.read());

			//if(fDPtype.read() == FMADD_ || fDPtype.read() == FMSUB_)

			fMovTRegIntsrRd->GenerateMe();
			assert(fMovTRegIntsrRd->GetInstructionSize());
			fMovTRegIntsrRd->fUimm16.write(op1->GetFpNumber()(31,16));
			fRegsManager->InvalidateReg(fMovTRegIntsrRd->fRdInd.read());

			if(fMovRegIntsrRm)delete fMovRegIntsrRm;//clean up from prev iteration

			fMovRegIntsrRm= new TMoveRegRegular("movetoregforfpM");
			fMovRegIntsrRm->BindRegsManager(fRegsManager);
			fMovRegIntsrRm->BindMemManager(fMemManager);
			fMovRegIntsrRm->fRMovetype->keep_only(MOV_IMM16);

			fMovRegIntsrRm->GenerateMe();
			assert(fMovRegIntsrRm->GetInstructionSize());
			fMovRegIntsrRm->fUimm16.write(op2->GetFpNumber()(15,0));
			fRegsManager->InvalidateReg(fMovRegIntsrRm->fRdInd.read());

			if(fMovTRegIntsrRm)delete fMovTRegIntsrRm;//clean up from prev iteration

			fMovTRegIntsrRm= new TMoveRegRegular("movetoregforfpM");
			fMovTRegIntsrRm->BindRegsManager(fRegsManager);
			fMovTRegIntsrRm->BindMemManager(fMemManager);
			fMovTRegIntsrRm->fRMovetype->keep_only(MOVT_IMM16);
			fMovTRegIntsrRm->fRdInd->keep_only(fMovRegIntsrRm->fRdInd.read());

			fMovTRegIntsrRm->GenerateMe();
			assert(fMovTRegIntsrRm->GetInstructionSize());
			fMovTRegIntsrRm->fUimm16.write(op2->GetFpNumber()(31,16));
			fRegsManager->InvalidateReg(fMovTRegIntsrRm->fRdInd.read());


			if(fMovRegIntsrRn)delete fMovRegIntsrRn;//clean up from prev iteration

			fMovRegIntsrRn= new TMoveRegRegular("movetoregforfpN");
			fMovRegIntsrRn->BindRegsManager(fRegsManager);
			fMovRegIntsrRn->BindMemManager(fMemManager);
			fMovRegIntsrRn->fRMovetype->keep_only(MOV_IMM16);



			//			fMovRegIntsrRn->fRdInd->keep_out(fLSinstr->fRmInd.read());
			//			fMovRegIntsrRn->fRdInd->keep_out(fLSinstr->fRnInd.read());

			fMovRegIntsrRn->GenerateMe();
			assert(fMovRegIntsrRn->GetInstructionSize());
			fMovRegIntsrRn->fUimm16.write(op3->GetFpNumber()(15,0));
			//			fRegsManager->InvalidateReg(fMovRegIntsrRn->fRdInd.read());



			if(fMovTRegIntsrRn)delete fMovTRegIntsrRn;//clean up from prev iteration

			fMovTRegIntsrRn= new TMoveRegRegular("movetoregforfpN");
			fMovTRegIntsrRn->BindRegsManager(fRegsManager);
			fMovTRegIntsrRn->BindMemManager(fMemManager);
			fMovTRegIntsrRn->fRMovetype->keep_only(MOVT_IMM16);
			fMovTRegIntsrRn->fRdInd->keep_only(fMovRegIntsrRn->fRdInd.read());

			fMovTRegIntsrRn->GenerateMe();
			assert(fMovTRegIntsrRn->GetInstructionSize());
			fMovTRegIntsrRn->fUimm16.write(op3->GetFpNumber()(31,16));
			//			fRegsManager->InvalidateReg(fMovTRegIntsrRn->fRdInd.read());





			//FLOATING POINT
			if(fpInstr)delete fpInstr;//clean up from prev iteration

			fpInstr=new TDPIntr("fp");
			fpInstr->BindRegsManager(fRegsManager);
			fpInstr->BindMemManager(fMemManager);

			fpInstr->fDPtype->keep_only(fDPtype.read());
			fpInstr->fRmInd->keep_only(fMovRegIntsrRm->fRdInd.read());
			fpInstr->fRnInd->keep_only(fMovRegIntsrRn->fRdInd.read());

			//if(fDPtype.read() == FMADD_ || fDPtype.read() == FMSUB_)

			fpInstr->fRdInd->keep_only(fMovRegIntsrRd->fRdInd.read());

			//fpInstr->fRdInd->keep_only(fLSinstr->fRdInd.read());

			fpInstr->GenerateMe();
			assert(fpInstr->GetInstructionSize());


			//load store
			if(fLSinstr)delete fLSinstr;

			fLSinstr = new TLSInstr("LSinstr");
			fLSinstr->BindRegsManager(fRegsManager);
			fLSinstr->BindMemManager(fMemManager);

			fLSinstr->fLoS->keep_only(false);//store
			fLSinstr->fRdInd->keep_only(fpInstr->fRdInd.read());
			//fLSinstr->fRdInd->keep_only(0);
			fLSinstr->GenerateMe();
			assert(fLSinstr->GetInstructionSize());
			assert( fLSinstr->fAccessSize.read()==DOUBLE_A);


			delete op1;
			delete op2;
			delete op3;

		}




	}

        };



#endif /* FP_MACRO_INSTR_H_ */
