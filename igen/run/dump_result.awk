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

function getCoreOffset64(coreNum) {
if( coreNum ==0  )  return 0x00000000
if( coreNum ==1  )  return 0x00100000
if( coreNum ==2  )  return 0x00200000
if( coreNum ==3  )  return 0x00300000
if( coreNum ==4  )  return 0x00400000
if( coreNum ==5  )  return 0x00500000
if( coreNum ==6  )  return 0x00600000
if( coreNum ==7  )  return 0x00700000
if( coreNum ==8  )  return 0x04000000
if( coreNum ==9  )  return 0x04100000
if( coreNum ==10  )  return 0x04200000
if( coreNum ==11  )  return 0x04300000
if( coreNum ==12  )  return 0x04400000
if( coreNum ==13  )  return 0x04500000
if( coreNum ==14  )  return 0x04600000
if( coreNum ==15  )  return 0x04700000
if( coreNum ==16  )  return 0x08000000
if( coreNum ==17  )  return 0x08100000
if( coreNum ==18  )  return 0x08200000
if( coreNum ==19  )  return 0x08300000
if( coreNum ==20  )  return 0x08400000
if( coreNum ==21  )  return 0x08500000
if( coreNum ==22  )  return 0x08600000
if( coreNum ==23  )  return 0x08700000
if( coreNum ==24  )  return 0x0c000000
if( coreNum ==25  )  return 0x0c100000
if( coreNum ==26  )  return 0x0c200000
if( coreNum ==27  )  return 0x0c300000
if( coreNum ==28  )  return 0x0c400000
if( coreNum ==29  )  return 0x0c500000
if( coreNum ==30  )  return 0x0c600000
if( coreNum ==31  )  return 0x0c700000
if( coreNum ==32  )  return 0x10000000
if( coreNum ==33  )  return 0x10100000
if( coreNum ==34  )  return 0x10200000
if( coreNum ==35  )  return 0x10300000
if( coreNum ==36  )  return 0x10400000
if( coreNum ==37  )  return 0x10500000
if( coreNum ==38  )  return 0x10600000
if( coreNum ==39  )  return 0x10700000
if( coreNum ==40  )  return 0x14000000
if( coreNum ==41  )  return 0x14100000
if( coreNum ==42  )  return 0x14200000
if( coreNum ==43  )  return 0x14300000
if( coreNum ==44  )  return 0x14400000
if( coreNum ==45  )  return 0x14500000
if( coreNum ==46  )  return 0x14600000
if( coreNum ==47  )  return 0x14700000
if( coreNum ==48  )  return 0x18000000
if( coreNum ==49  )  return 0x18100000
if( coreNum ==50  )  return 0x18200000
if( coreNum ==51  )  return 0x18300000
if( coreNum ==52  )  return 0x18400000
if( coreNum ==53  )  return 0x18500000
if( coreNum ==54  )  return 0x18600000
if( coreNum ==55  )  return 0x18700000
if( coreNum ==56  )  return 0x1c000000
if( coreNum ==57  )  return 0x1c100000
if( coreNum ==58  )  return 0x1c200000
if( coreNum ==59  )  return 0x1c300000
if( coreNum ==60  )  return 0x1c400000
if( coreNum ==61  )  return 0x1c500000
if( coreNum ==62  )  return 0x1c600000
if( coreNum ==63  )  return 0x1c700000
}

function getCoreOffset16(coreNum)
{

    if( coreNum ==0  )  return 0x00000000
    if( coreNum ==1  )  return 0x00100000
    if( coreNum ==2  )  return 0x00200000
    if( coreNum ==3  )  return 0x00300000
    if( coreNum ==4  )  return 0x04000000
    if( coreNum ==5  )  return 0x04100000 
    if( coreNum ==6  )  return 0x04200000 
    if( coreNum ==7  )  return 0x04300000
    if( coreNum ==8  )  return 0x08000000
    if( coreNum ==9  )  return 0x08100000
    if( coreNum ==10 )  return 0x08200000
    if( coreNum ==11 )  return 0x08300000
    if( coreNum ==12 )  return 0x0c000000
    if( coreNum ==13 )  return 0x0c100000
    if( coreNum ==14 )  return 0x0c200000
    if( coreNum ==15 )  return 0x0c300000
       
}


