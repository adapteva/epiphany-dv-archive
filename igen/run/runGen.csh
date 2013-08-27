#!/bin/csh
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


rm -fr PASS FAIL

set rerun = false  
set ntests = 1
set debug = false
set ncores  = 1
set chip_id   = 24 # assuming hex number 100,100
set chip_id_random = ""
set internal_only = ""
set external_prod_mode = ""

set nloops = 18
set timeout = 300000

set host_off = ""
set chip_ext_off = ""
set host_master_off = ""
set mc_off = ""
set random_stop_resume_on = ""


set host_north_off = ""
set host_south_off = ""
set host_west_off = ""
set host_east_off = ""


set no_fp = ""
set fp_only = ""
set integer_on = ""
set no_inter = ""
set no_run = 0
set wave = 0
set no_dma = ""
set hw_loop_on = ""
set auto_dma_on = ""
set bkpt_on = ""
set clock_gate_off = ""

set  host_offset_val_north = "1"
set  host_offset_val_south = "1"
set  host_offset_val_west = "1"
set  host_offset_val_east = "1"



set core_0 = 0
set core_1 = 1
set core_2 = 2
set core_3 = 3


set no_check_for_garbage_out = " -no_check_for_garbage_out "


set core_mem_end_address = ""

set tag = dv-workspace

