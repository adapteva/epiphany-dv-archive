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

#ifndef COMMON_H
#define COMMON_H

enum ELS_AccessSize {
	BYTE_A=0,
	HALF_A=1,
	WORD_A=2,
	DOUBLE_A=3
};
template<>
class scv_extensions<ELS_AccessSize> : public scv_enum_base<ELS_AccessSize> {
public:
	SCV_ENUM_CTOR(ELS_AccessSize) {
		SCV_ENUM(BYTE_A);
		SCV_ENUM(HALF_A);
		SCV_ENUM(WORD_A);
		SCV_ENUM(DOUBLE_A);
	}
};

#define EXT_MEM_START 0x100000

#endif
