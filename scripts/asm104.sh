#!/bin/csh
##################################################################
#Copyright (c) 2010, Oleg Raikhman, Adapteva, Inc
#All rights reserved.
#
#Redistribution and use in source and binary forms, with or without modification,
#are permitted provided that the following conditions are met:
#
#Redistributions of source code must retain the above copyright notice, this
#list of conditions and the following disclaimer.

#Redistributions in binary form must reproduce the above copyright notice, this
#list of conditions and the following disclaimer in the documentation and/or
#other materials provided with the distribution.

#Neither the name of the copyright holders nor the names of its
#contributors may be used to endorse or promote products derived from
#this software without specific prior written permission.
#
#THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
#ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
#WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
#ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
#(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
#LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
#ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
########################################################################

set usestdio = ""

set ncores = 1

set core_0 = 0
set core_1 = 1
set core_2 = 2
set core_3 = 3

set wave = ""

set no_run = 0
set sim_run = 0

set tag = dv-workspace

set srec_file = ""
set srec_out_file = "a.mem104"

set north_base = 0x062000000
set south_base = 0x0a2000000
set west_base = 0x081800000
set east_base = 0x82400000

set elf_file = "./a.elf"
set sim_mem_region = ""
set chip_base = "82000000"

set drive_host = "north"

if ($#argv == 0) then
#    goto USAGE
  
else

while $#argv > 1 
  switch ($argv[1])
 
    case -wave:
  
      set wave = " -always_record_wave -vcdlevel 99 "
      breaksw

         
    case -h:
    case -help:
	case --help:
	
      goto USAGE
      breaksw

    case -d:
    case -debug:
      
      set debug = " -d "
      breaksw
      
      
      
     case -srec_file:
      shift
      if ( $#argv  == 0)  then
		goto USAGE
      endif
      
      set srec_file = "$argv[1]"
      
     breaksw  
      
     case -north_base:
      shift
      if ( $#argv  == 0)  then
		goto USAGE
      endif
      
      set north_base = "$argv[1]"
      
      breaksw
  
    case -south_base:
      shift
      if ( $#argv  == 0)  then
		goto USAGE
      endif
      
      set south_base = "$argv[1]"
      
      breaksw    
   
   
   case -west_base:
      shift
      if ( $#argv  == 0)  then
		goto USAGE
      endif
      
      set west_base = "$argv[1]"
      
      breaksw
  
    case -east_base:
      shift
      if ( $#argv  == 0)  then
		goto USAGE
      endif
      
      set east_base = "$argv[1]"
      
      breaksw
       
   case -drive_host:
      shift
      if ( $#argv  == 0)  then
		goto USAGE
      endif
      
      set drive_host = "$argv[1]"
      
      breaksw 
        
   
    case -chip_base:
      shift
      if ( $#argv  == 0)  then
		goto USAGE
      endif
      
      set chip_base = "$argv[1]"
      
      breaksw
      
   	case -srec_out_file:
      shift
      if ( $#argv  == 0)  then
		goto USAGE
      endif
      
      set srec_out_file = "$argv[1]"
      
      breaksw   
      
      
          
      
    case -usestdio:
  
      set usestdio = " -usestdio "
      breaksw   
       
    case -no_run:
  
      set no_run = 1
      breaksw         

    case -sim_run:
  
      set sim_run = 1
      breaksw 
      
   case -core_0:
      	shift
      	if ( $#argv  == 0)  then
			goto USAGE
     	endif
         
      	set core_0 = $argv[1]
      breaksw
  
   case -core_1:
      	shift
      	if ( $#argv  == 0)  then
			goto USAGE
     	endif
         
      	set core_1 = $argv[1]
      breaksw
  
    case -core_2:
      	shift
      	if ( $#argv  == 0)  then
			goto USAGE
     	endif
         
      	set core_2 = $argv[1]
      breaksw
  
     case -core_3:
      	shift
      	if ( $#argv  == 0)  then
			goto USAGE
     	endif
         
      	set core_3 = $argv[1]
      breaksw

    case -tag:
      shift
      if ( $#argv  == 0)  then
		goto USAGE
      endif
      set tag = $argv[1]
      breaksw 
      
     case -sim_mem_region:
      shift
      if ( $#argv  == 0)  then
		goto USAGE
      endif
      set sim_mem_region = "$argv[1]"
      breaksw      
      
    case -[cC]:
      shift
      if ( $#argv  == 0)  then
		goto USAGE
      endif
      set ncores = $argv[1]
      breaksw
      
      
    case -elf_file:
      shift
      if ( $#argv  == 0)  then
		goto USAGE
      endif
      set elf_file = $argv[1]
      breaksw 
      
    case -force_stop_at_addr:
      shift
      if ( $#argv  == 0)  then
		goto USAGE
      endif
      set force_stop_at_addr = $argv[1]
      breaksw  


    case -[cC]:
      shift
      if ( $#argv  == 0)  then
		goto USAGE
      endif
      set ncores = $argv[1]
      breaksw



    case -timeout:
      shift
      if ( $#argv  == 0)  then
		goto USAGE
      endif
      set timeout = "-timeout $argv[1]"
      breaksw
     
   
    default:
      echo "invalid arg:"
      breaksw
   
  

  endsw
  

  shift
 
end
endif


	
	set hostname = `hostname`
	if("${hostname}" == "lithium" || "${hostname}" == "dmb89") then
		set path = ( /home/oraikhman/maxwell_tools_build/linux26_x86_64/bin/ $path )
	else 

    	pushd /soft/devtools/
  		source /soft/devtools/setup.csh
 		popd
	
	endif

	if( ${sim_run} == 1 )  	 then
		set memr = `epiphany-unknown-elf-objdump -h ${elf_file} | gawk '{ if(match($4, /^[1-9,a-f][1-9,a-f]/) ) {printf(" --memory-region 0x%s,0x%s",$4,$3) } } '`
		epiphany-unknown-elf-run ${sim_mem_region}  ${memr} -t ${elf_file}

	endif
	
	if( ${srec_file} == "") then
	
		set srec_file = `echo $elf_file:r`
	
		set srec_file = ${srec_file}.srec
	
		epiphany-unknown-elf-objcopy --input-target elf32-epiphany --output-target srec  --srec-forceS3   ${elf_file} ${srec_file}

		epiphany-unknown-elf-loader -chip_offset ${chip_base} -convert104 ${srec_out_file} ${srec_file}
		
		set srec_file = "${srec_out_file}"
	else 
	
		echo "Having srec file ${srec_file}"
	endif
	
if( ${no_run} == 1 )  	 then
	exit
endif	
	setenv LD_LIBRARY_PATH ~/${tag}/e/Debug/
	~/${tag}/epiphany-unknown-elf-vmodel-run/Debug/epiphany-unknown-elf-vmodel-run  ${usestdio} -n $ncores -core_0 ${core_0} -core_1 ${core_1} -core_2 ${core_2} -core_3 ${core_3} -west_base ${west_base} -batch "-${drive_host}file" ${srec_file} ${wave} -trace_external -force_stop_at_addr ${force_stop_at_addr}
	

exit 




USAGE:
	echo "usage dv_run_manual_test.sh [-tag] [-force_stop_at_addr <external addr>] [-elf_file file| -srec_file <file>] [-usestdio] [-chip_base <chip_base_addr>]  [ -sim_run [-sim_mem_region <mem_region_opt>] ] [-c <number cores>=1]  [-timeout =160000] [-core_{0,1,2,3} core_num_in_mesh] [-{north,south,west,east}_base <0xaddr>] [-drive_host {north,south,west,east}]"
echo "-force_stop_at_addr <external addr>  -- Force test to stop when the extrnal transaction is hit in the host, e.g. 72000000"
echo "-elf_file            --  The elf file to run, default is a.elf"
echo "-chip_base           --  The chip base address, the default is 82000000"
echo "-tag                 --  The tag to run the design and DV"
echo "-sim_run             --  The elf file will be run on the functional simu;ator"
echo "-sim_mem_region      --  The extend mem region needed by simulator, e.g. '--memory-region 0x072000000,0x100000'" 
echo "-north_base <0xaddr> --  The address of north host, the default is 0x62000000"
echo "-south_base <0xaddr> --  The address of south host, the default is 0xa2000000"
echo "-west_base <0xaddr>  --  The address of west host,  the default is 0x81800000"
echo "-east_base <0xaddr>  --  The address of east host,  the default is 0x82400000"
echo "-drive_host [host]   --  Load program from host, {north,south,west,east}"
echo "-srec_file <file>    --  Use srec file, rather than elf file"
echo "-srec_out_file <file>--  Generate serec file <file>, the default is a.mem104"

echo "Example : asm104.sh -tag dv-workspace -force_stop_at_addr 81800000 -chip_base 82600000 -elf_file $cwd/1_asm/a.elf"
echo "asm104.sh -west_base 8000000 -tag fake-dv-workspace -force_stop_at_addr 81800000 -chip_base 82000000 -elf_file dma.elf"
exit







##################################################################################################################################################
#svn export https://adapteva.svn.cvsdude.com/maxwell_sw/trunk/scripts/elf2emesh
#chmod a+x ./elf2emesh
#./elf2emesh -elf ${fname}.elf -off "2181038080"  -o ${fname}.memh

echo "104 file : ${fname}.memh"


echo "runing the simualtor"
epiphany-unknown-elf-run --memory-region 0x072000000,0x100000 -t ${fname}.elf

epiphany-unknown-elf-objcopy --input-target elf32-epiphany --output-target srec  --srec-forceS3   a.elf a.srec

epiphany-unknown-elf-loader -chip_offset 82000000 -convert104 a.mem104 a.srec


setenv LD_LIBRARY_PATH ~/${tag}/e/Debug/
~/${tag}/epiphany-unknown-elf-vmodel-run/Debug/epiphany-unknown-elf-vmodel-run -batch -westfile a.mem104 ${wave} -trace_external -force_stop_at_addr 72000000

exit
==============================================================================================================================================
/home/oraikhman/dv-workspace/epiphany-dv-run/asm104.sh -core_0 6 -west_base 80000000 -tag dv-workspace -force_stop_at_addr 81800000 -chip_base 82600000 -elf_file /home/oraikhman/tmp/ISR_FP/dma/Debug/dma.elf -no_run -srec_out_file 6.mem
/home/oraikhman/dv-workspace/epiphany-dv-run/asm104.sh -core_0 6 -west_base 80000000 -tag dv-workspace -force_stop_at_addr 81800000 -chip_base 92000000 -elf_file /home/oraikhman/tmp/ISR_FP/dma/Debug/dma.elf -no_run -srec_out_file 32.mem
/home/oraikhman/dv-workspace/epiphany-dv-run/asm104.sh -core_0 6 -west_base 80000000 -tag dv-workspace -force_stop_at_addr 81800000 -chip_base 96700000 -elf_file /home/oraikhman/tmp/ISR_FP/dma/Debug/dma.elf -no_run -srec_out_file 47.mem
/home/oraikhman/dv-workspace/epiphany-dv-run/asm104.sh -core_0 6 -west_base 80000000 -tag dv-workspace -force_stop_at_addr 81800000 -chip_base 9e700000 -elf_file /home/oraikhman/tmp/ISR_FP/dma/Debug/dma.elf -no_run -srec_out_file 63.mem
rm -fr 6_32_47_63.mem
cat 6.mem 32.mem 47.mem 63.mem >> 6_32_47_63.mem

/home/oraikhman/dv-workspace/epiphany-dv-run/asm104.sh -tag dv-workspace -core_0 6 -core_1 32 -core_2 47 -core_3 63 -west_base 80000000 -force_stop_at_addr 81800000 -srec_file $cwd/6_32_47_63.mem

==============================================================================================================================================
set efile = /home/oraikhman/tmp/ISR_FP/dma/Debug/dma.elf
rm -fr 0_1_8_9.mem 0.mem 1.mem 8.mem 9.mem
/home/oraikhman/dv-workspace/epiphany-dv-run/asm104.sh -chip_base 82000000 -elf_file ${efile} -no_run -srec_out_file 0.mem
/home/oraikhman/dv-workspace/epiphany-dv-run/asm104.sh -chip_base 82100000 -elf_file ${efile} -no_run -srec_out_file 1.mem
/home/oraikhman/dv-workspace/epiphany-dv-run/asm104.sh -chip_base 86000000 -elf_file ${efile} -no_run -srec_out_file 8.mem
/home/oraikhman/dv-workspace/epiphany-dv-run/asm104.sh -chip_base 86100000 -elf_file ${efile} -no_run -srec_out_file 9.mem

#cat 0.mem 1.mem 8.mem 9.mem | grep -v 'f0428_0_9' >> 0_1_8_9.mem
#cat 0.mem 1.mem 8.mem 9.mem | grep    'f0428_0_9' >> 0_1_8_9.mem

cat 0.mem 1.mem 8.mem 9.mem  >> 0_1_8_9.mem

/home/oraikhman/dv-workspace/epiphany-dv-run/asm104.sh -tag 4_cores_dv-workspace -core_0 0 -core_1 1 -core_2 8 -core_3 9 -west_base 80000000 -force_stop_at_addr 81800000 -srec_file $cwd/0_1_8_9.mem


