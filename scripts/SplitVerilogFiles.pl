#! /usr/bin/perl
require "/home/aolofsson/Bin/GeneralSubroutines.pl";
use Getopt::Long;
$Usage =<<EOF;
##########################################################################
#Function  : Silly script for splitting modules in one file into many
#Author    : Andreas Olofsson
#License   : BSD 3-Clause License (see bottom of source file)
##########################################################################
Usage: SplitVerilogFiles.pl   -v     <Verilog>
 	                      -dir   <Directory
###########################################################################
EOF
$result = GetOptions( 'v:s', 'dir:s');

if ((!defined $opt_v) | (!(defined $opt_dir))){
  printf $Usage;
  exit;
}
$VerilogFile=$opt_v;
$Dir=$opt_dir;
&split_verilog_files($VerilogFile,"$Dir");
sub split_verilog_files {
    my $File      =$_[0];
    my $Directory =$_[1];  
    open(FILE,$File);
    while(<FILE>){
	if(/^\s*module\s*(\w+)/){
	    $Module=$1;
	    open(FILEOUT,">$Directory/$Module.v");
	    print FILEOUT $_;
	}
	elsif(/endmodule/){
	    print FILEOUT $_;
	    close(FILEOUT);	    
	}
	else{
	    print FILEOUT $_;
	}
    }
close(FILE);
}
##################################################################
#
#Copyright (c) 2009, Andreas Olofsson, Adapteva, Inc.
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






