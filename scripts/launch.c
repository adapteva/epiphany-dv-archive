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

asm(".global __stack_start_;");
asm(".set __stack_start_ , 0x1ff0");
asm(".global __USE_INTERNAL_MEM_;");
asm(".set __USE_INTERNAL_MEM_, 1 ");
asm(".global __USE_INTERNAL_MEM_FOR_NEW_LIB_;");
asm(".set __USE_INTERNAL_MEM_FOR_NEW_LIB_ ,1 ");

#define ILATST_OFFSET 0xf042C

unsigned cores_addr_map [16] = {

		0x82400000,
		0x82500000,
		0x82600000,
		0x82700000,
		0x86400000,
		0x86500000,
		0x86600000,
		0x86700000,
		0x8a400000,
		0x8a500000,
		0x8a600000,
		0x8a700000,
		0x8e400000,
		0x8e500000,
		0x8e600000,
		0x8e700000

};

int main() {

  unsigned *addr;
  unsigned i;
  for(i=0;i<16;i++ ) {
    addr = (unsigned*)(cores_addr_map[i]);
    addr = (unsigned*) ((unsigned )  addr | ((unsigned) ILATST_OFFSET));
    ( *addr) = (unsigned)(1);
  }
  return 0;
}