echo ""
if ($#argv == 0) then
#    goto USAGE
  
else

while $#argv > 1 
  switch ($argv[1])
    
 
     
     case -chip_ext_off:
  
      set chip_ext_off = " -chip_ext_off "
      breaksw
      
      
     case -external_prod_mode:
  
      set external_prod_mode = " -external_prod_mode "
      breaksw
      
 
     case -host_master_off:
  
      set host_master_off = " -host_master_off "
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

     case -mc_off:
  
      set mc_off = " -mc_off "
      breaksw 
          
     case -host_off:
  
      set host_off = " -host_off "
      breaksw


    case -internal_only:
  
      set internal_only = " -internal_only "
      breaksw

    case -no_run:
  
      set no_run = 1
      breaksw

    case -wave:
  
      set wave = 99
      breaksw


    case -h:
    case -help:
	case --help:
	
      goto USAGE
      breaksw

    case -d:
    case -debug:
      
      set debug = true
      breaksw
      

    case -no_fp:
      
      set no_fp = " -no_fp "
      breaksw
    
    case -fp_only:
      
      set fp_only = " -fp_only "
      set nloops = 1
      set internal_only = " -internal_only "
      set no_inter = " -no_inter "
      breaksw
      
    case -integer_on:
      
      set integer_on = " -integer_on "
      breaksw
      
      
    case -no_dma:
      
      set no_dma = " -no_dma "
      breaksw
      
     
      
    case -auto_dma_on:
  		set auto_dma_on = " -auto_dma_on " 
  		breaksw	    
      
    case -hw_loop_on:
  		set hw_loop_on = " -hw_loop_on " 
  		set internal_only = " -internal_only "
  		breaksw	

    case -bkpt_on:
  		set bkpt_on = " -bkpt_on " 
  		breaksw
    case -clock_gate_off:
  		set clock_gate_off = " -clock_gate_off " 
  		breaksw


  
  	case -no_inter:
  		set no_inter = "-no_inter"
  		breaksw
      
      

    case -core_mem_end_address:
      shift
      if ( $#argv  == 0)  then
		goto USAGE
      endif
      
      
      set core_mem_end_address = " -core_mem_end_address $argv[1] "
      
      breaksw

      
    case -chip_id:
      shift
      if ( $#argv  == 0)  then
		goto USAGE
      endif
      
      
      set chip_id = $argv[1]
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

   case  -random_stop_resume_on:
  		set random_stop_resume_on = " -random_stop_resume_on "
  		breaksw
  
    case -chip_id_random:
      set chip_id_random = " on "
      breaksw  

    case -[nN]:
      shift
      if ( $#argv  == 0)  then
	goto USAGE
      endif
      set ntests = $argv[1]
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
      set timeout = $argv[1]
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


    case -tag:
      shift
      if ( $#argv  == 0)  then
		goto USAGE
      endif
      set tag = $argv[1]
      breaksw


    case -seed:
      shift
      if ( $#argv  == 0)  then
		goto USAGE
      endif
      
      set r_seed = $argv[1]
      set rerun = true
      set ntests = 1 
      #goto SEED
      breaksw
      
     case -m:
      shift
      if ( $#argv  == 0)  then
		goto USAGE
      endif
      echo "please compile before runnin tests"
      #set model_path = "$argv[1]"
      breaksw
      
           

   
   
    default:
      echo "invalid arg: $argv[1]"
      breaksw
   
  

  endsw
  

  shift
 
end
endif

if(${hw_loop_on} != "" ) then 
	set no_inter = "-no_inter"
	set internal_only = "-internal_only"
	set no_dma = "-no_dma"	
endif


if(${no_dma} != "" && ${host_master_off} != "" && ${no_inter} != "" ) then 
	# supported only core master
	#set no_check_for_garbage_out = ""
endif

echo "Running from tag $tag"

set workspace = ${tag}
set gen_path = /home/${user}/${workspace}/igen/run
set awk_path = ${gen_path}


set path = ( $path ${gen_path}/../../igen/Debug ${gen_path}/../../mgen/Debug  ${gen_path}/../../comparegen/Debug    )





echo "num tests is  ${ntests}"

set c_test = 1;

foreach seed ( `awk -v ntests=$ntests 'BEGIN { srand() ;for ( i = 0 ; i <ntests  ; i++ )  {print int(100000* rand()) int(100000* rand()) ; }}'` ) 
#foreach seed ( `awk -v ntests=$ntests 'BEGIN { srand() ;for ( i = 0 ; i <ntests  ; i++ )  {printf (" 0x%x%x \n",   int(100000* rand()) ,int(100000* rand())) ;  }}'` ) 
     


SEED:


    if(  $rerun == true ) then

	set a =  ${r_seed}
    else
	set a = $seed

    endif 
    
    set test_seed = ${a}

    echo "############################################################################"
	echo "------ WORKSPACE $workspace"         									
    echo "++++++++ Test % ${c_test} running epiphany IGen for $ncores cores using seed ${test_seed}"
    echo "############################################################################"
    @ c_test = ${c_test} + 1
    
    
    
    rm -fr core_* *load*.txt *random*.txt *dma*.txt
    
    if(${chip_id_random} == "on") then
    	
    	set chip_id = `gawk   -v seed=${test_seed} 'BEGIN {srand(seed);printf("%x\n",  (16*( 1+ int(  (100000* rand())%8))) +   (1+ int(  (100000* rand())%8)  ) );}'`
    	   	
    endif
    
    echo "MGEN: The memory range files have been generated chip Id : ${chip_id} "
    
    mgen ${core_mem_end_address}  -core_0 ${core_0} -core_1 ${core_1} -core_2 ${core_2} -core_3 ${core_3}    ${chip_ext_off}  ${external_prod_mode} ${mc_off}   ${host_north_off} ${host_south_off} ${host_east_off} ${host_west_off}  ${host_master_off} ${host_off}  ${internal_only} -id ${chip_id}  -c $ncores  -seed $a   -host_offset_val_north ${host_offset_val_north} -host_offset_val_south ${host_offset_val_south} -host_offset_val_west ${host_offset_val_west} -host_offset_val_east ${host_offset_val_east}  >&! mgen.log
    
    
    set nc = 0;
    
    rm -fr test.srec

    
    
 while $nc != $ncores 
	
	
	set ncreal = ${nc}
	
	
	if(${nc} == 0) then
		set ncreal =  ${core_0}
	endif  
		
	if(${nc} == 1) then
		set ncreal = ${core_1} 
	endif
	
	if(${nc} == 2) then
		set ncreal = ${core_2} 
	endif
		  
	if(${nc} == 3) then
		set ncreal = ${core_3} 
	endif  


	mkdir core_${ncreal}
	pushd core_${ncreal}
	
	
	rm -fr test.s test.o test.ldf  test.elf *.txt

	mv ../mem_sim_${ncreal}.txt .
	mv ../mem_ranges_${ncreal}.txt .


	set iseed = ${a}
	set igen_args = " ${no_inter} ${no_fp} ${integer_on} ${random_stop_resume_on} ${fp_only} ${no_dma} ${hw_loop_on} ${auto_dma_on} ${clock_gate_off}  ${bkpt_on}  -mem mem_ranges_${ncreal}.txt -seed ${iseed} -nloops ${nloops}  -nsubr 5"
	
	echo "-------- Running CORE %${ncreal} -----IGEN seed ${iseed}---------"
	

 	if( $debug == true ) then  
	    igen ${igen_args} >&! test_gen.log
	    #grep "WR allowed"  test_gen.log >> /tmp/TESTASET
	    #grep TESTASET  test_gen.log >> /tmp/TESTASET
	else
	    igen ${igen_args} >&! /dev/null
	endif
	

	set a = `echo ${a} | awk '{print strtonum($1) + 1}'`
	


	epiphany-unknown-elf-as -g -o test.o   test.s
	#cp test.s test.s.$seed
	#cp test.s test.${a}.SS 
	if( $status == 0 ) then 
	    echo "Assembler -- OK"
	else 
	    echo "ERROR assembler for seed $a"
	    exit 2
	endif
	#epiphany-unknown-elf-ld --help
	epiphany-unknown-elf-ld -N -e program_start --script test.ldf -o test.elf test.o 
	if( $status == 0 ) then 
	    echo "Linker -- OK"
	else 
	    echo "ERROR Linker  for seed $a"
	exit 3
	endif
	
	
    
 
	
    epiphany-unknown-elf-objcopy --output-target srec --srec-forceS3 test.elf test.srec
    cat test.srec >> ../test.srec
    
	epiphany-unknown-elf-objdump -h test.elf >! test.elf_dump_header
	epiphany-unknown-elf-objdump -s  test.elf >!  test.elf_dump
	epiphany-unknown-elf-objdump --disassemble-all test.elf >!  test.dis
	
	#post check generation 
	set main_s_check = ` tail -1 test.elf_dump | gawk '{a = strtonum(sprintf ("0x%s\n",$1)); if(a < 0x1000 ) {print 0} else {print 1 } }'`
	if( ${main_s_check} != 0 ) then 
		echo "ERROR: the main section  overlaps the program , check generation"
		exit 876 
	endif
	
	set ls_section_check = `grep output_L_ test.elf_dump_header | gawk 'BEGIN {st=0} {a = strtonum(sprintf ("0x%s\n",$3)); if(a < 256 ) {} else {st=1 } } END{print st}'`
	if( ${ls_section_check} != 0 ) then 
		echo "ERROR: the one of program section oversized (more than 0xff bytes), check generation"
		exit 877
	endif
	
	
		
	epiphany-unknown-elf-run --epiphany-extenal-memory=off    --trace=on `cat mem_sim_${ncreal}.txt`   test.elf  >& sim.log
	set simExitSt = $status
	echo "Simulator exit $simExitSt"

	#DMA need append the auto dma fileto one of host
	touch dma_out.txt host_auto_dma_fill.txt
	
	set chip_full_offset =  `grep chip_full_offset ../mgen.log | awk '{printf $2}'`
	cat  sim.log | grep -v "IGNORE ME"| gawk -v iseed=${iseed} -v coreNum=$ncreal  -v chip_offset=${chip_full_offset} -f ${awk_path}/dump_result.awk  
	if( $status == 0 ) then 
	    #echo  -- OK"
	else 
	    echo "ERROR: Access to non allocated memory seed $a"
	    exit 5
	endif

	set idle = `tail -1 sim.log | awk '{print $2}'`

	if( $idle != "idle" ) then 
	    echo "some problem with SIM/Igen ????"
	    exit
	else
	#	echo "The sim exit status is not 10 ==> probably the test is OK ???? "
	    echo "Reaching idle ... OK "
	endif
	
	echo "<" `egrep -i '(ldr|str)'   sim.log | wc -l` "> accesses to memory as result of Load/Store instructions "  
	echo "		<" `egrep -i '(ldr|str)'   sim.log  | egrep '[[].{2,3}[]]'| wc -l` "> accesses to memory as result of Load/Store instructions (POST MODIFY) " 
# 
	echo "<" `egrep -i '(pc <-)'   sim.log| wc -l` "> change of flow"
   
	echo  "<" `cat  sim.log| wc -l` "> instructions have been executed"
	
	echo "<" `egrep -i 'swi'   sim.log| wc -l` "> NO SWI calls TODO TODO"
	echo "<" `egrep -i 'gid'   test.s| wc -l` ">  planned nested ISRs"
	echo "<" `egrep -i 'TESTSET' sim.log| wc -l` ">  testset instructions"
	
	echo  "<"  `grep TCB test.s| grep "^0" | wc -l` "> dma configurations set for DMA 0"
	echo  "<"  `grep TCB test.s| grep "^1" | wc -l` "> dma configurations set for DMA 1"
	echo  "  <"  `grep -i slave test.s | wc -l` "> autoDMA set"
	
	#exit
	#@ a = $a - 1
	#if( $rerun == true )  then
	#    goto EXIT
	#endif

	popd
	@ nc = ${nc}  +  1
     end # num ores

	 
	 #set f_n_lines  = `cat core_*/mem_dump_in_full.txt | sort | gawk '{printf("%.8x_000000%s_%s_0_1\n",0,$2,$1);}' | wc -l`
	 
	 set f_n_lines  = `cat core_*/mem_dump_in_full.txt | awk '{print $1}' | sort -u | wc -l`
	 set orig_nlines = `( cat core_*/mem_dump_in.txt) | wc -l`
	 if( ${f_n_lines} != ${orig_nlines} ) then
	 	echo "ERROR : multi core gen fail" 
	 	exit 45
	 endif 
	 
	set north_base = `grep  "host_offset north" mgen.log | awk '{printf $3}'`
	set south_base = `grep  "host_offset south" mgen.log | awk '{printf $3}'`
	set west_base = `grep  "host_offset west" mgen.log | awk '{printf $3}'`
	set east_base = `grep  "host_offset east" mgen.log | awk '{printf $3}'`
	
	 
	touch north_mc_signoff_tran.txt south_mc_signoff_tran.txt west_mc_signoff_tran.txt east_mc_signoff_tran.txt
 	
	set hostToDriveAutoDMA =  `gawk -v ncores=$ncores  -v seed=${test_seed} 'BEGIN {srand(seed); print( int(  (100000* rand()%ncores)%4  ) );}'`
 	
 	if(${hostToDriveAutoDMA} == 0) then
 		 cat core_*/host_auto_dma_fill.txt >> host_data_random_north.txt
 	endif
 	cat north_mc_signoff_tran.txt >> host_data_random_north.txt
 	
	
 	if(${hostToDriveAutoDMA} == 1) then 
 		cat core_*/host_auto_dma_fill.txt >> host_data_random_south.txt
 	endif
 	cat south_mc_signoff_tran.txt >> host_data_random_south.txt
	
	
	if(${hostToDriveAutoDMA} == 2) then 
		cat core_*/host_auto_dma_fill.txt >> host_data_random_west.txt
 	endif
 	cat west_mc_signoff_tran.txt >> host_data_random_west.txt
	
	if(${hostToDriveAutoDMA} == 3) then 
		cat core_*/host_auto_dma_fill.txt >> host_data_random_east.txt
 	endif
 	cat east_mc_signoff_tran.txt >> host_data_random_east.txt
 		
	rm -fr east_mc_signoff_tran.txt  north_mc_signoff_tran.txt  south_mc_signoff_tran.txt  west_mc_signoff_tran.txt
	 
	 
	 echo "Test OK >>>>> Preparing the host command files " 
	 
	 
	 
	if(${ncores} == 16 ) then 
		
		 ( echo "ffffffff ff"; cat core_{0,1,2,3}/mem_dump_in_full.txt ) | sort | gawk  -v chip_offset=${chip_full_offset}      -f ${awk_path}/full.awk >! host_load_north.txt
		 ( echo "ffffffff ff"; cat core_{4,5,6,7}/mem_dump_in_full.txt ) | sort | gawk  -v chip_offset=${chip_full_offset}      -f ${awk_path}/full.awk >! host_load_south.txt
		 ( echo "ffffffff ff"; cat core_{8,9,10,11}/mem_dump_in_full.txt ) | sort | gawk  -v chip_offset=${chip_full_offset}      -f ${awk_path}/full.awk >! host_load_west.txt
		 ( echo "ffffffff ff"; cat core_{12,13,14,15}/mem_dump_in_full.txt ) | sort | gawk  -v chip_offset=${chip_full_offset}      -f ${awk_path}/full.awk >! host_load_east.txt
		
		
		head -4 host_data_ilat.txt | tail -4 >> host_load_north.txt
		head -8 host_data_ilat.txt | tail -4 >> host_load_south.txt
		head -12 host_data_ilat.txt | tail -4 >> host_load_west.txt
		head -16 host_data_ilat.txt | tail -4 >> host_load_east.txt
		
	endif
	 
	
	if(${ncores} == 4 ) then 
		
		 ( echo "ffffffff ff"; cat core_${core_0}/mem_dump_in_full.txt ) | sort | gawk  -v chip_offset=${chip_full_offset}      -f ${awk_path}/full.awk >! host_load_north.txt
		 ( echo "ffffffff ff"; cat core_${core_1}/mem_dump_in_full.txt ) | sort | gawk  -v chip_offset=${chip_full_offset}      -f ${awk_path}/full.awk >! host_load_south.txt
		 ( echo "ffffffff ff"; cat core_${core_2}/mem_dump_in_full.txt ) | sort | gawk  -v chip_offset=${chip_full_offset}      -f ${awk_path}/full.awk >! host_load_west.txt
		 ( echo "ffffffff ff"; cat core_${core_3}/mem_dump_in_full.txt ) | sort | gawk  -v chip_offset=${chip_full_offset}      -f ${awk_path}/full.awk >! host_load_east.txt
		
		
		 echo "cores core_${core_0} core_${core_1} core_${core_2} core_${core_3}"
		
		 #( echo "ffffffff ff"; cat core_0/mem_dump_in_full.txt ) | sort | gawk  -v chip_offset=${chip_full_offset}      -f ${awk_path}/full.awk >! host_load_north.txt
		 #( echo "ffffffff ff"; cat core_1/mem_dump_in_full.txt ) | sort | gawk  -v chip_offset=${chip_full_offset}      -f ${awk_path}/full.awk >! host_load_south.txt
		 #( echo "ffffffff ff"; cat core_8/mem_dump_in_full.txt ) | sort | gawk  -v chip_offset=${chip_full_offset}      -f ${awk_path}/full.awk >! host_load_west.txt
		 #( echo "ffffffff ff"; cat core_9/mem_dump_in_full.txt ) | sort | gawk  -v chip_offset=${chip_full_offset}      -f ${awk_path}/full.awk >! host_load_east.txt

		
		head -1 host_data_ilat.txt | tail -1 >> host_load_north.txt
		head -2 host_data_ilat.txt | tail -1 >> host_load_south.txt
		head -3 host_data_ilat.txt | tail -1 >> host_load_west.txt
		head -4 host_data_ilat.txt | tail -1 >> host_load_east.txt
		
	endif	
			
	if(${ncores} == 1 ) then 
		
		( echo "ffffffff ff"; cat core_*/mem_dump_in_full.txt ) | sort | gawk  -v chip_offset=${chip_full_offset}      -f ${awk_path}/full.awk >! host_load_west.txt
		cat host_data_ilat.txt >> host_load_west.txt
		
	endif 
	
	if(${ncores} == 2 ) then 
		
		( echo "ffffffff ff"; cat core_*/mem_dump_in_full.txt ) | sort | gawk  -v chip_offset=${chip_full_offset}      -f ${awk_path}/full.awk >! host_load_west.txt
		cat host_data_ilat.txt >> host_load_west.txt
		
	endif 
	
	
	if(${ncores} == 4 || ${ncores} == 1 || ${ncores} == 16 ||  ${ncores} == 2) then 
	 #OK
	else
		echo "ERROR ( $ncores ) Only 1,2,4 or 16 cores supported"
		exit 90
	endif 
	
 
 	rm -fr host_data_ilat.txt
 
 
 	cat host_data_random_north.txt >> host_load_north.txt
    cat host_data_random_south.txt >> host_load_south.txt
    cat host_data_random_west.txt >> host_load_west.txt
    cat host_data_random_east.txt >> host_load_east.txt
     	
    rm -fr host_data_random_north.txt  host_data_random_south.txt host_data_random_west.txt host_data_random_east.txt
    
   
 
	 #( echo "ffffffff ff"; cat core_*/mem_dump_in_full.txt ) | sort | gawk  -v chip_offset=${chip_full_offset}      -f ${awk_path}/full.awk >! host_load.txt
	 #cat host_data_ilat.txt >> host_load.txt 
     #cat host_data_random_north.txt >> host_load.txt
     
     
	 ( rm -fr spi_load.txt;  touch spi_load.txt ) #TODO
	 
	 # epiphany-unknown-elf-readelf --symbols core_0/test.elf | grep RDS | awk '{ printf ("%x\n",strtonum( sprintf("0x%s",$2)))}'
	 
	 sort  core_*/mem_dump_out_full.txt >! core_mem_data_result.txt
	 	
	 if( ${no_run} == 0 )  	 then
	 
	 	echo "Run design  -----------------------------------------------------------------------------------------"
	 
	 	#echo "00000000_00000000_00000000_0_0 //DUMMY" >! dummy.memh
	 	
	 	
		#echo $LD_LIBRARY_PATH 

		echo -n "BE AWARE :::: Running verilator translated model ... from >>>> "	
		ldd `which epiphany-unknown-elf-vmodel-run` | grep libepiphanymodel.so  |awk '{print $3}'
		rm -fr /tmp/vmodel-run-fif*
		
		
		#echo "Running DV from " `which epiphany-unknown-elf-vmodel-run` " with 
		
		epiphany-unknown-elf-vmodel-run  -core_0 ${core_0} -core_1 ${core_1} -core_2 ${core_2} -core_3 ${core_3}  ${random_stop_resume_on}  -north_base  ${north_base} -south_base  ${south_base}  -west_base  ${west_base} -east_base  ${east_base} -chip_base ${chip_full_offset} -seed  ${test_seed} -spifile  spi_load.txt -northfile host_load_north.txt -southfile host_load_south.txt -westfile host_load_west.txt -eastfile host_load_east.txt -vcdlevel  ${wave} -ncycles ${timeout} -n $ncores |& tee vmodel-run.${test_seed}.log
		
		#egrep '(PC=00000010 |PC=0000000c )' core_0_0.ptrace | awk 'BEGIN {print "ISR" } {print $4}'
		#grep rti core_0/test.dis | awk '{print $1}' | sed 's/://g' | awk ' BEGIN {printf("egrep   \"(PC=00000010|PC=0000000c"); }  { printf("|PC=00000%s", $1); } END {print")\"  core_0_0.ptrace "}'


		rm -fr host_addr_incoming_core_dut.mem
		cat host_addr_out_*.txt >! host_addr_incoming_core_dut.mem
		rm -fr host_addr_out_*.txt
		
		rm -fr host_addr_incoming_core_expected.mem
		cat core_*/sim_out.mem >! host_addr_incoming_core_expected.mem

		set mc_status = 0
		if( "${mc_off}" == "" ) then 	
			
			sort mc_dut_out*.txt >! mc_dut_out_sort.txt
			
			sort mc_data_out_expected.txt >! mc_data_out_expected_sort.txt
			rm -fr mc_data_out_expected.txt
			
			echo "Checking multicast  diff -q mc_dut_out_sort.txt mc_dut_out_sort.txt "
			
			diff -q mc_dut_out_sort.txt mc_data_out_expected_sort.txt
			set mc_status = ${status}
		 		
		endif
		rm -fr mc_dut_out*.txt
		
		rm -fr host_addr_incoming_core_expected_vs_dut.mem
		
		#check data and address	 
		comparegen ${no_check_for_garbage_out} |& tee ${test_seed}.error  #-dma_sr_region_address `epiphany-unknown-elf-readelf -s core_0/test.elf | grep DMA_INTERRUPT_BUFFER0 | awk '{print "0x"$2}'`
						
		if( ${status} != 0 || ${mc_status} != 0 ) then
			 echo ${test_seed} >> FAIL
			 gawk '{printf( "0x%.8x\n",strtonum($1))}' core_*/sim.log >! /tmp/${user}_sim.pc
			 awk '{print $4}' core_*_*.ptrace | awk -F'=' '{printf("0x%s\n", $2)}' > /tmp/${user}_verilator.pc
 			 #diff /tmp/${user}_sim.pc /tmp/${user}_verilator.pc
			 ( echo "Checker errors: " ; grep ERROR vmodel-run.${test_seed}.log )  >> ${test_seed}.error

			 tar czf ERROR_${test_seed}.tgz vmodel-run.${test_seed}.log ${test_seed}.error dump.vcd core_0_0.* core_*
			 
			exit 4
		else
			 gawk '{printf( "0x%.8x\n",strtonum($1))}' core_*/sim.log >! /tmp/${user}_sim.pc
			 awk '{print $4}' core_*_*.ptrace | awk -F'=' '{printf("0x%s\n", $2)}' > /tmp/${user}_verilator.pc
		
			rm -fr ${test_seed}.error vmodel-run.${test_seed}.log
			echo $a >> PASS
		endif
		
	
	 endif
	 

	 
	  #( echo "ffffffff ff"; cat core_*/mem_dump_out_full.txt ) | sort | gawk -f ${awk_path}/full.awk >! host_result.txt 
	 # gawk 'BEGIN { a =0; max_dj =0; } {  b=strtonum($1); if(abs(b-a) > max_dj) {max_dj=abs(b-a);}; a= strtonum($1) ;} function abs(a,b) { if(a>b) {return a-b} else {return b-a} } END{ printf( "%x\n",max_dj)} ' core_0/sim.log
	
end

touch FAIL 
cat FAIL


EXIT:
exit
  
USAGE: 
echo "usage runGen.sh [-tag] [-chip_id <id>] [-chip_id_random] [  [ [-chip_ext_off>] | [ -host_master_off] ]  [ -host_north_off ]  [-host_south_off] [-host_east_off] [-host_west_off]         [-host_off]  [-no_run] [ -wave] [-integer_on] [-no_dma] [-auto_dma_on]   [-hw_loop_on] [-no_inter] ([-no_fp]|[-fp_only]) [-mc_off] [-clock_gate_off] [-random_stop_resume_on | -bkpt_on] [-host_master_off]  [ -internal_only  ] [-c <number cores>=1]  [-nN <num_tests>=1] [ -seed n] [-timeout =160000] [-core_{0,1,2,3} core_num_in_mesh] [-core_mem_end_address <0xhex> ]"
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

exit
     
host off - don't use the host memory spave and drive host transaction
chip_ext_off - don't located the program data, code and dma buffers and control in host space






###########################################################################################################
GDB:
		#############
		#############
			 	
	 	echo "target remote :51000" >! gdb_x.cmd
	 	echo "load /home/oraikhman/RUN/core_0/test.elf" >> gdb_x.cmd
	 	set total_b = `grep -ci bkpt core_0/sim.log`
	 	set n_br = 0
	 	#echo "set logging on" >> gdb_x.cmd
	 	
	 	while ${n_br} != ${total_b} 
	 		@ n_br = ${n_br}  +  1
	 		echo "c" >> gdb_x.cmd
	 		echo "info reg pc" >> gdb_x.cmd
	 		if( ${n_br} != ${total_b}  ) then 
	 			echo "si" >> gdb_x.cmd
	 		endif	
	 		echo "info reg pc" >> gdb_x.cmd
	 	end
	 		
	 	echo "set logging on" >> gdb_x.cmd
	 	echo "info reg pc" >> gdb_x.cmd
	 	echo "kill" >> gdb_x.cmd
	 	echo "quit" >> gdb_x.cmd
	 	
	 	rm  -fr gdb.txt
		xterm -e "sleep 2; /home/oraikhman/maxwell_tools_build/objdir/gdb-6.8-a-obj/gdb/gdb --batch-silent --nw -x gdb_x.cmd; sleep 1" & #

	 	echo "Running model from ${model_path} /verilator  dummy.memh dummy.memh dummy.memh dummy.memh test42.vcd 1 ${wave} ${timeout}  40"
		####
		
		
		----	${model_path}/verilator  dummy.memh dummy.memh dummy.memh dummy.memh test42.vcd 1 ${wave} ${timeout}  10
		
		
		####
		sleep 3
		#exit
		set pc_run = `cat gdb.txt | head -1 | awk '{printf( "0x%x\n" , strtonum($2) ) }'`
		
		set pc_exp = `grep bkpt core_0/sim.log| tail -1  | awk '{printf( "0x%x\n" , 2 +  strtonum($1) )}'`
		
		if("${pc_run}" == "${pc_exp}" ) then
			echo "Great --- ${pc_exp} "
			set status = 0
		else
			echo "DIFF ${pc_exp} != ${pc_run}"
			set status = 233
			
		endif
		
		#exit
		
		#############
		#############
		

