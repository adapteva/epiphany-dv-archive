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

#ifndef INTSRUCT_IF_H
#define INTSRUCT_IF_H

//template <int NREGS>
class TInstr_If {

 public:
  virtual ~TInstr_If() {}
   TInstr_If() {
		   fMemManager=0;
		   fRegsManager=0;

		   isForced32bitsOpcode=false;
   }

 protected:
  sc_uint<32> fCpc;
  
  bool isForced32bitsOpcode;


 protected:
  TMemManager *fMemManager;
  TRegManager *fRegsManager;



 public:
  void BindMemManager(TMemManager *MemManager) {fMemManager=MemManager;}
  void BindRegsManager(TRegManager *RegsManager) {fRegsManager=RegsManager;}

 public:

  void Force32bitsOpcodeMode() {
	  isForced32bitsOpcode=true;
  }
  void RestoreRegularOpcodeMode(){
	  isForced32bitsOpcode=false;
  }


  void SetCPC( sc_uint<32>  _cPc) {fCpc=_cPc;}
  virtual sc_uint<32> GetNextPC() { return fCpc+GetInstructionSize(); }
  //TODO taking max size for intruction
  virtual sc_uint<32> GetInstructionSize() {return 4 ;}//TODO

  //virtual sc_uint<16> getOpcode16bits ()  =  0;//TODO
  // virtual sc_uint<32> getOpcode32bits ()  =  0;//TODO
  virtual void PrintMe(::std::ostream& = ::std::cout )  =  0;
  virtual void GenerateMe()  =  0;
};

#endif
