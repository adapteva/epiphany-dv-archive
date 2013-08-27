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
#ifndef REG_MOVE_REGULAR_H
#define REG_MOVE_REGULAR_H

enum ESpecialRegRead {
	READ_CONFIG,
	READ_IMASK
};
template<>
class scv_extensions<ESpecialRegRead> : virtual public scv_enum_base<ESpecialRegRead> {
public:

	SCV_ENUM_CTOR(ESpecialRegRead) {

		SCV_ENUM(READ_CONFIG);
		SCV_ENUM(READ_IMASK);

	}
};


enum ESpecialInstr {
	NOP,
	WAND,
	BKPT,
	MBKPT
};

template<>
class scv_extensions<ESpecialInstr> : virtual public scv_enum_base<ESpecialInstr> {
public:

	SCV_ENUM_CTOR(ESpecialInstr) {

		SCV_ENUM(NOP);
		SCV_ENUM(WAND);
		SCV_ENUM(BKPT);
		SCV_ENUM(MBKPT);
	}
};



enum ERegMovType {
	MOV_STATUS,
	MOV_IMM8,
	MOV_IMM16,
	MOVT_IMM16,
	MOV_REG,
	SPECIAL_I,
	READ_SPECIAL_REGS,
	WRITE_SPECIAL_REGS

};
template<>
class scv_extensions<ERegMovType> : virtual public scv_enum_base<ERegMovType> {
public:

	SCV_ENUM_CTOR(ERegMovType) {

		SCV_ENUM(MOV_STATUS);
		SCV_ENUM(MOV_IMM8);
		SCV_ENUM(MOV_IMM16);
		SCV_ENUM(MOVT_IMM16);
		SCV_ENUM(MOV_REG);

		SCV_ENUM(SPECIAL_I);

		SCV_ENUM(READ_SPECIAL_REGS);
		SCV_ENUM(WRITE_SPECIAL_REGS);
	}
};


