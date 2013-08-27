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

#ifndef _DV__H_
#define _DV__H_

#define  MAXPROGRAM   100000 //100k lines Max program
#define  MAXLINE      100   //MAX LINE SIZE INCLUDING COMMENTS






#define CORE_SPACE	  0x00100000
#define CORE_ADDR_SPACE_MASK 0xfffff

#define BKPT_INSTR_OPCODE 0x01c2
#define TRAP_INSTR_OPCODE 0x03e2

#define DV_BKPT_INSTR_OPCODE 0x05c2
#define M_BKPT_INSTR_OPCODE 0x07c2

#define NORTH_BASE_DEFAULT    0x062000000
#define SOUTH_BASE_DEFAULT     0x0a2000000
#define WEST_BASE_DEFAULT    0x081800000
#define EAST_BASE_DEFAULT    0x82400000
#define CHIP_BASE_DEFAULT  0x82000000



#define MMR_START    0xF0000



#define SPI_BACK_CORE_ROW_OFFSET 0
#define SPI_BACK_CORE_COL_OFFSET 1

#define CORE_R0       0x000F0000

#define CORE_R1       CORE_R0 + 4*1
#define CORE_R2       CORE_R0 + 4*2
#define CORE_R3       CORE_R0 + 4*3

#define CORE_R16      CORE_R0 + 4*16
#define CORE_R17      CORE_R0 + 4*17
#define CORE_R18      CORE_R0 + 4*18
#define CORE_R19      CORE_R0 + 4*19

#define CORE_R32      CORE_R0 + 4*32
#define CORE_R48      CORE_R0 + 4*48

	// Offsets into the GPRs of various "special" registers
#define ATDSP_GPR_SB   9	//!< Offset to static base register
#define ATDSP_GPR_SL  10	//!< Offset to stack limit register
#define ATDSP_GPR_FP  11	//!< Offset to frame pointer register
#define ATDSP_GPR_IP  12	//!< Offset to inter-proc scratch reg
#define ATDSP_GPR_SP  13	//!< Offset to stack pointer register
#define ATDSP_GPR_LR  14	//!< Offset to link register

	// Offsets into SCRs
#define ATDSP_SCR_CONFIG  0  //!< Offset to config register
#define ATDSP_SCR_STATUS  1  //!< Offset to status register
#define ATDSP_SCR_PC      2  //!< Offset to program counter reg
#define ATDSP_SCR_DEBUG   3  //!< Offset to debug register
#define ATDSP_SCR_IAB     4
#define ATDSP_SCR_LC      5
#define ATDSP_SCR_LS      6
#define ATDSP_SCR_LE      7
#define ATDSP_SCR_IRET    8  //!< Offset to interrupt return reg
#define ATDSP_SCR_IMASK   9
#define ATDSP_SCR_ILAT    10
#define ATDSP_SCR_ILATST  11
#define ATDSP_SCR_ILATCL  12
#define ATDSP_SCR_IPEND   13
#define ATDSP_SCR_CTIMER0 14
#define ATDSP_SCR_CTIMER1 15
#define ATDSP_SCR_HSTATUS 16
#define ATDSP_SCR_HSCONFIG 17
#define ATDSP_SCR_DEBUGCMD 18

#define ATDSP_SCR_DMA0_CONFIG     0
#define ATDSP_SCR_DMA0_STRIDE     1
#define ATDSP_SCR_DMA0_COUNT     2
#define ATDSP_SCR_DMA0_SRCADDR     3
#define ATDSP_SCR_DMA0_DSTADDR     4
#define ATDSP_SCR_DMA0_AUTODMA0     5
#define ATDSP_SCR_DMA0_AUTODMA1     6
#define ATDSP_SCR_DMA0_STATUS     7

