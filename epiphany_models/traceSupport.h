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

#ifndef _TRACE_SUPPORT_H_
#define _TRACE_SUPPORT_H_

#ifndef DV_ONLY
#include "top_wrapper.h"         //Verilated RTL Header
#else
#include "dv_top.h"
#endif

class TraceSupport {
private:
	top_wrap* iw_top;

public :

	TraceSupport(int _vcdLevel, char* _vcdfile,top_wrap* _itop) {
		iw_top=_itop;
		iw_top->IniTraceSupport(_vcdLevel,_vcdfile);
	}

	void StartRecord() {
		iw_top->StartRecord();

	}

	void CloseRecord() {
		iw_top->CloseRecord();
	}

	~TraceSupport() {
		iw_top->DeleteTraceFile();
	}

};



#endif
