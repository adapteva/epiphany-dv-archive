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

set export = ""
set build = ""
set run = ""
set seed = ""
set tag = "dv-workspace"
set ncores = ""
set wave = ""
set  debug = ""

set fp_only = ""
set integer_on = ""
set no_dma = ""
set no_inter = ""
set no_fp = ""    

set mc_off = ""
set host_master_off = ""
set host_off = ""
set chip_ext_off = ""
set hw_loop_on = ""
set auto_dma_on = "" 
    
set ntests = "1"
set timeout = "" 
set chip_id_random = "" 

set no_run = ""
set random_stop_resume_on = "" 
set bkpt_on = ""
set clock_gate_off = "" 
set d = "" 
 
set target_host = 127.0.0.1
 

set host_north_off = ""
set host_south_off = ""
set host_west_off = ""
set host_east_off = ""
set external_prod_mode = ""

set  host_offset_val_north = "1"
set  host_offset_val_south = "1"
set  host_offset_val_west = "1"
set  host_offset_val_east = "1"
set chip_id = ""
set internal_only = ""


set core_0 = 0
set core_1 = 1
set core_2 = 2
set core_3 = 3

set core_mem_end_address = ""

 
echo ""

if ($#argv == 0) then
#    goto USAGE
  
else

while $#argv > 1 
  switch ($argv[1])
 
     case -d:
  
      set d = "-d"
      breaksw   

    case -export:
  
      set export = "export"
      breaksw
   
      
    case -fpga:
  
      set run = "run_fpga"
      
      breaksw
      
     
    case -target_host:
      shift
      if ( $#argv  == 0)  then
		goto USAGE
      endif
      set target_host = "$argv[1]"
      breaksw
         
    case -run:
  
      set run = "run_vmodel"
      breaksw
    case -build:
  
      set build = "build"
      breaksw

    case -wave:
  
      set wave = " -wave"
      breaksw

    case -random_stop_resume_on:
  
      set random_stop_resume_on = " -random_stop_resume_on"
      breaksw
      
   case -bkpt_on:
  
      set bkpt_on = " -bkpt_on"
      breaksw
 
 
    case -clock_gate_off:
  
      set clock_gate_off = " -clock_gate_off"
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
      
 
      case -auto_dma_on:
      
      set auto_dma_on = "-auto_dma_on"
      breaksw  
    
     case -hw_loop_on:
      
      set hw_loop_on = "-hw_loop_on"
      breaksw   
  
    case -integer_on:
      
      set integer_on = "-integer_on"
      breaksw
      
  case -chip_id_random:
  
      set chip_id_random = "-chip_id_random"
      breaksw        
  
  case -chip_id:
      shift
      if ( $#argv  == 0)  then
		goto USAGE
      endif
      
      set chip_id = "-chip_id $argv[1]"
      
      breaksw
           
 case -core_mem_end_address:
      shift
      if ( $#argv  == 0)  then
		goto USAGE
      endif
      
      set core_mem_end_address = " -core_mem_end_address $argv[1] "   
      
      breaksw         
           
           
  case -no_run:
  
      set no_run = "-no_run"
      breaksw          
    case -fp_only:
      
      set fp_only = "-fp_only   -host_off -chip_ext_off -no_dma -no_inter -internal_only -c 1"
      breaksw
      
    case -no_dma:
      
      set no_dma = "-no_dma"
      breaksw
      
    case -no_fp:
      
      set no_fp = "-no_fp"
      breaksw  
  
  	case -no_inter:
  		set no_inter = "-no_inter"
  		breaksw
  	
  	case -mc_off:
  		set mc_off = "-mc_off"
  		breaksw
    
    case -host_master_off:
  		set host_master_off = "-host_master_off"
  		breaksw		

    case -host_off:
  		set host_off = "-host_off"
  		breaksw		

     
     case -chip_ext_off:
  
      set chip_ext_off = " -chip_ext_off "
      breaksw
    
    case -internal_only:
  
      set internal_only = " -internal_only "
      breaksw
      

     case -host_north_off:
  
      set host_north_off = " -host_north_off "
      breaksw  
            
     case -host_south_off:
  
      set host_south_off = " -host_south_off "
      breaksw  
            
     case -host_west_off:
  
      set host_west_off = " -host_west_off "
      breaksw      
      
     case -host_east_off:
  
      set host_east_off = " -host_east_off "
      breaksw  
      
     case -external_prod_mode:
  
      set external_prod_mode = " -external_prod_mode "
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

    case -[nN]:
      shift
      if ( $#argv  == 0)  then
	goto USAGE
      endif
      set ntests = "$argv[1]"
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


    case -seed:
      shift
      if ( $#argv  == 0)  then
		goto USAGE
      endif
      
      set seed = " -seed $argv[1]"
  
      breaksw


    case -host_offset_val_north:
      shift
      if ( $#argv  == 0)  then
		goto USAGE
      endif
      
      set host_offset_val_north = "$argv[1]"
  
      breaksw
     
     case -host_offset_val_south:
      shift
      if ( $#argv  == 0)  then
		goto USAGE
      endif
      
      set host_offset_val_south = "$argv[1]"
  
      breaksw
 
     case -host_offset_val_west:
      shift
      if ( $#argv  == 0)  then
		goto USAGE
      endif
      
      set host_offset_val_west = "$argv[1]"
  
      breaksw
     
     case -host_offset_val_east:
      shift
      if ( $#argv  == 0)  then
		goto USAGE
      endif
      
      set host_offset_val_east = "$argv[1]"
  
      breaksw       
   
    default:
      echo "invalid arg:"
      breaksw
   
  

  endsw
  

  shift
 
end
endif


#TODO: Fix these paths!!
#set hostname = `hostname`
#if("${hostname}" == "lithium" || "${hostname}" == "dmb89") then
#	set path = ( /home/oraikhman/maxwell_tools_build/linux26_x86_64/bin/ $path )
#else 
#
#    	pushd /soft/devtools/
#  		source /soft/devtools/setup.csh
# 		popd
#	
#endif

#EXPORT

if("${export}" == "export" ) then
  if("${tag}" == "dv-workspace") then 

	echo "not tag ... running for dv-workspace  "

 else	

		(cd ~ ; svn export --force https://adapteva.svn.cvsdude.com/maxwell_sw/tags/${tag} )
		(cd ~/ ; svn export --force https://adapteva.svn.cvsdude.com/maxwell_tools/${tags}/)
 endif
endif  



if("${build}" == "build" ) then
  
  	 echo "making link ln -s e${ncores} e .............. "
  
  	(cd ~/"${tag}"; \rm -fr e )
  	(cd ~/"${tag}";  ln -s e${ncores} e )
  
 
	(cd ~/${tag}/e/Debug/ ; make -j 4 all )

	(cd ~/${tag}/epiphany-unknown-elf-vmodel-run/Debug/; make clean all ; make -j 4 all )
	(cd ~/${tag}/epiphany-unknown-elf-loader/Debug/; make clean all ; make  all )
	
	(cd ~/${tag}/comparegen/Debug/ ; make all )
	(cd ~/${tag}/igen/Debug/ ; make all )
	(cd ~/${tag}/mgen/Debug/ ; make all ) 
	
	chmod a+x ~/${tag}/igen/run/runGen.csh 
	
	
	
	(cd ~/${tag}/epiphany-unknown-elf-gdbserver/Debug/ ; make -j 4 all )
	
	

endif





if("${run}" == "run_vmodel" ) then


	set path = (~/${tag}/epiphany-unknown-elf-gdbserver/Debug/  ~/${tag}/epiphany-unknown-elf-vmodel-run/Debug/ $path )
	setenv LD_LIBRARY_PATH ~/${tag}/e/Debug/
	

	rehash	
	
	echo " path : $path"
        echo "dv epiphany-unknown-elf-vmodel-run " `which epiphany-unknown-elf-vmodel-run`
	

	set dv_run_args = " ${core_mem_end_address}  -core_0 ${core_0} -core_1 ${core_1} -core_2 ${core_2} -core_3 ${core_3} -tag $tag ${debug} ${wave} -c ${ncores} ${seed} ${clock_gate_off} ${bkpt_on} ${random_stop_resume_on} ${host_off} ${host_master_off}  ${chip_ext_off} ${host_north_off} ${host_south_off} ${host_east_off} ${host_west_off} ${mc_off} ${no_run} ${chip_id_random}  ${chip_id} ${internal_only} ${integer_on}  ${auto_dma_on}  ${hw_loop_on} ${d}  ${fp_only} ${no_dma} ${no_fp} ${no_inter} ${timeout}  -n ${ntests} -host_offset_val_north ${host_offset_val_north} -host_offset_val_south ${host_offset_val_south} -host_offset_val_west ${host_offset_val_west} -host_offset_val_east ${host_offset_val_east} "
	echo "running ~/${tag}/igen/run/runGen.csh ${dv_run_args} ......... "
	~/${tag}/igen/run/runGen.csh ${dv_run_args}
endif



if("${run}" == "run_fpga" ) then
	echo " Runing the FPGA target, Please check the the gdbserver is up and running"
	set path = (~/${tag}/epiphany-unknown-elf-loader/Debug/  $path )
	
	
	
	set c_test = 0
	
	while  ${c_test} != ${ntests} 

		echo "PROD test ${c_test}" 
 
 		@ c_test = ${c_test} + 1
	
	
		set dv_run_args = " ${core_mem_end_address} -core_0 ${core_0} -core_1 ${core_1} -core_2 ${core_2} -core_3 ${core_3}   -no_run -tag $tag ${debug} ${wave} -c ${ncores} ${seed} ${clock_gate_off} ${bkpt_on} ${random_stop_resume_on} ${host_off} ${host_master_off}  ${chip_ext_off}  ${host_north_off} ${host_south_off} ${host_east_off}  ${external_prod_mode} ${host_west_off} ${mc_off} ${no_run} ${chip_id_random} ${chip_id} ${internal_only} ${integer_on}  ${auto_dma_on}  ${hw_loop_on} ${d}  ${fp_only} ${no_dma} ${no_fp} ${no_inter} ${timeout} -host_offset_val_north ${host_offset_val_north} -host_offset_val_south ${host_offset_val_south} -host_offset_val_west ${host_offset_val_west} -host_offset_val_east ${host_offset_val_east}   "
		echo "running ~/${tag}/igen/run/runGen.csh ${dv_run_args} ......... "
	
		#create results
		~/${tag}/igen/run/runGen.csh ${dv_run_args}
	
	
		#exit
		#( date ; /home/oraikhman/dv-workspace/epiphany-dv-run/dv_run.sh -tag dv-workspace -no_inter -c 16 -host_north_off -host_south_off -host_west_off -host_master_off -host_offset_val_east 8 -chip_id 89 -mc_off -fpga -no_dma -n 50 ; date ) | & tee 16.log

		rm -fr fpga*.srec  

		#loader 
		#epiphany-unknown-elf-loader -gdbserver_host_addr ${target_host} -batch -reset_target -run_target test.srec
	
		epiphany-unknown-elf-loader -gdbserver_host_addr ${target_host}	-batch -reset_target test.srec

	
		e-gdb -x launch.gdb launch.elf

		#exit 
	sleep 10
	#exit 
	#echo "LOADER DONE"
	
	echo "target remote ${target_host}:51016" >! gdb_x.cmd
	
	grep 'istribute address space' mgen.log | awk '{ stAddr=strtonum(sprintf("0x%s",substr($8,2))); printf ("dump srec memory fpga_out_%s.srec 0x%x   0x%x\n",$7,stAddr,stAddr+0x8000);}' >>  gdb_x.cmd
	grep 'host_offset east' mgen.log  | awk -v ncores=${ncores} '{a = strtonum($3) ; for(i=0;i<ncores; i=i+1 ){ste = a + i * 0x100000;  printf("dump srec memory fpga_out_%x_ext.srec 0x%x 0x%x \n",i, ste, ste+0x8000 );   }}' >>  gdb_x.cmd

	
	
	#set echo        
	#exit
	#echo "dump srec memory fpga_out_0.srec 0x82000000  0x82008000" >>  gdb_x.cmd
	#echo "dump srec memory fpga_out_1.srec 0x82100000  0x82108000" >>  gdb_x.cmd
	
	#external
	#echo "dump srec memory fpga_out_0_ext.srec 0x80000000  0x80008000" >>  gdb_x.cmd
	#echo "dump srec memory fpga_out_1_ext.srec 0x80100000  0x80108000" >>  gdb_x.cmd
	
	echo "q" >>  gdb_x.cmd

	#get data
	epiphany-unknown-elf-gdb -batch-silent --nw -x gdb_x.cmd  test.srec	
	
	#exit
	
	#convert to srec3
	#epiphany-unknown-elf-objcopy  --input-target srec --output-target srec --srec-forceS3   fpga_out_0.srec  fpga3_out_0.srec
	
	foreach fserc ( fpga_out_*.srec) 		
		set outsrec3 = $fserc:r
		set outsrec3 = ${outsrec3}_s3.srec
		
		epiphany-unknown-elf-objcopy  --input-target srec --output-target srec --srec-forceS3   $fserc $outsrec3
	end
	
	#exit 
	
	#epiphany-unknown-elf-objcopy  --input-target srec --output-target srec --srec-forceS3   fpga_out_1.srec  fpga3_out_1.srec

	#epiphany-unknown-elf-objcopy  --input-target srec --output-target srec --srec-forceS3   fpga_out_0_ext.srec fpga3_out_0_ext.srec 
	#epiphany-unknown-elf-objcopy  --input-target srec --output-target srec --srec-forceS3   fpga_out_1_ext.srec fpga3_out_1_ext.srec 

	#cat fpga3_out_0.srec  fpga3_out_1.srec fpga3_out_0_ext.srec fpga3_out_1_ext.srec  >! fpga3_out_all.srec
	
	#cat fpga3_out_0.srec  fpga3_out_1.srec   >! fpga3_out_all.srec
	
	#cat fpga3_out_0.srec  >! fpga3_out_all.srec
	
	cat  fpga_out_*s3.srec >! fpga3_out_all.srec

	#compare
	epiphany-unknown-elf-loader  -compare core_mem_data_result.txt  fpga3_out_all.srec



if( ${status} != 0  ) then
    echo "FAIL result"

    exit -7

endif
    echo "status $status"
    echo "PASS "
	
end
	
endif
	## ----------- ${c_test} != ${ntests} 
	

	
exit

USAGE: 
echo "run_dv.sh [-tag <tag> (HEAD) ]   [-export] [-build]  [-run]  [-c <number cores>=1]  [-nN <num_tests>=1] [ -seed <seed>] [-timeout <cycles>]  [-clock_gate_off] [-bkpt_on | -random_stop_resume_on]  [-integer_on] [-no_dma] [ -no_inter]  [-mc_off] [-host_off] [-host_master_off] [-chip_ext_off ]  [ -host_{north,south,east,west}_off ]  [-chip_id_random]  [-chip_id <id>]  [ [ -no_fp ] |[-fp_only] ]  [-auto_dma_on]  [-hw_loop_on] [ -host_offset_val <val> ]  [-core_{0,1,2,3} core_num_in_mesh] [-core_mem_end_address <0xhex>]"
echo "example:  -tag dv-workspace -no_inter -no_dma -c 2 -host_master_off -fpga -host_offset_val_east 8 -host_north_off -host_south_off -host_west_off -mc_off"
echo "example:  -tag dv-workspace -no_inter -no_dma -c 2 -host_off -fpga -host_offset_val_east 8        -host_north_off -host_south_off -host_west_off -chip_id 89 -mc_off"
#/home/oraikhman/dv-workspace/epiphany-dv-run/dv_run.sh -tag dv-workspace -no_inter -no_dma -c 16 -chip_ext_off -host_master_off -fpga -host_offset_val_east 8 -chip_id 89 -mc_off -target_host 192.168.1.108#
#/home/oraikhman/dv-workspace/epiphany-dv-run/dv_run.sh -tag dv-workspace -no_inter -chip_ext_off -c 16 -host_north_off -host_south_off -host_west_off -host_master_off -host_offset_val_east 8 -chip_id 89 -mc_off -fpga -no_dma | & tee 2.log

echo "--------------------------- runGen.csh options -------------------------------------------------------------------------------------------------------"
echo "-internal_only -- all transactions will be restricted to the internal memory"
echo "-host_off -- external ( host ) is 'closed' for transactions, only inter core traffic is allowed"
echo "-host_master_off -- the host will not generate transactions, slave mode only"
echo "-host_{north,south,east,west}_off -- close the corresponding host for traffic"
echo "-mc_off -- the multi cast transactions will be not generated, the is the default for the one core env"
echo "-chip_ext_off -- don't generate access to off chip memory ( host will active as master only) , while access to other core is possible"
echo "-no_inter -- don't generate the interrupts and exceptions"
echo "-fp_only -- generate the floating point intruction pattern only, ( rn=val1,rm=val2,rd=val3,rd=FP(rn,rm), [mem]=rd"
echo "-no_fp -- don't generate the floating point instructions"
echo "-integer_on -- generate the integer instructions" 
echo "-random_stop_resume_on -- generate sequence random stop and resume requests"
echo "-bkpt_on -- generate bkpt instruction" 
echo "-clock_gate_off -- support clock gating configuration setting"
echo "-hw_loop_on -- will turn on the HW loop generation, imply -no_inter and -internal_only -no_dma as well"
echo "-auto_dma_on -- will lallow auto dma generation"
echo "-chip_id -- set chip is <0x88>"
echo "-chip_id_random -- generate the chip_id in random way"
echo "-core_mem_end_address -- set the end of internal memory range, default is 0x8000, 32KB "
echo "-no_run -- don't run the design, generate and run test using the functional siumulator only"
echo "-wave --record the vcd file with max level"
echo "-tag <tag> tag to run the dv and design"

#INTERNAL 
#( date ; /home/oraikhman/dv-workspace/epiphany-dv-run/dv_run.sh -tag dv-workspace -no_inter -c 16 -host_north_off -host_south_off -host_west_off -host_master_off -chip_id 89 -mc_off -chip_ext_off -fpga -seed 2 ; date ) | & tee 16.log
#( date ; /home/oraikhman/dv-workspace/epiphany-dv-run/dv_run.sh -tag dv-workspace -no_inter -c 16 -host_north_off -host_south_off -host_west_off -host_master_off -chip_id 89 -mc_off -external_prod_mode -fpga -seed 2 ; date ) | & tee 16.log