#define ATDSP_SCR_DMA1_CONFIG     8
#define ATDSP_SCR_DMA1_STRIDE     9
#define ATDSP_SCR_DMA1_COUNT     10
#define ATDSP_SCR_DMA1_SRCADDR     11
#define ATDSP_SCR_DMA1_DSTADDR     12
#define ATDSP_SCR_DMA1_AUTODMA0     13
#define ATDSP_SCR_DMA1_AUTODMA1     14
#define ATDSP_SCR_DMA1_STATUS     15

#define ATDSP_SCR_MESH_CONFIG     0
#define ATDSP_SCR_MESH_STATUS     1
#define ATDSP_SCR_MESH_MULTICAST  2
#define ATDSP_SCR_MESH_SWRESET    3

#define SCR_1_OFFSET      0x400
#define CORE_CONFIG       SCR_1_OFFSET + MMR_START + (ATDSP_SCR_CONFIG<<2)
#define CORE_STATUS		  SCR_1_OFFSET + MMR_START + (ATDSP_SCR_STATUS<<2)
#define CORE_PC		      SCR_1_OFFSET + MMR_START + (ATDSP_SCR_PC<<2)
#define CORE_DEBUG		  SCR_1_OFFSET + MMR_START + (ATDSP_SCR_DEBUG<<2)
#define CORE_ILAT         SCR_1_OFFSET + MMR_START + (ATDSP_SCR_ILAT<<2)
#define CORE_IMASK         SCR_1_OFFSET + MMR_START + (ATDSP_SCR_IMASK<<2)
#define CORE_HSTATUS      SCR_1_OFFSET + MMR_START + (ATDSP_SCR_HSTATUS<<2)
#define CORE_HCONFIG      SCR_1_OFFSET + MMR_START + (ATDSP_SCR_HSCONFIG<<2)

#define CORE_DEBUGCMD      SCR_1_OFFSET + MMR_START + (ATDSP_SCR_DEBUGCMD<<2)

#define CORE_CTIMER0         SCR_1_OFFSET + MMR_START + (ATDSP_SCR_CTIMER0<<2)
#define CORE_CTIMER1         SCR_1_OFFSET + MMR_START + (ATDSP_SCR_CTIMER1<<2)
#define CORE_IPEND         SCR_1_OFFSET + MMR_START + (ATDSP_SCR_IPEND<<2)
#define CORE_ILATST         SCR_1_OFFSET + MMR_START + (ATDSP_SCR_ILATST<<2)





#define SCR_2_OFFSET      0x500
#define DMA0_CONFIG       SCR_2_OFFSET + MMR_START + (ATDSP_SCR_DMA0_CONFIG<<2)
#define DMA0_STATUS       SCR_2_OFFSET + MMR_START + (ATDSP_SCR_DMA0_STATUS<<2)
#define DMA0_AUTODMA0     SCR_2_OFFSET + MMR_START + (ATDSP_SCR_DMA0_AUTODMA0<<2)


#define DMA1_CONFIG       SCR_2_OFFSET + MMR_START + (ATDSP_SCR_DMA1_CONFIG<<2)
#define DMA1_STATUS       SCR_2_OFFSET + MMR_START + (ATDSP_SCR_DMA1_STATUS<<2)
#define DMA1_AUTODMA0     SCR_2_OFFSET + MMR_START + (ATDSP_SCR_DMA1_AUTODMA0<<2)

#define SCR_3_OFFSET      0x600

#define SCR_4_OFFSET      0x700
#define MESH_CONFIG      SCR_4_OFFSET + MMR_START + (ATDSP_SCR_MESH_CONFIG<<2)
#define MESH_COREID      SCR_4_OFFSET + MMR_START + (ATDSP_SCR_MESH_STATUS<<2)
#define MESH_MULTICAST   SCR_4_OFFSET + MMR_START + (ATDSP_SCR_MESH_MULTICAST<<2)
#define MESH_SWRESET     SCR_4_OFFSET + MMR_START + (ATDSP_SCR_MESH_SWRESET<<2)


