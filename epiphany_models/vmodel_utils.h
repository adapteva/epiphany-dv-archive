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


#ifndef VMODEL_UTILS_H_
#define VMODEL_UTILS_H_

#include "systemc.h"	// SystemC
#include "maddr_defs.h"

extern unsigned NORTH_BASE;
extern unsigned SOUTH_BASE;
extern unsigned WEST_BASE;
extern unsigned EAST_BASE;
extern unsigned CHIP_BASE;




inline unsigned GET_EXT_X_FROM_BASE_ADDR(sc_uint<32> addr) {
	return addr(25,23);
}

inline unsigned GET_EXT_Y_FROM_BASE_ADDR(sc_uint<32> addr) {
	return addr(31,29);
}


inline unsigned GET_NCORES_IN_ROW() {


	return 8;

//	if(NCORES == 4 )  {
//		return 2;
//	}
//	else {
//		return 4;
//	}
}

//inline unsigned I_TO_Y(unsigned i) {
////  unsigned y=(i/GET_NCORES_IN_ROW());
////  return y;
//	if(i==0) return 0;
//	if(i==1) return 0;
//	if(i==2) return 1;
//	if(i==3) return 1;
//	assert(0);
//}
//inline unsigned I_TO_X(unsigned i) {
////  unsigned x=(i%GET_NCORES_IN_ROW());
////  return x;
//	if(i==0) return 0;
//	if(i==1) return 1;
//	if(i==2) return 0;
//	if(i==3) return 1;
//	assert(0);
//}

inline unsigned XY_TO_INUM(sc_uint<32> addr) {
	unsigned x = addr(22,20);
	unsigned y = addr(28,26);
	return y*GET_NCORES_IN_ROW() +x;
}

inline unsigned MAKE_ADDRESS_INTERNAL(unsigned addr) {
	return CORE_ADDR_SPACE_MASK & addr;
}

inline unsigned IS_ADDRESS_INTERNAL(unsigned addr) {
	return addr < CORE_SPACE;
}

inline bool IS_ADDRESS_IN_CHIP_RANGE(sc_uint<32> addr) {
	return (addr(31,29) == sc_uint<32>(CHIP_BASE)(31,29) ) && (addr(25,23) == sc_uint<32>(CHIP_BASE)(25,23) ) &&//same as chip id
			(MAKE_ADDRESS_INTERNAL(addr) < CORE_SPACE) /*&& // less than internal space FIXME
			(XY_TO_INUM(addr) < NCORES)*/; //[ 0..MAX_SUPPORTEDC_CORES]
}

extern unsigned fMapCoreCountToCoreNum[];
inline unsigned CORE_NUM_2_ADDR( unsigned core_num) {

  assert(core_num<NCORES);

  return (( fMapCoreCountToCoreNum[core_num] / GET_NCORES_IN_ROW() ) << 26 ) |  ( (fMapCoreCountToCoreNum[core_num] % GET_NCORES_IN_ROW()) <<  20)  ;

  //return  ( (I_TO_Y(core_num)) <<  26)   |  ( (I_TO_X(core_num)) <<  20)  ;
}


inline unsigned MAKE_ADDR_GLOBAL_FROM_CORE_NUM(unsigned addr , unsigned core_num) {

  assert(addr<CORE_SPACE);
  assert(core_num<NCORES);

  return CHIP_BASE | CORE_NUM_2_ADDR(core_num) |  addr;
}



#endif /* VMODEL_UTILS_H_ */
