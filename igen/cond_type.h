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

#ifndef COND_TYPE_H
#define COND_TYPE_H


enum ECondCode {
	EQ,

    NE,

    GTU,

    GTEU,

    LTEU,

    LTU,

    GT,

    GTE,

    LT,

    LTE,

    BEQ,

    BNE,

    BLT,

    BLTE,

  Unconditional,
  Branch_Link
};

struct PrintCondS {
  static void PrintCond(ECondCode condType , ::std::ostream& s= ::std::cout ) {
    switch (condType) {

    case EQ:
    	s << "EQ";
    	break;

    case NE:
    	s << "NE";
    	break;

    case GTU:
    	s << "GTU";
    	break;

    case GTEU:
    	s << "GTEU";
    	break;

    case LTEU:
    	s << "LTEU";
    	break;

    case LTU:
    	s << "LTU";
    	break;

    case GT:
    	s << "GT";
    	break;

    case GTE:
    	s << "GTE";
    	break;

    case LT:
    	s << "LT";
    	break;

    case LTE:
    	s << "LTE";
    	break;

    case BEQ:
    	s << "BEQ";
    	break;

    case BNE:
    	s << "BNE";
    	break;
    case BLT:
    	s << "BLT";
    	break;

    case BLTE:
    	s << "BLTE";
    	break;


    case Unconditional:
        s << "";
    break;
    case Branch_Link:
        s << "B___LINK____";
    break;
    }
  }
};


template<>
class scv_extensions<ECondCode> : public scv_enum_base<ECondCode> {
public:

  SCV_ENUM_CTOR(ECondCode) {

	  SCV_ENUM(EQ);

	  SCV_ENUM(NE);

	  SCV_ENUM(GTU);

	  SCV_ENUM(GTEU);

	  SCV_ENUM(LTEU);

	  SCV_ENUM(LTU);

	  SCV_ENUM(GT);

	  SCV_ENUM(GTE);

	  SCV_ENUM(LT);

	  SCV_ENUM(LTE);


	  SCV_ENUM(BEQ);
	  SCV_ENUM(BNE);
	  SCV_ENUM(BLT);
	  SCV_ENUM(BLTE);


    SCV_ENUM(Unconditional);
    SCV_ENUM(Branch_Link); 
 }
};

#endif