#define PAGE_MISS_ILAT_BIT 0x4
#define DEBUG_HALT 0x1
#define DEBUG_RESUME 0x0
#define OUT_TRAN_FALSE 0x0
extern unsigned NCORES;



#define CORE_IDLE_BIT 0x1
#define CORE_IDLE_VAL 0x0

#define        DMA_IDLE       0 //dma is idle(can be reprogrammed)
#define        DMA_ACTIVE   5 //active transaction stage(master or slave)
#define        DMA_SLAVE_READY    6 //slave mode, autodma ready to receive
//#define        DMA_ERROR    11  //autodma buffer over-run error
#define        DMA_ERROR    0xd  //autodma buffer over-run error
/*
 * bits [19:16] = f (MMR space)
bits [10:8] = 3
bits [7:2] = 6'h3f
 */

#define       SPI_RETURN_ADDR  (0xf << 16) | (0x3<<8) | (0x3f<<2)
#endif

/*
 * REGISTER-NAME	STAUS	ADDRESS	FUNCTION	GROUP	ACCESS	SUB-FIELDS	FOR USER	NOTE
CONFIG	CHANGED	0x400	configuration register	CORE	RD/WR	[0]=truncate mode (0=round to nearest, 1=trunacte)	YES
						[1]=invalid exception enable	YES
						[2]=overflow exception enable	YES
						[3]=underflow exception enable	YES
						[7:4] = timer0 counting mode:         0000=off         0001=clk         0010=idle cycles         0011=clk64 (64 bit carry en, used for timer1 only)         0100=ialu valid         0101=fpu valid         0110=dual issue         0111=e1-stalls         1000=ra stalls         1001=iab stalls??         1010=fetch contention stalls         1011=FREE         1100=ext fetch stalls         1101=ext load stalls         1110=ext event0         1111=ext event1	YES
						[11:8]=timer1 timer mode(can be used as upper part of 64 bit counter)	YES	the output of timer0 can be used as a carry in for timer1 by setting the mode of timer 1 to “clk64”
						[15:12] = mesh routing mode for external load/store	YES
						[16]      = split register file into 32 fpu/32 pointers	NO	for later...
						[19:17] = arithmetic mode            000=float-32            001=float-64            010=fract-mac16            011=fract-complex16            100=integer-32            101=integer-64            110=integer-16            111=integer-8	NO	for later...
						[20]       = emulation mode (stops iab prefetching)	NO
						[21]      = single issue mode	NO
						[27:22] = RESERVED	NO
						[31:28] =  clock divider ratio            0000=no division            0001=divide by 2            0010=divide by 4            Etc	YES
STATUS	CHANGED	0x404	status register	CORE	RD/WR*	[0]=core active indicator	YES
						[1]=global interrupt disable	YES
						[2]=processor mode(1=user mode, 0=kernel mode)	YES	might be useful for stupid programmers even without multiuser OS?
						[3]=wired AND global flag	YES
						[4]=integer zero	YES
						[5]=integer negative	YES
						[6]=integer carry	YES
						[7]=integer overflow	YES
						[8]=fpu zero flag	YES
						[9]=fpu negative flag	YES
						[10]=fpu overflow flag	YES
						[11]=fpu carry flag(not used)	YES	will be used by integer unit!!
						[12]=ialu overflow flag(sticky)	YES
						[13]=fpu invalid flag(sticky)	YES
						[14]=fpu overflow flag(sticky)\	YES
						[15]=fpu underflow flag(sticky)\	YES
						[17:16]=exception cause 00=no exception 01=load-store exception 10=fpu exception 11=unimplemented instruction	YES
						[18]=external load stalled
						[19]=external fetch stalled
						[31:20]=RESERVED
PC		0x408	program counter	CORE	RD/WR*		NO
DEBUG		0x40C	debug register	CORE	RD/WR		NO	used by debugger tools only
IAB		0x410	iab fifo register for external fetch	CORE	WR*		NO	I don't see a general use for this now...
LC		0x414	hw loop counter	CORE	RD/WR		YES
LS		0x418	hardware loop start	CORE	RD/WR		YES
LE		0x41C	hardware loop end	CORE	RD/WR		YES
IRET		0x420	interrupt return register	CORE	RD/WR		YES
IMASK		0x424	interrupt masking register	CORE	RD/WR		YES
ILAT		0x428	interrupt latch register	CORE	RD/WR*		YES
ILATST		0x42C	interrupt latch register bit set	CORE	WR		YES
ILATCL		0x430	interrupt latch register bit clear	CORE	WR		YES
IPEND		0x434	interrupt servicing interrupts	CORE	RD/WR*		NO
CTIMER0		0x438	event counter	CORE	RD/WR		YES
CTIMER1	NEW	0x43C	event counter	CORE	RD/WR		YES
HSTATUS	NEW	0x440	hidden status register	CORE	WR		NO
RESERVED		0x444-0x4FF
DMA0_CONFIG	CHANGED	0x500	configuration register	DMA	RD/WR	[0]=dma enable	YES
						[1]=dma master mode	YES
						[2]=dma chain mode(descriptor fetched at end)	YES
						[3]=2d dma mode	YES
						[5:4]=interrupt enables for inner and outer loop	YES
						[8:6]=datamode   000=byte   001=short   010=word   011=double,   Others reserved	YES
						[11:9]=dma throttle mode    000=no burst    001=2-burst    010=8-burst    011=32 burst   100=128 burst   101=512 burst   110=2048 burst   111=inifinite burst	YES	arbiters held while in burst
						[15:12]=ctrlmode for dma transactions	YES
						[31:16]=next descriptor pointer	YES
DMA0_STRIDE		0x504	stride register	DMA	RD/WR	[15:0]=source stride	YES
						[15:0]=destination stride	YES
DMA0_COUNT		0x508	count register	DMA	RD/WR	[15:0]=inner loop counter	YES
						[31:16]=outer loop counter	YES
DMA0_SRCADDR		0x50C	source register	DMA	RD/WR		YES
DMA0_DSTADDR		0x510	destination register	DMA	RD/WR		YES
DMA0_AUTODMA0		0x514	autodma register	DMA	RD/WR		YES
DMA0_AUTODMA1		0x518	autodma register	DMA	RD/WR*		YES
DMA0_STATUS		0x51C	dma status register	DMA	RD/WR*	[3:0]=dma state	YES
						[15:4]=reserved	YES
						[31:16]=current dma descriptor pointer	YES
DMA1_CONFIG		0x520	(samed as dma0*)
DMA1_STRIDE		0x524
DMA1_COUNT		0x528
DMA1_SRCADDR		0x52C
DMA1_DSTADDR		0x530
DMA1_AUTODMA0		0x534
DMA1_AUTODMA1		0x538
DMA1_STATUS		0x53C
RESERVED		0x540-0x5FF
MEM_CONFIG	NEW	0x600		MEMORY		[4:0]=memory speed setting	NO
RESERVED		0x540-0x5FF
MESH_CONFIG	NEW	0x600	mesh control register	MESH	RD/WR	[0]=round robin enable	YES
						[11:1]=RESERVED	YES
						[12]=blocks signal propagation to right	YES
						[13]=blocks signal propagation to left	YES
						[14]=blocks signal propagation to the top	YES
						[15]=blocks signal propagation to the bottom
MESH_STATUS	NEW	0x604	mesh statys and ID register	MESH	RD	[11:0]=core-id		note, ID is in this register!!
						[15:12]=reserved
MESH_MULTICAST	NEW	0x608	multicast address	MESH	RD/WR	[11:0]=multicast address	YES
MESH_SWRESET	NEW	0x60C	core specific software reset		WR	[0]=sw reset	NO	writing a one puts core in reset state.  To get out of reset state, write a 0
RESERVED		0x610-0x6FF
 *
 *
 */
