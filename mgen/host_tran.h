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
#ifndef HOST_TRAN_H_
#define HOST_TRAN_H_

#include <scv.h>

enum ECTRL_MODE {
	REGULAR_CTRL_MODE=0x0,
			MULTI_CAST_CTRL_MODE=0x3,
			    MODIFY_READ_MODE=0xC//or message in case of write
};

template<>
class scv_extensions<ECTRL_MODE> : public scv_enum_base<ECTRL_MODE> {
public:
	SCV_ENUM_CTOR(ECTRL_MODE) {
		SCV_ENUM(REGULAR_CTRL_MODE);
		SCV_ENUM(MULTI_CAST_CTRL_MODE);
	}
};

extern void _scv_pop_constraint();


// Create a actual contraint for the testcase
struct THostBurstGen : public scv_constraint_base {

	scv_smart_ptr<sc_uint<32> > addr;

	scv_smart_ptr<sc_uint<2> > dataSize;
	scv_bag<sc_uint<2> > dataSizeDist;


	scv_smart_ptr<sc_uint<16> >length;

	scv_smart_ptr<bool> isWriteOrReadBurst;
	scv_bag<bool> isWriteOrReadBurstDist;

	scv_smart_ptr<bool> same_addr_burst;
	scv_bag<bool> same_addr_burst_dist;


	//add config variable
	scv_smart_ptr<sc_uint<32> > addr_min;
	scv_smart_ptr<sc_uint<32> > addr_max;
	scv_smart_ptr<ECTRL_MODE> ctrlMode;




	SCV_CONSTRAINT_CTOR(THostBurstGen) {


		// Hard limit on max tran size
		length->keep_only(1,64);

		// Disable randomization
		addr_min->disable_randomization();
		addr_max->disable_randomization();
		ctrlMode->disable_randomization();

		//add extra constraints
		SCV_CONSTRAINT ((addr() >= addr_min()) &&
				(addr() < addr_max()) );

		SCV_CONSTRAINT ( length() <  (addr_max() - addr() ) );

		//SCV_CONSTRAINT (ctrlMode() != MULTI_CAST_CTRL_MODE  || dataSize() != DOUBLE_A );


		dataSizeDist.add(BYTE_A,3);
		dataSizeDist.add(HALF_A,3);
		dataSizeDist.add(WORD_A,3);
		dataSizeDist.add(DOUBLE_A,90);
		dataSize->set_mode(dataSizeDist);

		//SCV_CONSTRAINT ( dataSize() != DOUBLE_A);



		isWriteOrReadBurstDist.add(true,85);
		isWriteOrReadBurstDist.add(false,15);
		isWriteOrReadBurst->set_mode(isWriteOrReadBurstDist);

		//FIXME
		//isWriteOrReadBurst->keep_only(true);

		same_addr_burst_dist.add(true,10);
		same_addr_burst_dist.add(false,90);
		same_addr_burst->set_mode(same_addr_burst_dist);


	}
public:
	void next() {
		scv_constraint_base::next();
		if(ctrlMode.read() == MULTI_CAST_CTRL_MODE && dataSize.read() ==DOUBLE_A ) {
			dataSize.write(WORD_A);
		}
	}
};

bool CompareRangeAddr( pair<sc_uint<32> , sc_uint<32> > i, pair<sc_uint<32> , sc_uint<32> > j) {
	return (  i.first ) < (  j.first );
}

#endif /* HOST_TRAN_H_ */