//template <int NREGS>
class TMoveRegRegular
:       public TInstr_If,
public scv_constraint_base
{
	//implement the Instruction interface
public:


	virtual void PrintMe(::std::ostream& s= ::std::cout ) {
		switch (fRMovetype.read()) {

		case SPECIAL_I:

			if(fSpecialInstrSel.read() == NOP) {
				s << "nop" ;
				if(isHwLoopOn) {
					s <<".l";
				}
				s << endl;
			}
			if(fSpecialInstrSel.read() == WAND) {
				s << "wand" << endl;
			}

			if(fSpecialInstrSel.read() == BKPT) {
				assert(isBkptOn);
				//the simulator will stop on bkpt
				s << "dvbkpt" << endl;
			}
			if(fSpecialInstrSel.read() == MBKPT) {
				assert(isBkptOn);
				s << "mbkpt" << endl;
			}
			break;

		case READ_SPECIAL_REGS:
			s << "MOVFS" ;
			if(isForced32bitsOpcode) {
				s <<".l";
			}
			s << " R" << fRdInd.read() ;
			switch(fRSpecilaReadRegtype.read()) {
			case READ_CONFIG:
				s << ",CONFIG;";
				break;
			case READ_IMASK:
				s << ",IMASK;";
				break;
			};

			s << endl;
			break;

			case WRITE_SPECIAL_REGS:
				s << "MOVTS" ;
				if(isForced32bitsOpcode) {
					s <<".l";
				}
				s << " LC,";
				assert(isHwLoopOn==false);
				s << " R" << fRdInd.read() ;
				s << endl;
				break;

			case MOV_REG:
				s << "MOV" ;
				PrintCondS::PrintCond(fCondType.read(),s);

				if(isForced32bitsOpcode) {
					s <<".l";
				}

				s << " R" << fRdInd.read() << ",R" << fRnInd.read() <<  ";" << endl;
				break;
			case MOV_IMM16:
				s << "MOV" ;
				if(isForced32bitsOpcode) {
					s <<".l";
				}

				s << " R" << fRdInd.read() << ",#0x" << hex << fUimm16.read() <<  ";" << endl;
				break;
			case MOVT_IMM16:
				s << "MOVT" ;
				s << " R" << fRdInd.read() << ",#0x" << hex << fUimm16.read() <<  ";" << endl;
				break;
			case MOV_IMM8 :
				s << "MOV" ;
				if(isForced32bitsOpcode) {
					s <<".l";
				}

				s << " R" << fRdInd.read() << ",#0x" << hex << fUimm8.read() <<  ";" << endl;
				break;
			case MOV_STATUS :
				if(fMovToStatusDir.read()) {
					s << "MOVTS" ;
					if(isForced32bitsOpcode) {
						s <<".l";
					}
					s << " STATUS," ;
					s << " R" << fRnInd.read() <<   ";" << endl;
				} else {
					s << "MOVFS" ;
					if(isForced32bitsOpcode) {
						s <<".l";
					}

					s << " R" << fRdInd.read() << ",STATUS;" << endl;
				}

				break;
		}


		s << dec;
	}
	//contraint part
public:
	scv_smart_ptr<ESpecialRegRead> fRSpecilaReadRegtype;
	scv_smart_ptr<ERegMovType> fRMovetype;

	scv_smart_ptr<unsigned> fRdInd;
	scv_smart_ptr<unsigned> fRnInd;
	scv_smart_ptr<sc_uint<8>  > fUimm8;
	scv_smart_ptr<sc_uint<16> > fUimm16;
	scv_smart_ptr<ECondCode> fCondType;

	scv_bag<pair<sc_uint<16>, sc_uint<16> > > fImm16Dist;

	scv_smart_ptr<bool>  fMovToStatusDir;

	scv_smart_ptr<ESpecialInstr> fSpecialInstrSel;

public:
	SCV_CONSTRAINT_CTOR(TMoveRegRegular) {


		SCV_CONSTRAINT(fRMovetype() != MOV_STATUS);//TODO check status

		if(isHwLoopOn) {
			SCV_CONSTRAINT(fRMovetype() != WRITE_SPECIAL_REGS);
			SCV_CONSTRAINT(fRMovetype() != READ_SPECIAL_REGS);
		}

		//SCV_CONSTRAINT(fRdInd() < NREGS);
		SCV_CONSTRAINT(fRnInd() < NREGS);

		//	SCV_CONSTRAINT(fCondType() != Unconditional);
		SCV_CONSTRAINT(fCondType() != Branch_Link);


		if(isBkptOn==false) {
			fSpecialInstrSel->keep_out(BKPT);
			fSpecialInstrSel->keep_out(MBKPT);
		}

		fSpecialInstrSel->keep_out(WAND); //TODO
	}

public:

	virtual void GenerateMe() {
		this->next();
	}


	//overwrite the next method -- just update the dest reg in RF

	void next() {

		if( fRegsManager ) {

			fRdInd->keep_only(0,NREGS-1);
			fRdInd->keep_out(fRegsManager->GetMemPoitersRegs().first, fRegsManager->GetMemPoitersRegs().second);

		}


		scv_constraint_base::next();



		sc_uint<32> val;

		if( fRegsManager ) {


			switch (fRMovetype.read()) {

			case READ_SPECIAL_REGS :
				fRegsManager->InvalidateReg(fRdInd.read());
				break;

			case MOV_STATUS :
				if(fMovToStatusDir.read()) {
					fRegsManager->InvalidateAllFlags();
				} else {
					fRegsManager->InvalidateReg(fRdInd.read());
				}

				break;

			case MOV_IMM8:
				fRegsManager->SetReg(fRdInd.read(),fUimm8.read());
				break;

			case MOV_IMM16:
				fRegsManager->SetReg(fRdInd.read(),fUimm16.read());
				break;

			case MOVT_IMM16:

				if(fRegsManager->GetReg(fRdInd.read(),val)) {
					val(31,16)= fUimm16.read();
					fRegsManager->SetReg(fRdInd.read(),val);
				} else {
					fRegsManager->InvalidateReg(fRdInd.read());
				}
				break;

			case MOV_REG:
				bool isToInvalidate = true;
				bool condVal,condVal2,condVal3;
				bool cond_eval;

				sc_uint<32> val;
				if( fCondType.read() == Unconditional &&  fRegsManager->GetReg(fRnInd.read(),val)) {

					fRegsManager->SetReg(fRdInd.read(),val);
					isToInvalidate = false;
				}

				//EQ
				if( fCondType.read() ==  EQ && fRegsManager->GetReg(fRnInd.read(),val)  && fRegsManager->GetAZ(condVal)) {

					cond_eval=condVal;
					if(cond_eval)
						fRegsManager->SetReg(fRdInd.read(),val);
					isToInvalidate = false;

					cerr << "moveq _set" << endl;
				}

				//NE
				if( fCondType.read() ==  NE && fRegsManager->GetReg(fRnInd.read(),val)  && fRegsManager->GetAZ(condVal)) {

					cerr << " get AZ " << condVal << endl;
					cond_eval=!condVal;
					cerr << " cond eval" << cond_eval << endl;
					if(cond_eval)
						fRegsManager->SetReg(fRdInd.read(),val);
					isToInvalidate = false;

					cerr << "movne _set" << endl;
				}

				//GTU
				if( fCondType.read() ==  GTU && fRegsManager->GetReg(fRnInd.read(),val)&& fRegsManager->GetAC(condVal)  && fRegsManager->GetAZ(condVal2)) {

					cond_eval= (!condVal2) && (condVal);
					if(cond_eval)
						fRegsManager->SetReg(fRdInd.read(),val);
					isToInvalidate = false;

					cerr << "movgtu _set" << endl;
				}


				//GTEU
				if( fCondType.read() ==  GTEU && fRegsManager->GetReg(fRnInd.read(),val)  && fRegsManager->GetAC(condVal)) {

					cond_eval=condVal;
					if(cond_eval)
						fRegsManager->SetReg(fRdInd.read(),val);
					isToInvalidate = false;

					cerr << "movgteu _set" << endl;
				}



				//LTEU
				if( fCondType.read() ==  LTEU && fRegsManager->GetReg(fRnInd.read(),val)&& fRegsManager->GetAC(condVal)  && fRegsManager->GetAZ(condVal2)) {

					cond_eval=  (condVal2) || (!condVal);
					if(cond_eval)
						fRegsManager->SetReg(fRdInd.read(),val);
					isToInvalidate = false;

					cerr << "movlteu _set" << endl;
				}

				//LTU
				if( fCondType.read() ==  LTU && fRegsManager->GetReg(fRnInd.read(),val)  && fRegsManager->GetAC(condVal)) {

					cond_eval=  (!condVal);
					if(cond_eval)
						fRegsManager->SetReg(fRdInd.read(),val);
					isToInvalidate = false;

					cerr << "movltu _set" << endl;
				}

				//GT
				if( fCondType.read() ==  GT && fRegsManager->GetReg(fRnInd.read(),val)&& fRegsManager->GetAZ(condVal)  && fRegsManager->GetAV(condVal2) && fRegsManager->GetAN(condVal3)) {

					cond_eval =   (!condVal) && (condVal2  == condVal3);
					if(cond_eval)
						fRegsManager->SetReg(fRdInd.read(),val);
					isToInvalidate = false;

					cerr << condVal << " " <<  condVal2 << " " << condVal3 << endl;
					cerr << "movgt _set" << endl;
				}

				//GTE
				if( fCondType.read() ==  GTE && fRegsManager->GetReg(fRnInd.read(),val) && fRegsManager->GetAV(condVal2) && fRegsManager->GetAN(condVal3)) {

					cond_eval = (condVal2  == condVal3);
					if(cond_eval)
						fRegsManager->SetReg(fRdInd.read(),val);
					isToInvalidate = false;

					cerr << "movgte _set" << endl;
				}

				//LT
				if( fCondType.read() ==  LT && fRegsManager->GetReg(fRnInd.read(),val) && fRegsManager->GetAV(condVal2) && fRegsManager->GetAN(condVal3)) {

					cond_eval = (condVal2  != condVal3);
					if(cond_eval)
						fRegsManager->SetReg(fRdInd.read(),val);
					isToInvalidate = false;

					cerr << "movlt _set" << endl;
				}

				//LTE
				if( fCondType.read() ==  LTE && fRegsManager->GetReg(fRnInd.read(),val)&& fRegsManager->GetAZ(condVal)  && fRegsManager->GetAV(condVal2) && fRegsManager->GetAN(condVal3)) {

					cond_eval = condVal || (condVal2  != condVal3);
					if(cond_eval)
						fRegsManager->SetReg(fRdInd.read(),val);
					isToInvalidate = false;

					cerr << "movlte _set" << endl;
				}

				if(isToInvalidate) {
					fRegsManager->InvalidateReg(fRdInd.read());
				}

				break;

			}
			PrintMe(cerr);

		}

	}
};





#endif