BEGIN {


	#set seed
	#printf("awk seed %d\n",iseed);
	srand(iseed);

    Idx= 0
    while ((getline      <  "test.elf_dump_header") > 0)  {
	if( Idx == 0 && $1 != 0 ) {
	    continue;
	}
	if(Idx == strtonum($1)   ) {
	    #print "PPP" $0
	    
	    #print Idx;
	    #print $4;
	    s_size_s = sprintf("0x%s",$3);
	    s_addr_s = sprintf("0x%s",$4);
	    
	    s_size = strtonum(s_size_s);
	    s_addr = strtonum(s_addr_s );
	    
	    #printf("### address -- 0x%x size 0x%x\n", s_addr , s_size);
	    if (s_addr  in Section_H) {
		
		print "ERRRR ..." s_addr
		exit(9);

	    } else {

		Section_H[s_addr]=s_size;
	    }
	    
	    Idx=Idx+1;  
	}
    }
    close("test.elf_dump_header");
    #exit
    readsection = 0;
    test_elf_res = "test.elf_dump"
    data_ind = 0;


    while ((getline      <  test_elf_res) > 0) {
	#print " ----- " $0
	
	if( match($0  ,"Contents of section output") ) {
	    readsection = 1;
	    continue;
	}
	if( readsection == 1 ) {
	    readsection = 2;
	    #read address
	    
	    #print  "\naddress "  $1
	    sd_addr_s = sprintf("0x%s",$1);
	    sd_addr = strtonum(sd_addr_s);


	    if ( sd_addr in Section_H) {

		
		readsection = 2;
		data_ind = 0;
		#printf "\n";
		#printf( "found address 0x%x\n", sd_addr);
		#printf ( "section size %d\n" , Section_H[sd_addr]); 

	    } else {
		print "ERRRR 2 ..." sd_addr
		exit(9);
		
	    }

	    

	}
	if( readsection == 2 ) {
	    
	    #printf( "addr %x : " , $1); 
	    for ( i = 2 ; i <= 5 ; i=i+1 ) {
		
		c_data = $i;
		for( j = 1 ; j <= length(c_data) ; j=j+2) {
		    
		    d_data = substr( c_data , j , 2 );
		    d_address = sd_addr+data_ind;

		    #printf("#### %x %s %x\n", d_address  ,d_data, Section_H[sd_addr]) ;

		    MEM[sd_addr+data_ind] = d_data;
		    data_ind = data_ind + 1;
		    
		}
		if( data_ind >= Section_H[sd_addr] ) {
		    
		    break;
		}
		
	    }
	    #print "---\n" data_ind "---\n";
	    # print  "-Section_H[sd_addr -- " Section_H[sd_addr]  "---\n";
	}
	
    }
    close( test_elf_res);


    n = asorti(MEM, MEM_SORT_IN)
    

    for (i = 1; i <= n; i++) {
	printf("%.8x %s\n",MEM_SORT_IN[i], MEM[MEM_SORT_IN[i]]) > "mem_dump_in.txt";
	if (strtonum(MEM_SORT_IN[i]) < 0x100000) {
	    printf("%.8x %s\n",strtonum(chip_offset) + strtonum(getCoreOffset64(strtonum(coreNum))) + strtonum(MEM_SORT_IN[i]), MEM[MEM_SORT_IN[i]]) > "mem_dump_in_full.txt";  
	} else {
	    printf("%.8x %s\n",MEM_SORT_IN[i], MEM[MEM_SORT_IN[i]]) > "mem_dump_in_full.txt"; 
	    
	}
    }


    #for (ind in MEM ) {
	#printf("%.8x %s\n", ind , MEM[ind]) ;
    #}

    close("mem_dump_in.txt");
}
{
 #PARSE THE SIM.LOG   

    if( match($2 , /str[dhb]*$/) || match($2 , /ldr[dhb]*$/) || match($2 , /testset*$/) ){
	#print $0;
	isTestAsetWriteToMem = 0;
	for ( i = 3 ; i <= NF ; i = i + 1) {
	    if(  $i ==  "memory" ||  $i ==  "registers") { 


		if( match($2 , /testset*$/) &&  $i ==  "memory"  ) {
		  m_addr_i = i - 4 ;  
		} else {
		  m_addr_i = i - 1 ; #m_addr_i = i + 2 ;   
		}
		
		
		m_data_i = i + 2 ; #m_data_i = i - 1;
		
		m_addr =  strtonum($m_addr_i) ;
		m_data =  sprintf("%.8x",strtonum(substr($m_data_i,1,length($m_data_i))));

		#print m_data;
		if( match($2 , /testset*$/)) {
		    
		    if($i ==  "memory") {
			isTestAsetWriteToMem = 1;#the register update will be followed by memory
			break;
		    } else {
			if(i+2==NF) {
			    break;
			}
			
		    }
		    
		} else {
		    break;
		}
	    }
	}

	
	if(  i - 1 == NF  )  {
	    print "ERRR ... 5 "
	    exit 5
	}
	if ( m_addr in MEM) {

	    if( match($2 , /testset*$/) )  {
		
		if(isTestAsetWriteToMem==1) {
		    MEM[m_addr]= substr(m_data,7,2);
		    MEM[m_addr+1]= substr(m_data,5,2);			
		    MEM[m_addr+2]= substr(m_data,3,2);
		    MEM[m_addr+3]= substr(m_data,1,2);
		}

		
		SIM_LOG[m_addr+1]= substr(m_data,5,2);
		SIM_LOG[m_addr]= substr(m_data,7,2);
		SIM_LOG[m_addr+2]= substr(m_data,3,2);
		SIM_LOG[m_addr+3]= substr(m_data,1,2);	
		
		

	    }	 	    
	    if( match($2 , /str[dhb]*$/) )  {
		
		MEM[m_addr]= substr(m_data,7,2);
		SIM_LOG[m_addr]= substr(m_data,7,2);
		
		printf( "%x\n",m_addr) > "sim_out.mem" 
		
	    }
	    if( match($2 , /str[dh]*$/) )  {
		MEM[m_addr+1]= substr(m_data,5,2);
			SIM_LOG[m_addr+1]= substr(m_data,5,2);
	    }
	    if( match($2 , /str[d]*$/) )  {		
		MEM[m_addr+2]= substr(m_data,3,2);
		MEM[m_addr+3]= substr(m_data,1,2);
		
		SIM_LOG[m_addr+2]= substr(m_data,3,2);
		SIM_LOG[m_addr+3]= substr(m_data,1,2);	
		
	    }
	    if( match($2 , /strd$/) )  {		#double
	   	
	   	
		if( m_addr+4 in MEM ) {
		    i = i + 6;
		    m_addr_i = i - 1 ; #m_addr_i = i + 2 ; 
		    m_data_i = i + 2;  #m_data_i = i - 1;
		    m_addr =  strtonum($m_addr_i) ;
		    
		    
		    m_data =  sprintf("%.8x",strtonum(substr($m_data_i,1,length($m_data_i))));
		    
		    MEM[m_addr+0]= substr(m_data,7,2);
		    MEM[m_addr+1]= substr(m_data,5,2);
		    MEM[m_addr+2]= substr(m_data,3,2);
		    MEM[m_addr+3]= substr(m_data,1,2);
		    
		    SIM_LOG[m_addr+0]= substr(m_data,7,2);
		    SIM_LOG[m_addr+1]= substr(m_data,5,2);
		    SIM_LOG[m_addr+2]= substr(m_data,3,2);
		    SIM_LOG[m_addr+3]= substr(m_data,1,2);	
				
		} else {
		    printf( "ERRR ... 6 double address %x\n",m_addr);
	    		exit 6
		}
	    }
	    
	} else {
	    printf( "ERRR m_addr in MEM ... 6 address %x\n",m_addr);
	    exit 6
	}

    }

}


