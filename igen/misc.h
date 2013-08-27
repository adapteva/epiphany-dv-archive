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

#ifndef MISC_H
#define MISC_H

#define if_then(a, b) (!(a) || (b))
#define if_then_else(a, b, c) ((!(a) || (b)) && ((a) || (c)))

#include <complex>

template <class Value1>
istream& operator>> (istream& s,  std::pair<Value1,Value1>& p ) {
	std::complex<Value1> v;
	s >> v;
	p = std::pair<Value1,Value1>(v.real(),v.imag());

	return s;
}
template <class Value1>
ostream& operator<< (ostream& s,  std::pair<Value1,Value1>& p ) {
	std::complex<Value1> v	 = std::complex<Value1>(p.first,p.second);
	s << v;

	return s;
}






#endif
