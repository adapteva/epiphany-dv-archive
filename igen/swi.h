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

#ifndef SWI_H
#define SWI_H



//template <int NREGS>
class TSWI
:       public TInstr_If,
public scv_constraint_base
{
	//implement the Instruction interface
public:

	virtual void PrintMe(::std::ostream& s= ::std::cout ) {
		s << "swi 0;" << endl;

	}
	//contraint part
public:


public:
	SCV_CONSTRAINT_CTOR(TSWI) {


	}

public:

	virtual void GenerateMe() {
		this->next();
	}



};





#endif