END {
 	#parse DMA file
	dma_out = "dma_out.txt"
	
	dma_tran_log = "dma_tran_log.txt"
	
	printf("------------------------------\n") >  dma_tran_log
	
	currTran=0;
	packetData63_32 ="";
	packetData31_0 = "";
	
 	while ((getline      <  dma_out) > 0) {
 	
 		srcAddr = strtonum($1);
 		dstAddr = strtonum($2);
 		


		if ( ( srcAddr in MEM ) && ( dstAddr in MEM ) ) { 
		
			MEM[dstAddr] = MEM[srcAddr];
			
			printf("[%x]=%s ===> [%x]=%s\n",srcAddr,MEM[srcAddr],dstAddr,MEM[dstAddr]) > dma_tran_log		
			
		} else {
			#printf("slave ---> DMA %x \n" , srcAddr);
			if( (srcAddr == 0x000F0514 || srcAddr ==  0x000F0534) && ( dstAddr in MEM ) ) {
				tranSize=1;
				for(j=0;j<strtonum($3);j++) {
					tranSize = tranSize * 2 ;
				}
				
				if(currTran==0) {
					autoDmaDstAddr=dstAddr;
				}
				
				#printf ("autodma %x\n",srcAddr)> "mem_auto_dma_out_full.txt";
				dma_data = 256 * rand(); 
				
				MEM[dstAddr] = sprintf("%.2x",strtonum(dma_data));
				
				if(currTran>3) {
					packetData63_32 = sprintf("%s%s",MEM[dstAddr],packetData63_32);
				} else {
					packetData31_0 = sprintf("%s%s",MEM[dstAddr],packetData31_0);
				}
				
						
				#printf("%.8x ",(strtonum(chip_offset) + strtonum(getCoreOffset64(strtonum(coreNum))) + srcAddr) ) > "host_auto_dma_fill.txt";
				if( dstAddr < 0x100000) {
					#printf("%.8x %.2x\n",(strtonum(chip_offset) + strtonum(getCoreOffset64(strtonum(coreNum))) + dstAddr) , strtonum(dma_data)) > "host_auto_dma_fill.txt";					
				} else {
					#printf("%.8x %.2x\n",(dstAddr) , strtonum(dma_data)) > "host_auto_dma_fill.txt";
					printf("ERROR : dstAddr DMA addr ( XXX --> %x ) is not internal\n",dstAddr)
					exit 68
				}	
				
				if(currTran+1==tranSize) {
					#printf("---------%.8x---%.8x\n", strtonum(sprintf("0x%s",packetData63_32)),strtonum(sprintf("0x%s",packetData31_0))) > "host_auto_dma_fill.txt";	
					printf("%.8x_%.8x_%.8x_0_%x\n",strtonum(sprintf("0x%s",packetData63_32)),strtonum(sprintf("0x%s",packetData31_0)),
							(strtonum(chip_offset) + strtonum(getCoreOffset64(strtonum(coreNum))) + srcAddr), 4*strtonum($3)+1) > "host_auto_dma_fill.txt";
					packetData63_32="";packetData31_0="";
					currTran=0;
				} else {
		
					currTran=currTran+1;
				}
				
			} else {
				printf("ERROR : DMA addr ( %x --> %x ) not in range\n",srcAddr,dstAddr)
				exit 67
			}
		}
		

		
		
	}
	close(dma_out);



    n = asorti(MEM, MEM_SORT_OUT)


    for (i = 1; i <= n; i++) {
	printf("%.8x %s\n",MEM_SORT_OUT[i], MEM[MEM_SORT_OUT[i]]) > "mem_dump_out.txt";
	if (strtonum(MEM_SORT_OUT[i]) < 0x100000) {
	    printf("%.8x %s\n",strtonum(chip_offset) + strtonum(getCoreOffset64(strtonum(coreNum))) + strtonum(MEM_SORT_OUT[i]), MEM[MEM_SORT_OUT[i]]) > "mem_dump_out_full.txt";  
	} else {
	    printf("%.8x %s\n",MEM_SORT_OUT[i], MEM[MEM_SORT_OUT[i]]) > "mem_dump_out_full.txt"; 
	    
	}
    }

    close("mem_dump_out.txt");
   
   n = asorti(SIM_LOG, SIM_LOG_SORT)
   for (addr_sim in SIM_LOG_SORT) {
   		printf("%.8x %.2x\n",(addr_sim) , strtonum(SIM_LOG_SORT[addr_sim])) > "addr_sim.log";
   }
  
   
    
}