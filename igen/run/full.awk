#Copyright (C) 2011 Adapteva, Inc.
#Contributed by Oleg Raikhman <support@adapteva.com>
#
#This program is free software: you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation, either version 3 of the License, or
#(at your option) any later version.
#
#This program is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#GNU General Public License for more details.
#
#You should have received a copy of the GNU General Public License
#along with this program, see the file COPYING. If not, see
#<http://www.gnu.org/licenses/>.

BEGIN {
	chip_offset
    #addr=0x90000000   ;
    addr=strtonum(chip_offset);
    size = 1;
    data_str="";
    s_length=4;
}



{
    
    
    next_addr = strtonum(sprintf("0x%s",$1)) 
    data_str = sprintf("%s%s",data,data_str);
    if( addr + 1 == next_addr &&  size < s_length ){
 	 
	size = size + 1;
     } else {
 	#printf( "=========%x___%s size %d\n",addr-size+1, data_str, size);
	if(size==1) {tr_str="000000";type=1;}
	if(size==2) {tr_str="0000";type=5;}
	if(size==3) {tr_str="00";type=9;}
	if(size==4) {tr_str="";type=9;}
	#if(size==5) {tr_str="000000";type=0xd;}
	#if(size==6) {tr_str="0000";type=0xd;}
	#if(size==7) {tr_str="00";type=0xd;}	
	#if(size==8) {tr_str="";type=0xd;}
	if( length(data_str) > 0 ) printf("%.8x_%s%s_%x_0_%x\n",0,tr_str,data_str,addr-size+1,type);
	

	size = 1;data_str="";

	s_length=1;
	if( (next_addr % 2) ==0  )  s_length=2;
	if( (next_addr % 4) ==0  )  s_length=4;
	#if( (next_addr % 8) ==0  )  s_length=8;
	
     }

    addr = next_addr;
    data = $2
    #print $0
    
}

