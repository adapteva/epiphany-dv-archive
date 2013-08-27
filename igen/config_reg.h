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

#ifndef CONFIG_REG_H_
#define CONFIG_REG_H_


extern bool isGenInter;


enum EISRSignatureVal {
	NO_ISR=0,
			ISR_NO_NESTING=1,
			ISR_WITH_NESTING=2
};

enum ETimerType {
	Timer32Clk,
	Timer64Clk,
	DisableTimer
};



template<>
class scv_extensions<ETimerType> : public scv_enum_base<ETimerType> {
public:

	SCV_ENUM_CTOR(ETimerType) {
		SCV_ENUM(Timer32Clk);
		SCV_ENUM(Timer64Clk);
		SCV_ENUM(DisableTimer);

	}
};



class TConfigRegSetI : public TInstr_If,
public scv_constraint_base
{
private:
	static const unsigned fTmpRegId=1;
public :
	unsigned GetTmpRegId() { return fTmpRegId;}
public:

	virtual void PrintMask(::std::ostream& s= ::std::cout ) {

		sc_uint<32> fRegVal=0;

		fRegVal[3] = fTimer0Mask.read();
		fRegVal[4] = fTimer1Mask.read();
		fRegVal[8] = fSWIMask.read();

		//generate the mask interrupt value and set IMASK register
		s << "MOV R" << fTmpRegId<< hex <<   ",%low(0x"  << fRegVal<<")" << dec<< endl;
		s << "MOVT R" << fTmpRegId<< hex <<  ",%high(0x" << fRegVal<<")" << dec << endl;
		s << "MOVTS IMASK," ;
		s << " R" <<fTmpRegId <<   ";" << endl;
	}

	void DisableTimer0() {
		fTimer0Type.write(DisableTimer);
	}
	void DisableTimer1() {
		fTimer1Type.write(DisableTimer);
	}
	//used in end of test to clear ILAT if timers is enable but masked
	void FinilizeTimers(::std::ostream& s= ::std::cout) {

		sc_uint<32> fRegVal=0;

		if(fTimer0Type.read() == DisableTimer) {
			s << "mov r0" << hex <<   ",%low(0x"  << 0<<")" << dec<< endl;
			s << "movts ctimer0, r0" << endl;
		}
		if(fTimer1Type.read() == DisableTimer) {
			s << "mov r0" << hex <<   ",%low(0x"  << 0<<")" << dec<< endl;
			s << "movts ctimer1, r0" << endl;
		}


		fRegVal=0;
		if(fTimer0Type.read() != DisableTimer && fTimer0Mask.read() == true) {
			fRegVal[3] = 1;
			s << "//clear ilat for timer 0" << endl;
			s << "mov r0" << hex <<   ",%low(0x"  << fRegVal<<")" << dec<< endl;
			s << "movts ILATCL, r0" << endl;

		}

		fRegVal=0;
		if(fTimer1Type.read() != DisableTimer && fTimer1Mask.read() == true) {
			fRegVal[4] = 1;
			s << "//clear ilat for timer 1" << endl;
			s << "mov r0" << hex <<   ",%low(0x"  << fRegVal<<")" << dec<< endl;
			s << "movts ILATCL, r0" << endl;
		}

		//wait until ilat is 0
		//		s << "WAIT_FOR_ILAT_EQ_0: movfs  r0 ,ilat" << endl;
		//		s << "sub r0, r0 , 0" << endl;
		//		s << "BNE WAIT_FOR_ILAT_EQ_0" << endl;
	}

	unsigned GenerateTimer0Value() {
		//ctimer0->keep_only(550,700);
		ctimer0->next();
		return ctimer0.read();
	}
	unsigned GenerateTimer1Value() {
		//ctimer1->keep_only(550,700);
		ctimer1->next();
		return ctimer1.read();
	}
	void ReprogramTimer(unsigned int tnum, unsigned val,::std::ostream& s= ::std::cout) {
		s << "//reprogram timer "<< tnum-1 << endl;

		s << "\tMOV R"<<fTmpRegId << ",%low(0x" << hex << val << dec <<  ")" << endl;
		s << "\tMOVT R"<< fTmpRegId << ",%high(0x" << hex << val<< dec << ")" << endl;
		s << "\tmovts ctimer"<< tnum-1 << ",r"<< fTmpRegId << "" << endl;

	}

	virtual void PrintMe(::std::ostream& s= ::std::cout ) {
		assert(fRegsManager);



		ReprogramTimer(1,ctimer0.read(),s);
		ReprogramTimer(2,ctimer1.read(),s);

		//s <<dec  << "MOV R"<< tmpRgInd  <<  ",%low(" << hex << "ISR_SAVE_RESTORE_SPACE" << ");" << dec << endl;
		//s <<dec  << "MOVT R" << tmpRgInd  << ",%high(" << hex << "ISR_SAVE_RESTORE_SPACE" << ");"<< dec << endl;
		//s << "\tstr r0,[ R" << tmpRgInd << ",64];save r0" << endl;

		//			s << "\tmovfs  R0,ctimer; check timer ==0 " << endl;
		//			s << "\tsub r0,r0,0;" << endl;
		//			s << "\tbne 0x0; should be never taken" << endl;



		//			if(!isReset) {
		//				s << "\tMOV R"<< tmpRgInd << ",%low(" << ctimer.read()<< ")" << endl;
		//				s << "\tMOVT R"<< tmpRgInd << ",%high(" << ctimer.read()<< ")" << endl;
		//			} else {
		//				s << "\tMOV R"<< tmpRgInd << ",%low(0)" << endl;
		//			}
		//			s << "\tmovts ctimer,r"<< tmpRgInd << "" << endl;






		PrintMask(s);

		PrintConfig(s);

	}

	void PrintConfig(::std::ostream& s= ::std::cout ) {
		sc_uint<32> fRegValCoreConfig=0;
		sc_uint<32> fRegValMeshConfig=0;


		fRegValCoreConfig=0;
		fRegValCoreConfig[0] = isTruncateMode.read();


		if(is32IntMode.read()) {
			s << "//set integer mode" << endl;
			fRegValCoreConfig(19,17)=4;
		}


		switch (fTimer0Type.read()) {
		case Timer32Clk:
			s << "//set regular timer in config" << endl;
			fRegValCoreConfig(7,4) = 1;
			break;
		case Timer64Clk:
			assert(0);
			fRegValCoreConfig(7,4) = 3;
			break;
		case DisableTimer:
			fRegValCoreConfig(7,4) = 0;
			break;
		}

		switch (fTimer1Type.read()) {
		case Timer32Clk:
			s << "//set regular timer in config" << endl;
			fRegValCoreConfig(11,8) = 1;
			break;
		case Timer64Clk:
			assert(0);
			fRegValCoreConfig(11,8) = 3;
			break;
		case DisableTimer:
			fRegValCoreConfig(11,8) = 0;
			break;
		}


		if(fClockGating.read()) {

			//coreconfig, bit[22]-->1 (address XXXF0400)
			fRegValCoreConfig[22]=1;

			//meshconfig, but[1]-->1 (address XXXF0700)
			fRegValMeshConfig[1]=1;
		}

		 //[23] multicore bkpt enable
		if(isBkptOn && fEnableMCBkpt.read() == true) {
			s << "//Enable MCBkpt" << endl;
			fRegValCoreConfig[23]=1;
		}

		if(fDebugPauseDMA_timers.read() == true) {
			s << "//DMA/timer pause on" << endl;
			fRegValCoreConfig[24]=1;
		}
		if(fEnableKernelMode.read() == true) {
			s << "//Kernel mode on" << endl;
			fRegValCoreConfig[25]=1;
		}

		s << "MOV  R" << fTmpRegId << hex << ",%low(0x"  <<fRegValMeshConfig << ");" <<dec << endl;
		s << "MOVT R" << fTmpRegId << hex << ",%high(0x" <<fRegValMeshConfig << ");" <<dec << endl;
		s << "MOVTS meshconfig,R" << fTmpRegId<<endl;


		s << "MOV  R" << fTmpRegId << hex << ",%low(0x"  <<fRegValCoreConfig << ");" <<dec << endl;
		s << "MOVT R" << fTmpRegId << hex << ",%high(0x" <<fRegValCoreConfig << ");" <<dec << endl;
		s << "MOVTS config,R" << fTmpRegId<<endl;

		//		s <<dec  << "MOV R"<< 0  <<  ",%low(" << hex << "ISR_SAVE_RESTORE_SPACE" << ");" << dec << endl;
		//		s <<dec  << "MOVT R" << 0  << ",%high(" << hex << "ISR_SAVE_RESTORE_SPACE" << ");"<< dec << endl;


	}

private:


	scv_smart_ptr<bool> isTruncateMode;

	scv_smart_ptr<bool> is32IntMode;

	scv_smart_ptr<ETimerType> fTimer0Type;
	scv_smart_ptr<ETimerType> fTimer1Type;

	scv_smart_ptr<bool> fTimer0Mask;
	scv_smart_ptr<bool> fTimer1Mask;
	scv_smart_ptr<bool> fSWIMask;

	scv_smart_ptr<bool> fEnableMCBkpt;//[23]=enables multicore bkpt input to core
	scv_smart_ptr<bool> fDebugPauseDMA_timers;//[24]=debug pause mode, turns off the DMA + ctimers when in debug mode
	scv_smart_ptr<bool> fEnableKernelMode;//[25]=enables limited kernel/user mode

	scv_smart_ptr<bool> fClockGating;
	scv_bag<bool> fClockGatingDist;

	scv_smart_ptr<bool> fNesting[3];

	scv_smart_ptr<sc_uint<32> > ctimer0;
	scv_smart_ptr<sc_uint<32> > ctimer1;


	scv_smart_ptr<sc_uint<32> > ctimer0Expected;
	scv_smart_ptr<sc_uint<32> > ctimer1Expected;

	//scv_smart_ptr<bool>   nesting;
public:
	SCV_CONSTRAINT_CTOR(TConfigRegSetI) {

		if(isGenDMA || isGenInter){
			SCV_CONSTRAINT(fEnableKernelMode() == false);
		}



		ctimer0Expected->keep_only(1,6);
		ctimer1Expected->keep_only(1,6);

		SCV_CONSTRAINT(fSWIMask() == false);


		if(isClockGateOff) {
			SCV_CONSTRAINT(fClockGating() == false);
		} else {
			fClockGatingDist.add(true,95);
			fClockGatingDist.add(false,5);
			fClockGating->set_mode(fClockGatingDist);
		}


		if(isIntegerModeOn==false) {
			SCV_CONSTRAINT(is32IntMode() == false);
		} else {
			SCV_CONSTRAINT(is32IntMode() == true);
		}

		if(isGenInter == false) {
			SCV_CONSTRAINT(fTimer0Type() == DisableTimer);
			SCV_CONSTRAINT(fTimer1Type() == DisableTimer);


			SCV_CONSTRAINT(fNesting[1]() == false);
			SCV_CONSTRAINT(fNesting[2]() == false);
			SCV_CONSTRAINT(fNesting[0]() == false);

		} else {

			//SCV_CONSTRAINT(fTimer0Type() != Timer64Clk);
			//SCV_CONSTRAINT(fTimer1Type() != Timer64Clk);
			SCV_CONSTRAINT(fNesting[0]() == false);



			SCV_CONSTRAINT(fNesting[1]() == true);
			SCV_CONSTRAINT(fNesting[2]() == true);

			//TODO
			//SCV_CONSTRAINT(fTimer0Type() == DisableTimer);
			//SCV_CONSTRAINT(fTimer1Type() == DisableTimer);
			SCV_CONSTRAINT(fTimer0Type() == Timer32Clk);
			SCV_CONSTRAINT(fTimer1Type() == Timer32Clk);



			SCV_CONSTRAINT(fTimer0Mask() == false);
			SCV_CONSTRAINT(fTimer1Mask() == false);

		}

	}


public:
	virtual void GenerateMe() {
		next();

		ctimer1->keep_only(50,300);
		ctimer1->next();

		ctimer0->keep_only(50,300);
		//ctimer0->keep_only(ctimer1.read() + 50 ,ctimer1.read() + 100 );
		ctimer0->next();

		if(!isShouldBeOccurTimer0() ) {
			ctimer0Expected.write(0);
		}
		if(!isShouldBeOccurTimer1() ) {
			ctimer1Expected.write(0);
		}

	}


public:

	unsigned GetExpectedForTimer0() {
		return ctimer0Expected.read();
	}
	unsigned GetExpectedForTimer1() {
		return ctimer1Expected.read();
	}


	//allow timer 0, used by nesting ISR in timer1
	void UmaskTimer0() {
		fTimer0Mask.write(false);
	}
	void MaskTimer0() {
		fTimer0Mask.write(true);
	}
	void UmaskTimer1() {
		fTimer1Mask.write(false);
	}
	void MaskTimer1() {
		fTimer1Mask.write(true);
	}
	void UmaskAllTimers() {
		UmaskTimer0();
		UmaskTimer1();
	}

	void MaskAllTimers() {
		MaskTimer0();
		MaskTimer1();
	}


	bool isNestedAllowed(unsigned k) {


		if(k==0)assert(fNesting[k].read()==false);
		return fNesting[k].read();

	}



	bool isShouldBeOccurTimer0() {
		if(fTimer0Mask.read() == false && fTimer0Type.read() == Timer32Clk) {
			return true;
		}
	}
	bool isShouldBeOccurTimer1() {
		if(fTimer1Mask.read() == false && fTimer1Type.read() == Timer32Clk) {
			return true;
		}
	}
	/*
	unsigned GetExpectedTimerISRSignatureValue(unsigned isr_num) {

		unsigned expSignatureVal  = NO_ISR;
		if(isr_num == 1 ) {//timer0
			if(isNestedAllowed()) {
				expSignatureVal=ISR_WITH_NESTING;
			} else {
				if(isShouldBeOccurTimer0()) {
					expSignatureVal=ISR_NO_NESTING;
				}
			}
		}
		if(isr_num == 2 ) {//timer1
			if(isShouldBeOccurTimer1()) {
				if(isNestedAllowed() ) {
					expSignatureVal=ISR_WITH_NESTING;
				} else {
					expSignatureVal=ISR_NO_NESTING;
				}
			}

		}

		return expSignatureVal;

	}
	 */
};


#endif /* CONFIG_REG_H_ */
