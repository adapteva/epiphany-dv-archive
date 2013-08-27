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

#ifndef SUBROUTINE_H
#define SUBROUTINE_H
struct TSubRoutine {
	/* start address */
	sc_uint<32> fStartAddress;
	/* max size in bytes*/
	unsigned sSize;
	//vector<TInstr_If *> fInstructionList;
	TSubRoutine() {
		fStartAddress=0;

		sSize =  (MAX_SUB_ROUTINE_SIZE-(12 /*jump in end of section*/ + 12 /*loop*/));
	}
};

#endif
