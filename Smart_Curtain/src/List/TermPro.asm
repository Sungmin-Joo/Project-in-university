
;CodeVisionAVR C Compiler V2.05.0 Professional
;(C) Copyright 1998-2010 Pavel Haiduc, HP InfoTech s.r.l.
;http://www.hpinfotech.com

;Chip type                : ATmega128
;Program type             : Application
;Clock frequency          : 14.745600 MHz
;Memory model             : Small
;Optimize for             : Size
;(s)printf features       : int, width
;(s)scanf features        : int, width
;External RAM size        : 0
;Data Stack size          : 1024 byte(s)
;Heap size                : 0 byte(s)
;Promote 'char' to 'int'  : Yes
;'char' is unsigned       : Yes
;8 bit enums              : No
;global 'const' stored in FLASH: Yes
;Enhanced core instructions    : On
;Smart register allocation     : On
;Automatic register allocation : On

	#pragma AVRPART ADMIN PART_NAME ATmega128
	#pragma AVRPART MEMORY PROG_FLASH 131072
	#pragma AVRPART MEMORY EEPROM 4096
	#pragma AVRPART MEMORY INT_SRAM SIZE 4351
	#pragma AVRPART MEMORY INT_SRAM START_ADDR 0x100

	#define CALL_SUPPORTED 1

	.LISTMAC
	.EQU UDRE=0x5
	.EQU RXC=0x7
	.EQU USR=0xB
	.EQU UDR=0xC
	.EQU SPSR=0xE
	.EQU SPDR=0xF
	.EQU EERE=0x0
	.EQU EEWE=0x1
	.EQU EEMWE=0x2
	.EQU EECR=0x1C
	.EQU EEDR=0x1D
	.EQU EEARL=0x1E
	.EQU EEARH=0x1F
	.EQU WDTCR=0x21
	.EQU MCUCR=0x35
	.EQU RAMPZ=0x3B
	.EQU SPL=0x3D
	.EQU SPH=0x3E
	.EQU SREG=0x3F
	.EQU XMCRA=0x6D
	.EQU XMCRB=0x6C

	.DEF R0X0=R0
	.DEF R0X1=R1
	.DEF R0X2=R2
	.DEF R0X3=R3
	.DEF R0X4=R4
	.DEF R0X5=R5
	.DEF R0X6=R6
	.DEF R0X7=R7
	.DEF R0X8=R8
	.DEF R0X9=R9
	.DEF R0XA=R10
	.DEF R0XB=R11
	.DEF R0XC=R12
	.DEF R0XD=R13
	.DEF R0XE=R14
	.DEF R0XF=R15
	.DEF R0X10=R16
	.DEF R0X11=R17
	.DEF R0X12=R18
	.DEF R0X13=R19
	.DEF R0X14=R20
	.DEF R0X15=R21
	.DEF R0X16=R22
	.DEF R0X17=R23
	.DEF R0X18=R24
	.DEF R0X19=R25
	.DEF R0X1A=R26
	.DEF R0X1B=R27
	.DEF R0X1C=R28
	.DEF R0X1D=R29
	.DEF R0X1E=R30
	.DEF R0X1F=R31

	.EQU __SRAM_START=0x0100
	.EQU __SRAM_END=0x10FF
	.EQU __DSTACK_SIZE=0x0400
	.EQU __HEAP_SIZE=0x0000
	.EQU __CLEAR_SRAM_SIZE=__SRAM_END-__SRAM_START+1

	.MACRO __CPD1N
	CPI  R30,LOW(@0)
	LDI  R26,HIGH(@0)
	CPC  R31,R26
	LDI  R26,BYTE3(@0)
	CPC  R22,R26
	LDI  R26,BYTE4(@0)
	CPC  R23,R26
	.ENDM

	.MACRO __CPD2N
	CPI  R26,LOW(@0)
	LDI  R30,HIGH(@0)
	CPC  R27,R30
	LDI  R30,BYTE3(@0)
	CPC  R24,R30
	LDI  R30,BYTE4(@0)
	CPC  R25,R30
	.ENDM

	.MACRO __CPWRR
	CP   R@0,R@2
	CPC  R@1,R@3
	.ENDM

	.MACRO __CPWRN
	CPI  R@0,LOW(@2)
	LDI  R30,HIGH(@2)
	CPC  R@1,R30
	.ENDM

	.MACRO __ADDB1MN
	SUBI R30,LOW(-@0-(@1))
	.ENDM

	.MACRO __ADDB2MN
	SUBI R26,LOW(-@0-(@1))
	.ENDM

	.MACRO __ADDW1MN
	SUBI R30,LOW(-@0-(@1))
	SBCI R31,HIGH(-@0-(@1))
	.ENDM

	.MACRO __ADDW2MN
	SUBI R26,LOW(-@0-(@1))
	SBCI R27,HIGH(-@0-(@1))
	.ENDM

	.MACRO __ADDW1FN
	SUBI R30,LOW(-2*@0-(@1))
	SBCI R31,HIGH(-2*@0-(@1))
	.ENDM

	.MACRO __ADDD1FN
	SUBI R30,LOW(-2*@0-(@1))
	SBCI R31,HIGH(-2*@0-(@1))
	SBCI R22,BYTE3(-2*@0-(@1))
	.ENDM

	.MACRO __ADDD1N
	SUBI R30,LOW(-@0)
	SBCI R31,HIGH(-@0)
	SBCI R22,BYTE3(-@0)
	SBCI R23,BYTE4(-@0)
	.ENDM

	.MACRO __ADDD2N
	SUBI R26,LOW(-@0)
	SBCI R27,HIGH(-@0)
	SBCI R24,BYTE3(-@0)
	SBCI R25,BYTE4(-@0)
	.ENDM

	.MACRO __SUBD1N
	SUBI R30,LOW(@0)
	SBCI R31,HIGH(@0)
	SBCI R22,BYTE3(@0)
	SBCI R23,BYTE4(@0)
	.ENDM

	.MACRO __SUBD2N
	SUBI R26,LOW(@0)
	SBCI R27,HIGH(@0)
	SBCI R24,BYTE3(@0)
	SBCI R25,BYTE4(@0)
	.ENDM

	.MACRO __ANDBMNN
	LDS  R30,@0+(@1)
	ANDI R30,LOW(@2)
	STS  @0+(@1),R30
	.ENDM

	.MACRO __ANDWMNN
	LDS  R30,@0+(@1)
	ANDI R30,LOW(@2)
	STS  @0+(@1),R30
	LDS  R30,@0+(@1)+1
	ANDI R30,HIGH(@2)
	STS  @0+(@1)+1,R30
	.ENDM

	.MACRO __ANDD1N
	ANDI R30,LOW(@0)
	ANDI R31,HIGH(@0)
	ANDI R22,BYTE3(@0)
	ANDI R23,BYTE4(@0)
	.ENDM

	.MACRO __ANDD2N
	ANDI R26,LOW(@0)
	ANDI R27,HIGH(@0)
	ANDI R24,BYTE3(@0)
	ANDI R25,BYTE4(@0)
	.ENDM

	.MACRO __ORBMNN
	LDS  R30,@0+(@1)
	ORI  R30,LOW(@2)
	STS  @0+(@1),R30
	.ENDM

	.MACRO __ORWMNN
	LDS  R30,@0+(@1)
	ORI  R30,LOW(@2)
	STS  @0+(@1),R30
	LDS  R30,@0+(@1)+1
	ORI  R30,HIGH(@2)
	STS  @0+(@1)+1,R30
	.ENDM

	.MACRO __ORD1N
	ORI  R30,LOW(@0)
	ORI  R31,HIGH(@0)
	ORI  R22,BYTE3(@0)
	ORI  R23,BYTE4(@0)
	.ENDM

	.MACRO __ORD2N
	ORI  R26,LOW(@0)
	ORI  R27,HIGH(@0)
	ORI  R24,BYTE3(@0)
	ORI  R25,BYTE4(@0)
	.ENDM

	.MACRO __DELAY_USB
	LDI  R24,LOW(@0)
__DELAY_USB_LOOP:
	DEC  R24
	BRNE __DELAY_USB_LOOP
	.ENDM

	.MACRO __DELAY_USW
	LDI  R24,LOW(@0)
	LDI  R25,HIGH(@0)
__DELAY_USW_LOOP:
	SBIW R24,1
	BRNE __DELAY_USW_LOOP
	.ENDM

	.MACRO __GETD1S
	LDD  R30,Y+@0
	LDD  R31,Y+@0+1
	LDD  R22,Y+@0+2
	LDD  R23,Y+@0+3
	.ENDM

	.MACRO __GETD2S
	LDD  R26,Y+@0
	LDD  R27,Y+@0+1
	LDD  R24,Y+@0+2
	LDD  R25,Y+@0+3
	.ENDM

	.MACRO __PUTD1S
	STD  Y+@0,R30
	STD  Y+@0+1,R31
	STD  Y+@0+2,R22
	STD  Y+@0+3,R23
	.ENDM

	.MACRO __PUTD2S
	STD  Y+@0,R26
	STD  Y+@0+1,R27
	STD  Y+@0+2,R24
	STD  Y+@0+3,R25
	.ENDM

	.MACRO __PUTDZ2
	STD  Z+@0,R26
	STD  Z+@0+1,R27
	STD  Z+@0+2,R24
	STD  Z+@0+3,R25
	.ENDM

	.MACRO __CLRD1S
	STD  Y+@0,R30
	STD  Y+@0+1,R30
	STD  Y+@0+2,R30
	STD  Y+@0+3,R30
	.ENDM

	.MACRO __POINTB1MN
	LDI  R30,LOW(@0+(@1))
	.ENDM

	.MACRO __POINTW1MN
	LDI  R30,LOW(@0+(@1))
	LDI  R31,HIGH(@0+(@1))
	.ENDM

	.MACRO __POINTD1M
	LDI  R30,LOW(@0)
	LDI  R31,HIGH(@0)
	LDI  R22,BYTE3(@0)
	LDI  R23,BYTE4(@0)
	.ENDM

	.MACRO __POINTW1FN
	LDI  R30,LOW(2*@0+(@1))
	LDI  R31,HIGH(2*@0+(@1))
	.ENDM

	.MACRO __POINTD1FN
	LDI  R30,LOW(2*@0+(@1))
	LDI  R31,HIGH(2*@0+(@1))
	LDI  R22,BYTE3(2*@0+(@1))
	LDI  R23,BYTE4(2*@0+(@1))
	.ENDM

	.MACRO __POINTB2MN
	LDI  R26,LOW(@0+(@1))
	.ENDM

	.MACRO __POINTW2MN
	LDI  R26,LOW(@0+(@1))
	LDI  R27,HIGH(@0+(@1))
	.ENDM

	.MACRO __POINTBRM
	LDI  R@0,LOW(@1)
	.ENDM

	.MACRO __POINTWRM
	LDI  R@0,LOW(@2)
	LDI  R@1,HIGH(@2)
	.ENDM

	.MACRO __POINTBRMN
	LDI  R@0,LOW(@1+(@2))
	.ENDM

	.MACRO __POINTWRMN
	LDI  R@0,LOW(@2+(@3))
	LDI  R@1,HIGH(@2+(@3))
	.ENDM

	.MACRO __POINTWRFN
	LDI  R@0,LOW(@2*2+(@3))
	LDI  R@1,HIGH(@2*2+(@3))
	.ENDM

	.MACRO __GETD1N
	LDI  R30,LOW(@0)
	LDI  R31,HIGH(@0)
	LDI  R22,BYTE3(@0)
	LDI  R23,BYTE4(@0)
	.ENDM

	.MACRO __GETD2N
	LDI  R26,LOW(@0)
	LDI  R27,HIGH(@0)
	LDI  R24,BYTE3(@0)
	LDI  R25,BYTE4(@0)
	.ENDM

	.MACRO __GETB1MN
	LDS  R30,@0+(@1)
	.ENDM

	.MACRO __GETB1HMN
	LDS  R31,@0+(@1)
	.ENDM

	.MACRO __GETW1MN
	LDS  R30,@0+(@1)
	LDS  R31,@0+(@1)+1
	.ENDM

	.MACRO __GETD1MN
	LDS  R30,@0+(@1)
	LDS  R31,@0+(@1)+1
	LDS  R22,@0+(@1)+2
	LDS  R23,@0+(@1)+3
	.ENDM

	.MACRO __GETBRMN
	LDS  R@0,@1+(@2)
	.ENDM

	.MACRO __GETWRMN
	LDS  R@0,@2+(@3)
	LDS  R@1,@2+(@3)+1
	.ENDM

	.MACRO __GETWRZ
	LDD  R@0,Z+@2
	LDD  R@1,Z+@2+1
	.ENDM

	.MACRO __GETD2Z
	LDD  R26,Z+@0
	LDD  R27,Z+@0+1
	LDD  R24,Z+@0+2
	LDD  R25,Z+@0+3
	.ENDM

	.MACRO __GETB2MN
	LDS  R26,@0+(@1)
	.ENDM

	.MACRO __GETW2MN
	LDS  R26,@0+(@1)
	LDS  R27,@0+(@1)+1
	.ENDM

	.MACRO __GETD2MN
	LDS  R26,@0+(@1)
	LDS  R27,@0+(@1)+1
	LDS  R24,@0+(@1)+2
	LDS  R25,@0+(@1)+3
	.ENDM

	.MACRO __PUTB1MN
	STS  @0+(@1),R30
	.ENDM

	.MACRO __PUTW1MN
	STS  @0+(@1),R30
	STS  @0+(@1)+1,R31
	.ENDM

	.MACRO __PUTD1MN
	STS  @0+(@1),R30
	STS  @0+(@1)+1,R31
	STS  @0+(@1)+2,R22
	STS  @0+(@1)+3,R23
	.ENDM

	.MACRO __PUTB1EN
	LDI  R26,LOW(@0+(@1))
	LDI  R27,HIGH(@0+(@1))
	CALL __EEPROMWRB
	.ENDM

	.MACRO __PUTW1EN
	LDI  R26,LOW(@0+(@1))
	LDI  R27,HIGH(@0+(@1))
	CALL __EEPROMWRW
	.ENDM

	.MACRO __PUTD1EN
	LDI  R26,LOW(@0+(@1))
	LDI  R27,HIGH(@0+(@1))
	CALL __EEPROMWRD
	.ENDM

	.MACRO __PUTBR0MN
	STS  @0+(@1),R0
	.ENDM

	.MACRO __PUTBMRN
	STS  @0+(@1),R@2
	.ENDM

	.MACRO __PUTWMRN
	STS  @0+(@1),R@2
	STS  @0+(@1)+1,R@3
	.ENDM

	.MACRO __PUTBZR
	STD  Z+@1,R@0
	.ENDM

	.MACRO __PUTWZR
	STD  Z+@2,R@0
	STD  Z+@2+1,R@1
	.ENDM

	.MACRO __GETW1R
	MOV  R30,R@0
	MOV  R31,R@1
	.ENDM

	.MACRO __GETW2R
	MOV  R26,R@0
	MOV  R27,R@1
	.ENDM

	.MACRO __GETWRN
	LDI  R@0,LOW(@2)
	LDI  R@1,HIGH(@2)
	.ENDM

	.MACRO __PUTW1R
	MOV  R@0,R30
	MOV  R@1,R31
	.ENDM

	.MACRO __PUTW2R
	MOV  R@0,R26
	MOV  R@1,R27
	.ENDM

	.MACRO __ADDWRN
	SUBI R@0,LOW(-@2)
	SBCI R@1,HIGH(-@2)
	.ENDM

	.MACRO __ADDWRR
	ADD  R@0,R@2
	ADC  R@1,R@3
	.ENDM

	.MACRO __SUBWRN
	SUBI R@0,LOW(@2)
	SBCI R@1,HIGH(@2)
	.ENDM

	.MACRO __SUBWRR
	SUB  R@0,R@2
	SBC  R@1,R@3
	.ENDM

	.MACRO __ANDWRN
	ANDI R@0,LOW(@2)
	ANDI R@1,HIGH(@2)
	.ENDM

	.MACRO __ANDWRR
	AND  R@0,R@2
	AND  R@1,R@3
	.ENDM

	.MACRO __ORWRN
	ORI  R@0,LOW(@2)
	ORI  R@1,HIGH(@2)
	.ENDM

	.MACRO __ORWRR
	OR   R@0,R@2
	OR   R@1,R@3
	.ENDM

	.MACRO __EORWRR
	EOR  R@0,R@2
	EOR  R@1,R@3
	.ENDM

	.MACRO __GETWRS
	LDD  R@0,Y+@2
	LDD  R@1,Y+@2+1
	.ENDM

	.MACRO __PUTBSR
	STD  Y+@1,R@0
	.ENDM

	.MACRO __PUTWSR
	STD  Y+@2,R@0
	STD  Y+@2+1,R@1
	.ENDM

	.MACRO __MOVEWRR
	MOV  R@0,R@2
	MOV  R@1,R@3
	.ENDM

	.MACRO __INWR
	IN   R@0,@2
	IN   R@1,@2+1
	.ENDM

	.MACRO __OUTWR
	OUT  @2+1,R@1
	OUT  @2,R@0
	.ENDM

	.MACRO __CALL1MN
	LDS  R30,@0+(@1)
	LDS  R31,@0+(@1)+1
	ICALL
	.ENDM

	.MACRO __CALL1FN
	LDI  R30,LOW(2*@0+(@1))
	LDI  R31,HIGH(2*@0+(@1))
	CALL __GETW1PF
	ICALL
	.ENDM

	.MACRO __CALL2EN
	LDI  R26,LOW(@0+(@1))
	LDI  R27,HIGH(@0+(@1))
	CALL __EEPROMRDW
	ICALL
	.ENDM

	.MACRO __GETW1STACK
	IN   R26,SPL
	IN   R27,SPH
	ADIW R26,@0+1
	LD   R30,X+
	LD   R31,X
	.ENDM

	.MACRO __GETD1STACK
	IN   R26,SPL
	IN   R27,SPH
	ADIW R26,@0+1
	LD   R30,X+
	LD   R31,X+
	LD   R22,X
	.ENDM

	.MACRO __NBST
	BST  R@0,@1
	IN   R30,SREG
	LDI  R31,0x40
	EOR  R30,R31
	OUT  SREG,R30
	.ENDM


	.MACRO __PUTB1SN
	LDD  R26,Y+@0
	LDD  R27,Y+@0+1
	SUBI R26,LOW(-@1)
	SBCI R27,HIGH(-@1)
	ST   X,R30
	.ENDM

	.MACRO __PUTW1SN
	LDD  R26,Y+@0
	LDD  R27,Y+@0+1
	SUBI R26,LOW(-@1)
	SBCI R27,HIGH(-@1)
	ST   X+,R30
	ST   X,R31
	.ENDM

	.MACRO __PUTD1SN
	LDD  R26,Y+@0
	LDD  R27,Y+@0+1
	SUBI R26,LOW(-@1)
	SBCI R27,HIGH(-@1)
	CALL __PUTDP1
	.ENDM

	.MACRO __PUTB1SNS
	LDD  R26,Y+@0
	LDD  R27,Y+@0+1
	ADIW R26,@1
	ST   X,R30
	.ENDM

	.MACRO __PUTW1SNS
	LDD  R26,Y+@0
	LDD  R27,Y+@0+1
	ADIW R26,@1
	ST   X+,R30
	ST   X,R31
	.ENDM

	.MACRO __PUTD1SNS
	LDD  R26,Y+@0
	LDD  R27,Y+@0+1
	ADIW R26,@1
	CALL __PUTDP1
	.ENDM

	.MACRO __PUTB1PMN
	LDS  R26,@0
	LDS  R27,@0+1
	SUBI R26,LOW(-@1)
	SBCI R27,HIGH(-@1)
	ST   X,R30
	.ENDM

	.MACRO __PUTW1PMN
	LDS  R26,@0
	LDS  R27,@0+1
	SUBI R26,LOW(-@1)
	SBCI R27,HIGH(-@1)
	ST   X+,R30
	ST   X,R31
	.ENDM

	.MACRO __PUTD1PMN
	LDS  R26,@0
	LDS  R27,@0+1
	SUBI R26,LOW(-@1)
	SBCI R27,HIGH(-@1)
	CALL __PUTDP1
	.ENDM

	.MACRO __PUTB1PMNS
	LDS  R26,@0
	LDS  R27,@0+1
	ADIW R26,@1
	ST   X,R30
	.ENDM

	.MACRO __PUTW1PMNS
	LDS  R26,@0
	LDS  R27,@0+1
	ADIW R26,@1
	ST   X+,R30
	ST   X,R31
	.ENDM

	.MACRO __PUTD1PMNS
	LDS  R26,@0
	LDS  R27,@0+1
	ADIW R26,@1
	CALL __PUTDP1
	.ENDM

	.MACRO __PUTB1RN
	MOVW R26,R@0
	SUBI R26,LOW(-@1)
	SBCI R27,HIGH(-@1)
	ST   X,R30
	.ENDM

	.MACRO __PUTW1RN
	MOVW R26,R@0
	SUBI R26,LOW(-@1)
	SBCI R27,HIGH(-@1)
	ST   X+,R30
	ST   X,R31
	.ENDM

	.MACRO __PUTD1RN
	MOVW R26,R@0
	SUBI R26,LOW(-@1)
	SBCI R27,HIGH(-@1)
	CALL __PUTDP1
	.ENDM

	.MACRO __PUTB1RNS
	MOVW R26,R@0
	ADIW R26,@1
	ST   X,R30
	.ENDM

	.MACRO __PUTW1RNS
	MOVW R26,R@0
	ADIW R26,@1
	ST   X+,R30
	ST   X,R31
	.ENDM

	.MACRO __PUTD1RNS
	MOVW R26,R@0
	ADIW R26,@1
	CALL __PUTDP1
	.ENDM

	.MACRO __PUTB1RON
	MOV  R26,R@0
	MOV  R27,R@1
	SUBI R26,LOW(-@2)
	SBCI R27,HIGH(-@2)
	ST   X,R30
	.ENDM

	.MACRO __PUTW1RON
	MOV  R26,R@0
	MOV  R27,R@1
	SUBI R26,LOW(-@2)
	SBCI R27,HIGH(-@2)
	ST   X+,R30
	ST   X,R31
	.ENDM

	.MACRO __PUTD1RON
	MOV  R26,R@0
	MOV  R27,R@1
	SUBI R26,LOW(-@2)
	SBCI R27,HIGH(-@2)
	CALL __PUTDP1
	.ENDM

	.MACRO __PUTB1RONS
	MOV  R26,R@0
	MOV  R27,R@1
	ADIW R26,@2
	ST   X,R30
	.ENDM

	.MACRO __PUTW1RONS
	MOV  R26,R@0
	MOV  R27,R@1
	ADIW R26,@2
	ST   X+,R30
	ST   X,R31
	.ENDM

	.MACRO __PUTD1RONS
	MOV  R26,R@0
	MOV  R27,R@1
	ADIW R26,@2
	CALL __PUTDP1
	.ENDM


	.MACRO __GETB1SX
	MOVW R30,R28
	SUBI R30,LOW(-@0)
	SBCI R31,HIGH(-@0)
	LD   R30,Z
	.ENDM

	.MACRO __GETB1HSX
	MOVW R30,R28
	SUBI R30,LOW(-@0)
	SBCI R31,HIGH(-@0)
	LD   R31,Z
	.ENDM

	.MACRO __GETW1SX
	MOVW R30,R28
	SUBI R30,LOW(-@0)
	SBCI R31,HIGH(-@0)
	LD   R0,Z+
	LD   R31,Z
	MOV  R30,R0
	.ENDM

	.MACRO __GETD1SX
	MOVW R30,R28
	SUBI R30,LOW(-@0)
	SBCI R31,HIGH(-@0)
	LD   R0,Z+
	LD   R1,Z+
	LD   R22,Z+
	LD   R23,Z
	MOVW R30,R0
	.ENDM

	.MACRO __GETB2SX
	MOVW R26,R28
	SUBI R26,LOW(-@0)
	SBCI R27,HIGH(-@0)
	LD   R26,X
	.ENDM

	.MACRO __GETW2SX
	MOVW R26,R28
	SUBI R26,LOW(-@0)
	SBCI R27,HIGH(-@0)
	LD   R0,X+
	LD   R27,X
	MOV  R26,R0
	.ENDM

	.MACRO __GETD2SX
	MOVW R26,R28
	SUBI R26,LOW(-@0)
	SBCI R27,HIGH(-@0)
	LD   R0,X+
	LD   R1,X+
	LD   R24,X+
	LD   R25,X
	MOVW R26,R0
	.ENDM

	.MACRO __GETBRSX
	MOVW R30,R28
	SUBI R30,LOW(-@1)
	SBCI R31,HIGH(-@1)
	LD   R@0,Z
	.ENDM

	.MACRO __GETWRSX
	MOVW R30,R28
	SUBI R30,LOW(-@2)
	SBCI R31,HIGH(-@2)
	LD   R@0,Z+
	LD   R@1,Z
	.ENDM

	.MACRO __GETBRSX2
	MOVW R26,R28
	SUBI R26,LOW(-@1)
	SBCI R27,HIGH(-@1)
	LD   R@0,X
	.ENDM

	.MACRO __GETWRSX2
	MOVW R26,R28
	SUBI R26,LOW(-@2)
	SBCI R27,HIGH(-@2)
	LD   R@0,X+
	LD   R@1,X
	.ENDM

	.MACRO __LSLW8SX
	MOVW R30,R28
	SUBI R30,LOW(-@0)
	SBCI R31,HIGH(-@0)
	LD   R31,Z
	CLR  R30
	.ENDM

	.MACRO __PUTB1SX
	MOVW R26,R28
	SUBI R26,LOW(-@0)
	SBCI R27,HIGH(-@0)
	ST   X,R30
	.ENDM

	.MACRO __PUTW1SX
	MOVW R26,R28
	SUBI R26,LOW(-@0)
	SBCI R27,HIGH(-@0)
	ST   X+,R30
	ST   X,R31
	.ENDM

	.MACRO __PUTD1SX
	MOVW R26,R28
	SUBI R26,LOW(-@0)
	SBCI R27,HIGH(-@0)
	ST   X+,R30
	ST   X+,R31
	ST   X+,R22
	ST   X,R23
	.ENDM

	.MACRO __CLRW1SX
	MOVW R26,R28
	SUBI R26,LOW(-@0)
	SBCI R27,HIGH(-@0)
	ST   X+,R30
	ST   X,R30
	.ENDM

	.MACRO __CLRD1SX
	MOVW R26,R28
	SUBI R26,LOW(-@0)
	SBCI R27,HIGH(-@0)
	ST   X+,R30
	ST   X+,R30
	ST   X+,R30
	ST   X,R30
	.ENDM

	.MACRO __PUTB2SX
	MOVW R30,R28
	SUBI R30,LOW(-@0)
	SBCI R31,HIGH(-@0)
	ST   Z,R26
	.ENDM

	.MACRO __PUTW2SX
	MOVW R30,R28
	SUBI R30,LOW(-@0)
	SBCI R31,HIGH(-@0)
	ST   Z+,R26
	ST   Z,R27
	.ENDM

	.MACRO __PUTD2SX
	MOVW R30,R28
	SUBI R30,LOW(-@0)
	SBCI R31,HIGH(-@0)
	ST   Z+,R26
	ST   Z+,R27
	ST   Z+,R24
	ST   Z,R25
	.ENDM

	.MACRO __PUTBSRX
	MOVW R30,R28
	SUBI R30,LOW(-@1)
	SBCI R31,HIGH(-@1)
	ST   Z,R@0
	.ENDM

	.MACRO __PUTWSRX
	MOVW R30,R28
	SUBI R30,LOW(-@2)
	SBCI R31,HIGH(-@2)
	ST   Z+,R@0
	ST   Z,R@1
	.ENDM

	.MACRO __PUTB1SNX
	MOVW R26,R28
	SUBI R26,LOW(-@0)
	SBCI R27,HIGH(-@0)
	LD   R0,X+
	LD   R27,X
	MOV  R26,R0
	SUBI R26,LOW(-@1)
	SBCI R27,HIGH(-@1)
	ST   X,R30
	.ENDM

	.MACRO __PUTW1SNX
	MOVW R26,R28
	SUBI R26,LOW(-@0)
	SBCI R27,HIGH(-@0)
	LD   R0,X+
	LD   R27,X
	MOV  R26,R0
	SUBI R26,LOW(-@1)
	SBCI R27,HIGH(-@1)
	ST   X+,R30
	ST   X,R31
	.ENDM

	.MACRO __PUTD1SNX
	MOVW R26,R28
	SUBI R26,LOW(-@0)
	SBCI R27,HIGH(-@0)
	LD   R0,X+
	LD   R27,X
	MOV  R26,R0
	SUBI R26,LOW(-@1)
	SBCI R27,HIGH(-@1)
	ST   X+,R30
	ST   X+,R31
	ST   X+,R22
	ST   X,R23
	.ENDM

	.MACRO __MULBRR
	MULS R@0,R@1
	MOVW R30,R0
	.ENDM

	.MACRO __MULBRRU
	MUL  R@0,R@1
	MOVW R30,R0
	.ENDM

	.MACRO __MULBRR0
	MULS R@0,R@1
	.ENDM

	.MACRO __MULBRRU0
	MUL  R@0,R@1
	.ENDM

	.MACRO __MULBNWRU
	LDI  R26,@2
	MUL  R26,R@0
	MOVW R30,R0
	MUL  R26,R@1
	ADD  R31,R0
	.ENDM

;NAME DEFINITIONS FOR GLOBAL VARIABLES ALLOCATED TO REGISTERS
	.DEF _Temp_sort=R4
	.DEF _index_bri=R6
	.DEF _i=R8
	.DEF _j=R10
	.DEF _Mode_flag=R12

	.CSEG
	.ORG 0x00

;START OF CODE MARKER
__START_OF_CODE:

;INTERRUPT VECTORS
	JMP  __RESET
	JMP  _ext_int0_isr
	JMP  0x00
	JMP  0x00
	JMP  0x00
	JMP  _ext_int4_isr
	JMP  0x00
	JMP  0x00
	JMP  0x00
	JMP  0x00
	JMP  0x00
	JMP  0x00
	JMP  0x00
	JMP  0x00
	JMP  0x00
	JMP  _timer0_out__comp
	JMP  0x00
	JMP  0x00
	JMP  0x00
	JMP  0x00
	JMP  0x00
	JMP  0x00
	JMP  0x00
	JMP  0x00
	JMP  0x00
	JMP  0x00
	JMP  0x00
	JMP  0x00
	JMP  0x00
	JMP  0x00
	JMP  _usart1_receive
	JMP  0x00
	JMP  0x00
	JMP  0x00
	JMP  0x00

_tbl10_G100:
	.DB  0x10,0x27,0xE8,0x3,0x64,0x0,0xA,0x0
	.DB  0x1,0x0
_tbl16_G100:
	.DB  0x0,0x10,0x0,0x1,0x10,0x0,0x1,0x0

_0x35:
	.DB  0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20
	.DB  0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20
_0x36:
	.DB  0xC8
_0x83:
	.DB  0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0
	.DB  0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0
	.DB  0x0,0x0,0x0,0x0,0x0,0x0
_0x114:
	.DB  0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0
	.DB  0x0,0x0
_0x0:
	.DB  0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20
	.DB  0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20
	.DB  0x0,0x6D,0x6F,0x64,0x65,0x20,0x3A,0x20
	.DB  0x41,0x75,0x74,0x6F,0x0,0x6D,0x6F,0x64
	.DB  0x65,0x20,0x3A,0x20,0x4D,0x61,0x6E,0x75
	.DB  0x61,0x6C,0x0,0x6D,0x6F,0x64,0x65,0x20
	.DB  0x3A,0x20,0x54,0x69,0x6D,0x65,0x72,0x0
	.DB  0x4D,0x6F,0x64,0x65,0x20,0x53,0x65,0x6C
	.DB  0x65,0x63,0x74,0x0,0x43,0x75,0x72,0x74
	.DB  0x61,0x69,0x6E,0x20,0x44,0x6F,0x77,0x6E
	.DB  0x0,0x4E,0x6F,0x20,0x6D,0x6F,0x76,0x65
	.DB  0x0,0x43,0x75,0x72,0x74,0x61,0x69,0x6E
	.DB  0x20,0x75,0x70,0x20,0x0,0x55,0x70,0x5F
	.DB  0x4D,0x61,0x78,0x0,0x44,0x6F,0x77,0x6E
	.DB  0x5F,0x4D,0x61,0x78,0x0,0x43,0x75,0x72
	.DB  0x74,0x61,0x69,0x6E,0x20,0x4E,0x6F,0x4D
	.DB  0x6F,0x76,0x0,0xD,0x65,0x6E,0x74,0x65
	.DB  0x72,0x20,0x68,0x6F,0x75,0x72,0x20,0x3A
	.DB  0x20,0x0,0xA,0xD,0x20,0x54,0x69,0x6D
	.DB  0x65,0x20,0x53,0x65,0x74,0x74,0x69,0x6E
	.DB  0x67,0x20,0x4F,0x6B,0x21,0xA,0xD,0x0
	.DB  0x25,0x32,0x64,0x68,0x20,0x25,0x32,0x64
	.DB  0x6D,0x20,0x25,0x32,0x64,0x73,0x0,0x73
	.DB  0x65,0x6C,0x65,0x63,0x74,0x20,0x75,0x70
	.DB  0x64,0x6F,0x77,0x6E,0x0,0x55,0x70,0x20
	.DB  0x72,0x65,0x73,0x65,0x72,0x76,0x61,0x74
	.DB  0x69,0x6F,0x6E,0x0,0x44,0x6F,0x77,0x6E
	.DB  0x20,0x72,0x65,0x73,0x65,0x72,0x76,0x61
	.DB  0x74,0x69,0x6F,0x6E,0x0,0x53,0x74,0x61
	.DB  0x72,0x74,0x20,0x4D,0x6F,0x64,0x65,0x20
	.DB  0x0,0x41,0x75,0x74,0x6F,0x20,0x4D,0x6F
	.DB  0x64,0x65,0x20,0x20,0x0,0x4D,0x61,0x6E
	.DB  0x75,0x61,0x6C,0x20,0x4D,0x6F,0x64,0x65
	.DB  0x0,0x54,0x69,0x6D,0x65,0x72,0x20,0x4D
	.DB  0x6F,0x64,0x65,0x20,0x0,0xD,0xA,0x65
	.DB  0x72,0x72,0x6F,0x72,0x20,0x72,0x65,0x74
	.DB  0x72,0x79,0xA,0xD,0x0

__GLOBAL_INI_TBL:
	.DW  0x11
	.DW  _0xD
	.DW  _0x0*2

	.DW  0x0C
	.DW  _0xD+17
	.DW  _0x0*2+17

	.DW  0x11
	.DW  _0xD+29
	.DW  _0x0*2

	.DW  0x0E
	.DW  _0xD+46
	.DW  _0x0*2+29

	.DW  0x11
	.DW  _0xD+60
	.DW  _0x0*2

	.DW  0x0D
	.DW  _0xD+77
	.DW  _0x0*2+43

	.DW  0x11
	.DW  _0xD+90
	.DW  _0x0*2

	.DW  0x0C
	.DW  _0xD+107
	.DW  _0x0*2+56

	.DW  0x10
	.DW  _Erase
	.DW  _0x35*2

	.DW  0x01
	.DW  _Treshold
	.DW  _0x36*2

	.DW  0x0D
	.DW  _0x5F
	.DW  _0x0*2+68

	.DW  0x08
	.DW  _0x5F+13
	.DW  _0x0*2+81

	.DW  0x0C
	.DW  _0x5F+21
	.DW  _0x0*2+89

	.DW  0x08
	.DW  _0x5F+33
	.DW  _0x0*2+81

	.DW  0x0C
	.DW  _0x70
	.DW  _0x0*2+89

	.DW  0x07
	.DW  _0x70+12
	.DW  _0x0*2+101

	.DW  0x0D
	.DW  _0x70+19
	.DW  _0x0*2+68

	.DW  0x09
	.DW  _0x70+32
	.DW  _0x0*2+108

	.DW  0x0E
	.DW  _0x70+41
	.DW  _0x0*2+117

	.DW  0x0F
	.DW  _0x84
	.DW  _0x0*2+131

	.DW  0x16
	.DW  _0x84+15
	.DW  _0x0*2+146

	.DW  0x0E
	.DW  _0x84+37
	.DW  _0x0*2+183

	.DW  0x0F
	.DW  _0x84+51
	.DW  _0x0*2+197

	.DW  0x11
	.DW  _0x84+66
	.DW  _0x0*2+212

	.DW  0x0C
	.DW  _0x84+83
	.DW  _0x0*2+89

	.DW  0x07
	.DW  _0x84+95
	.DW  _0x0*2+101

	.DW  0x0D
	.DW  _0x84+102
	.DW  _0x0*2+68

	.DW  0x09
	.DW  _0x84+115
	.DW  _0x0*2+108

	.DW  0x0C
	.DW  _0xE1
	.DW  _0x0*2+229

	.DW  0x0C
	.DW  _0xE1+12
	.DW  _0x0*2+241

	.DW  0x0C
	.DW  _0xE1+24
	.DW  _0x0*2+253

	.DW  0x0C
	.DW  _0xE1+36
	.DW  _0x0*2+265

	.DW  0x10
	.DW  _0xF5
	.DW  _0x0*2+277

	.DW  0x0A
	.DW  0x04
	.DW  _0x114*2

_0xFFFFFFFF:
	.DW  0

__RESET:
	CLI
	CLR  R30
	OUT  EECR,R30

;INTERRUPT VECTORS ARE PLACED
;AT THE START OF FLASH
	LDI  R31,1
	OUT  MCUCR,R31
	OUT  MCUCR,R30
	STS  XMCRB,R30

;DISABLE WATCHDOG
	LDI  R31,0x18
	OUT  WDTCR,R31
	OUT  WDTCR,R30

;CLEAR R2-R14
	LDI  R24,(14-2)+1
	LDI  R26,2
	CLR  R27
__CLEAR_REG:
	ST   X+,R30
	DEC  R24
	BRNE __CLEAR_REG

;CLEAR SRAM
	LDI  R24,LOW(__CLEAR_SRAM_SIZE)
	LDI  R25,HIGH(__CLEAR_SRAM_SIZE)
	LDI  R26,LOW(__SRAM_START)
	LDI  R27,HIGH(__SRAM_START)
__CLEAR_SRAM:
	ST   X+,R30
	SBIW R24,1
	BRNE __CLEAR_SRAM

;GLOBAL VARIABLES INITIALIZATION
	LDI  R30,LOW(__GLOBAL_INI_TBL*2)
	LDI  R31,HIGH(__GLOBAL_INI_TBL*2)
__GLOBAL_INI_NEXT:
	LPM  R24,Z+
	LPM  R25,Z+
	SBIW R24,0
	BREQ __GLOBAL_INI_END
	LPM  R26,Z+
	LPM  R27,Z+
	LPM  R0,Z+
	LPM  R1,Z+
	MOVW R22,R30
	MOVW R30,R0
__GLOBAL_INI_LOOP:
	LPM  R0,Z+
	ST   X+,R0
	SBIW R24,1
	BRNE __GLOBAL_INI_LOOP
	MOVW R30,R22
	RJMP __GLOBAL_INI_NEXT
__GLOBAL_INI_END:

	OUT  RAMPZ,R24

;HARDWARE STACK POINTER INITIALIZATION
	LDI  R30,LOW(__SRAM_END-__HEAP_SIZE)
	OUT  SPL,R30
	LDI  R30,HIGH(__SRAM_END-__HEAP_SIZE)
	OUT  SPH,R30

;DATA STACK POINTER INITIALIZATION
	LDI  R28,LOW(__SRAM_START+__DSTACK_SIZE)
	LDI  R29,HIGH(__SRAM_START+__DSTACK_SIZE)

	JMP  _main

	.ESEG
	.ORG 0

	.DSEG
	.ORG 0x500

	.CSEG
;#include <mega128.h>
	#ifndef __SLEEP_DEFINED__
	#define __SLEEP_DEFINED__
	.EQU __se_bit=0x20
	.EQU __sm_mask=0x1C
	.EQU __sm_powerdown=0x10
	.EQU __sm_powersave=0x18
	.EQU __sm_standby=0x14
	.EQU __sm_ext_standby=0x1C
	.EQU __sm_adc_noise_red=0x08
	.SET power_ctrl_reg=mcucr
	#endif
;#include "lcd.h"

	.CSEG
_LCD_Data:
;	ch -> Y+0
	LDS  R30,101
	ORI  R30,4
	CALL SUBOPT_0x0
	ANDI R30,0xFD
	CALL SUBOPT_0x0
	CALL SUBOPT_0x1
	RJMP _0x2060004
_LCD_Comm:
;	command -> Y+0
	LDS  R30,101
	ANDI R30,0xFB
	CALL SUBOPT_0x0
	ANDI R30,0xFD
	CALL SUBOPT_0x0
	CALL SUBOPT_0x1
	RJMP _0x2060004
_LCD_Delay:
;	ms -> Y+0
	LD   R30,Y
	LDI  R31,0
	CALL SUBOPT_0x2
	RJMP _0x2060004
_LCD_Char:
;	ch -> Y+0
	LDI  R30,LOW(1)
	ST   -Y,R30
	RCALL _LCD_Delay
	LD   R30,Y
	ST   -Y,R30
	RCALL _LCD_Data
	RJMP _0x2060004
_LCD_Str:
;	*str -> Y+0
_0x3:
	LD   R26,Y
	LDD  R27,Y+1
	LD   R30,X
	CPI  R30,0
	BREQ _0x5
	ST   -Y,R30
	RCALL _LCD_Char
	LD   R30,Y
	LDD  R31,Y+1
	ADIW R30,1
	ST   Y,R30
	STD  Y+1,R31
	RJMP _0x3
_0x5:
	RJMP _0x2060003
_LCD_Pos:
;	x -> Y+1
;	y -> Y+0
	LDD  R30,Y+1
	LDI  R26,LOW(64)
	MULS R30,R26
	MOVW R30,R0
	LD   R26,Y
	ADD  R30,R26
	ORI  R30,0x80
	ST   -Y,R30
	RCALL _LCD_Comm
	RJMP _0x2060003
_LCD_Clear:
	LDI  R30,LOW(1)
	CALL SUBOPT_0x3
	RET
_LCD_PORT_Init:
	LDI  R30,LOW(255)
	OUT  0x1A,R30
	LDS  R30,100
	ORI  R30,LOW(0xF)
	STS  100,R30
	RET
_LCD_Init:
	RCALL _LCD_PORT_Init
	CALL SUBOPT_0x4
	CALL SUBOPT_0x4
	CALL SUBOPT_0x4
	LDI  R30,LOW(12)
	CALL SUBOPT_0x3
	LDI  R30,LOW(6)
	CALL SUBOPT_0x3
	RCALL _LCD_Clear
	RET
;	p -> Y+0
;	p -> Y+0
;#include "Term.h"
_LCD_High_line:
;	Auto_flag -> Y+0
	LD   R26,Y
	LDD  R27,Y+1
	SBIW R26,1
	BRNE _0xC
	CALL SUBOPT_0x5
	__POINTW1MN _0xD,0
	CALL SUBOPT_0x6
	__POINTW1MN _0xD,17
	RJMP _0x10D
_0xC:
	LD   R26,Y
	LDD  R27,Y+1
	SBIW R26,2
	BRNE _0xF
	CALL SUBOPT_0x5
	__POINTW1MN _0xD,29
	CALL SUBOPT_0x6
	__POINTW1MN _0xD,46
	RJMP _0x10D
_0xF:
	LD   R26,Y
	LDD  R27,Y+1
	SBIW R26,3
	BRNE _0x11
	CALL SUBOPT_0x5
	__POINTW1MN _0xD,60
	CALL SUBOPT_0x6
	__POINTW1MN _0xD,77
	RJMP _0x10D
_0x11:
	LD   R30,Y
	LDD  R31,Y+1
	SBIW R30,0
	BRNE _0x13
	CALL SUBOPT_0x5
	__POINTW1MN _0xD,90
	CALL SUBOPT_0x6
	__POINTW1MN _0xD,107
_0x10D:
	ST   -Y,R31
	ST   -Y,R30
	RCALL _LCD_Str
_0x13:
	RJMP _0x2060003

	.DSEG
_0xD:
	.BYTE 0x77

	.CSEG
_motor_up:
	SBI  0x18,0
	CBI  0x18,1
	RJMP _0x2060006
_motor_stop:
	CBI  0x18,0
	CBI  0x18,1
	CBI  0x18,2
	RET
_motor_down:
	CBI  0x18,0
	SBI  0x18,1
_0x2060006:
	SBI  0x18,2
	RET
_PinE_4_init:
	CBI  0x2,4
	SBI  0x3,4
	IN   R30,0x20
	ANDI R30,0xFB
	OUT  0x20,R30
	IN   R30,0x3A
	OUT  0x3A,R30
	IN   R30,0x39
	ORI  R30,0x10
	RJMP _0x2060005
_PinD_0_init:
	CBI  0x11,4
	LDI  R26,LOW(106)
	LDI  R27,HIGH(106)
	LD   R30,X
	ST   X,R30
	IN   R30,0x39
	ORI  R30,1
_0x2060005:
	OUT  0x39,R30
	RET
_myDelay_us:
	CALL SUBOPT_0x7
;	delay -> Y+2
;	i -> R16,R17
_0x2D:
	LDD  R30,Y+2
	MOVW R26,R16
	LDI  R31,0
	CP   R26,R30
	CPC  R27,R31
	BRGE _0x2E
	__DELAY_USB 5
	__ADDWRN 16,17,1
	RJMP _0x2D
_0x2E:
	LDD  R17,Y+1
	LDD  R16,Y+0
	ADIW R28,3
	RET
_SSound:
;	time -> Y+0
	LDS  R30,101
	ORI  R30,0x10
	CALL SUBOPT_0x8
	LDS  R30,101
	ANDI R30,0xEF
	CALL SUBOPT_0x8
	RJMP _0x2060003
_Timer2_Init:
	LDI  R30,LOW(0)
	OUT  0x25,R30
	IN   R30,0x25
	ORI  R30,LOW(0x68)
	OUT  0x25,R30
	LDI  R30,LOW(128)
	OUT  0x23,R30
	RET
_Timer0_Init:
	LDI  R30,LOW(15)
	OUT  0x33,R30
	LDI  R30,LOW(0)
	OUT  0x32,R30
	LDI  R30,LOW(14)
	OUT  0x31,R30
	IN   R30,0x37
	ORI  R30,2
	OUT  0x37,R30
	RET
_Init_USART1:
	LDS  R30,154
	ORI  R30,LOW(0x98)
	STS  154,R30
	LDI  R30,LOW(0)
	STS  155,R30
	STS  152,R30
	LDI  R30,LOW(7)
	STS  153,R30
	BSET 7
	RET
_putch_USART1:
;	data -> Y+0
_0x2F:
	LDS  R30,155
	ANDI R30,LOW(0x20)
	BREQ _0x2F
	LD   R30,Y
	STS  156,R30
_0x2060004:
	ADIW R28,1
	RET
_puts_USART1:
;	*str -> Y+0
_0x32:
	LD   R26,Y
	LDD  R27,Y+1
	LD   R30,X
	CPI  R30,0
	BREQ _0x34
	ST   -Y,R30
	RCALL _putch_USART1
	LD   R30,Y
	LDD  R31,Y+1
	ADIW R30,1
	ST   Y,R30
	STD  Y+1,R31
	RJMP _0x32
_0x34:
	RJMP _0x2060003
;#include <stdio.h>
;#define Buzz  195
;#define warning 300
;#define Up_max 1
;#define Down_max 2
;#define Switch 3
;#define Bright_check 4
;#define Mode_Sel 5
;unsigned int average[10] = {0,};   //10칸을 0으로 초기화
;unsigned int Temp_sort = 0;
;int index_bri = 0;
;int i,j;
;unsigned char Erase[] = "                ";

	.DSEG
;int Mode_flag = 0;
;int bright_count = 0;
;short Bright_val;
;short pre_Bright;
;int Count = 0;
;int B_on = 0;
;int Mo_count = 0;
;int Mo_exit = 0;
;char Time[] = {0,0,0,0,0,0};
;int Mo_time_count = 0;
;int reservation_mode = 0;
;int wrong_flag = 0;
;int index = 0;
;int Treshold = 200;
;int Treshold_count = 0;
;int no_interrupt_flag = 0;
;
;short Get_ADC(int Number)
; 0000 0023 {

	.CSEG
_Get_ADC:
; 0000 0024     ADMUX = Number;
;	Number -> Y+0
	LD   R30,Y
	OUT  0x7,R30
; 0000 0025     ADMUX |= (1<<REFS1);
	SBI  0x7,7
; 0000 0026     ADCSRA |= (1<<ADEN)|(1<<ADSC)|(1<<ADFR)|(1<<ADPS2)|(1<<ADPS1);
	IN   R30,0x6
	ORI  R30,LOW(0xE6)
	OUT  0x6,R30
; 0000 0027     delay_ms(10);
	LDI  R30,LOW(10)
	LDI  R31,HIGH(10)
	CALL SUBOPT_0x2
; 0000 0028     while(!(ADCSRA&(1<<ADIF)));
_0x37:
	SBIS 0x6,4
	RJMP _0x37
; 0000 0029     return ADCW;
	IN   R30,0x4
	IN   R31,0x4+1
_0x2060003:
	ADIW R28,2
	RET
; 0000 002A }
;
;interrupt [TIM0_COMP] void timer0_out__comp(void)   //타이머 CTC 비교일치시 수행되는 함수 정의
; 0000 002D {
_timer0_out__comp:
	CALL SUBOPT_0x9
; 0000 002E     bright_count++;
	LDI  R26,LOW(_bright_count)
	LDI  R27,HIGH(_bright_count)
	CALL SUBOPT_0xA
; 0000 002F     Count++;
	LDI  R26,LOW(_Count)
	LDI  R27,HIGH(_Count)
	CALL SUBOPT_0xA
; 0000 0030     if(bright_count > 499)
	LDS  R26,_bright_count
	LDS  R27,_bright_count+1
	CPI  R26,LOW(0x1F4)
	LDI  R30,HIGH(0x1F4)
	CPC  R27,R30
	BRGE PC+3
	JMP _0x3A
; 0000 0031     {
; 0000 0032         if(B_on == 1)
	LDS  R26,_B_on
	LDS  R27,_B_on+1
	SBIW R26,1
	BREQ PC+3
	JMP _0x3B
; 0000 0033         {
; 0000 0034             average[index_bri]  = Get_ADC(Bright_check); //조도는 시도때도없이 확인할 필요가 없기때문에 일정시간동안만 하는것을 구현
	MOVW R30,R6
	CALL SUBOPT_0xB
	ADD  R30,R26
	ADC  R31,R27
	PUSH R31
	PUSH R30
	CALL SUBOPT_0xC
	POP  R26
	POP  R27
	ST   X+,R30
	ST   X,R31
; 0000 0035             index_bri++;
	MOVW R30,R6
	ADIW R30,1
	MOVW R6,R30
; 0000 0036             if(index_bri >= 9)
	LDI  R30,LOW(9)
	LDI  R31,HIGH(9)
	CP   R6,R30
	CPC  R7,R31
	BRGE PC+3
	JMP _0x3C
; 0000 0037             {
; 0000 0038                 index_bri = 0;
	CLR  R6
	CLR  R7
; 0000 0039                 for(i = 0; i < 10 ; i++)//10개의 입력받은 데이터 값을 내림차순으로
	CLR  R8
	CLR  R9
_0x3E:
	LDI  R30,LOW(10)
	LDI  R31,HIGH(10)
	CP   R8,R30
	CPC  R9,R31
	BRLT PC+3
	JMP _0x3F
; 0000 003A                 {                       //정렬하는 알고리즘 입니다.
; 0000 003B                     for(j = i+1; j < 10 ; j++){
	MOVW R30,R8
	ADIW R30,1
	MOVW R10,R30
_0x41:
	LDI  R30,LOW(10)
	LDI  R31,HIGH(10)
	CP   R10,R30
	CPC  R11,R31
	BRGE _0x42
; 0000 003C                         if(average[i]>average[j])
	MOVW R30,R8
	CALL SUBOPT_0xB
	ADD  R26,R30
	ADC  R27,R31
	LD   R0,X+
	LD   R1,X
	MOVW R30,R10
	CALL SUBOPT_0xB
	ADD  R26,R30
	ADC  R27,R31
	CALL __GETW1P
	CP   R30,R0
	CPC  R31,R1
	BRSH _0x43
; 0000 003D                         {
; 0000 003E                             Temp_sort = average[i];
	MOVW R30,R8
	CALL SUBOPT_0xB
	ADD  R26,R30
	ADC  R27,R31
	LD   R4,X+
	LD   R5,X
; 0000 003F                             average[i] = average[j];
	MOVW R30,R8
	CALL SUBOPT_0xB
	ADD  R30,R26
	ADC  R31,R27
	MOVW R0,R30
	MOVW R30,R10
	CALL SUBOPT_0xB
	ADD  R26,R30
	ADC  R27,R31
	CALL __GETW1P
	MOVW R26,R0
	ST   X+,R30
	ST   X,R31
; 0000 0040                             average[j] = Temp_sort;
	MOVW R30,R10
	CALL SUBOPT_0xB
	ADD  R30,R26
	ADC  R31,R27
	ST   Z,R4
	STD  Z+1,R5
; 0000 0041                             j = i+1;
	MOVW R30,R8
	ADIW R30,1
	MOVW R10,R30
; 0000 0042                         }
; 0000 0043                     }
_0x43:
	MOVW R30,R10
	ADIW R30,1
	MOVW R10,R30
	RJMP _0x41
_0x42:
; 0000 0044                 }
	MOVW R30,R8
	ADIW R30,1
	MOVW R8,R30
	RJMP _0x3E
_0x3F:
; 0000 0045                 pre_Bright = Bright_val;
	CALL SUBOPT_0xD
; 0000 0046                 Bright_val =  average[4];
	__GETW1MN _average,8
	STS  _Bright_val,R30
	STS  _Bright_val+1,R31
; 0000 0047                 if(Bright_val <= 200)
	CALL SUBOPT_0xE
	CPI  R26,LOW(0xC9)
	LDI  R30,HIGH(0xC9)
	CPC  R27,R30
	BRGE _0x44
; 0000 0048                 {
; 0000 0049                     PORTC = 0x00;
	LDI  R30,LOW(0)
	RJMP _0x10E
; 0000 004A                 }
; 0000 004B                 else if(Bright_val <= 400)
_0x44:
	CALL SUBOPT_0xE
	CPI  R26,LOW(0x191)
	LDI  R30,HIGH(0x191)
	CPC  R27,R30
	BRGE _0x46
; 0000 004C                 {
; 0000 004D                     PORTC = 0x08;
	LDI  R30,LOW(8)
	RJMP _0x10E
; 0000 004E                 }
; 0000 004F                 else if(Bright_val <= 600)
_0x46:
	CALL SUBOPT_0xE
	CPI  R26,LOW(0x259)
	LDI  R30,HIGH(0x259)
	CPC  R27,R30
	BRGE _0x48
; 0000 0050                 {
; 0000 0051                     PORTC = 0x0C;
	LDI  R30,LOW(12)
	RJMP _0x10E
; 0000 0052                 }
; 0000 0053                 else if(Bright_val <= 800)
_0x48:
	CALL SUBOPT_0xE
	CPI  R26,LOW(0x321)
	LDI  R30,HIGH(0x321)
	CPC  R27,R30
	BRGE _0x4A
; 0000 0054                 {
; 0000 0055                     PORTC = 0x0E;
	LDI  R30,LOW(14)
	RJMP _0x10E
; 0000 0056                 }
; 0000 0057                 else
_0x4A:
; 0000 0058                 {
; 0000 0059                     PORTC = 0x0f;
	LDI  R30,LOW(15)
_0x10E:
	OUT  0x15,R30
; 0000 005A                 }
; 0000 005B 
; 0000 005C             }
; 0000 005D         }
_0x3C:
; 0000 005E         bright_count = 0;
_0x3B:
	LDI  R30,LOW(0)
	STS  _bright_count,R30
	STS  _bright_count+1,R30
; 0000 005F     }
; 0000 0060 }
_0x3A:
	RJMP _0x113
;
;
;interrupt [EXT_INT4] void ext_int4_isr(void)
; 0000 0064 {
_ext_int4_isr:
	CALL SUBOPT_0xF
; 0000 0065     delay_ms(100);
; 0000 0066     if(no_interrupt_flag == 1)
	LDS  R26,_no_interrupt_flag
	LDS  R27,_no_interrupt_flag+1
	SBIW R26,1
	BREQ _0x4D
; 0000 0067     {
; 0000 0068 
; 0000 0069     }
; 0000 006A     else if(reservation_mode == 1)
	LDS  R26,_reservation_mode
	LDS  R27,_reservation_mode+1
	SBIW R26,1
	BRNE _0x4E
; 0000 006B     {
; 0000 006C         reservation_mode = 0;
	LDI  R30,LOW(0)
	STS  _reservation_mode,R30
	STS  _reservation_mode+1,R30
; 0000 006D         delay_ms(200);
	CALL SUBOPT_0x10
; 0000 006E     }
; 0000 006F     else if(Mode_flag !=0)
	RJMP _0x4F
_0x4E:
	MOV  R0,R12
	OR   R0,R13
	BREQ _0x50
; 0000 0070     {
; 0000 0071         while(1)
_0x51:
; 0000 0072         {
; 0000 0073             if(PINE.4 == 1)
	SBIS 0x1,4
	RJMP _0x54
; 0000 0074             {
; 0000 0075                 Mo_count = Mode_flag;
	__PUTWMRN _Mo_count,0,12,13
; 0000 0076                 Mode_flag = 0;
	CLR  R12
	CLR  R13
; 0000 0077                 delay_ms(100);
	CALL SUBOPT_0x11
; 0000 0078                 break;
	RJMP _0x53
; 0000 0079             }
; 0000 007A         }
_0x54:
	RJMP _0x51
_0x53:
; 0000 007B     }
; 0000 007C     else
	RJMP _0x55
_0x50:
; 0000 007D     {
; 0000 007E         Mo_exit = 1;
	LDI  R30,LOW(1)
	LDI  R31,HIGH(1)
	STS  _Mo_exit,R30
	STS  _Mo_exit+1,R31
; 0000 007F     }
_0x55:
_0x4F:
_0x4D:
; 0000 0080 }
	RJMP _0x113
;void Auto_mode(void)
; 0000 0082 {
_Auto_mode:
; 0000 0083     short gab,temp_val;
; 0000 0084     LCD_Pos(1,0);
	CALL __SAVELOCR4
;	gab -> R16,R17
;	temp_val -> R18,R19
	CALL SUBOPT_0x12
; 0000 0085     LCD_Str(Erase);
; 0000 0086     B_on = 1;
	CALL SUBOPT_0x13
; 0000 0087     Bright_val = Get_ADC(Bright_check);// 초기 한번 측정 후 반응
	CALL SUBOPT_0xC
	STS  _Bright_val,R30
	STS  _Bright_val+1,R31
; 0000 0088     pre_Bright = Bright_val;
	CALL SUBOPT_0xD
; 0000 0089     gab  = 50;
	__GETWRN 16,17,50
; 0000 008A     while(1)
_0x56:
; 0000 008B     { //Auto_mode
; 0000 008C         if(gab >= 50)
	__CPWRN 16,17,50
	BRGE PC+3
	JMP _0x59
; 0000 008D         {
; 0000 008E             if(Bright_val < Treshold){
	LDS  R30,_Treshold
	LDS  R31,_Treshold+1
	CALL SUBOPT_0xE
	CP   R26,R30
	CPC  R27,R31
	BRGE _0x5A
; 0000 008F                 while(1){
_0x5B:
; 0000 0090                     delay_ms(5);
	CALL SUBOPT_0x14
; 0000 0091                     B_on = 0;
	CALL SUBOPT_0x15
; 0000 0092                     temp_val = Get_ADC(Down_max);
	CALL SUBOPT_0x16
	MOVW R18,R30
; 0000 0093                     B_on = 1;
	CALL SUBOPT_0x13
; 0000 0094                     if(temp_val < 600){
	__CPWRN 18,19,600
	BRGE _0x5E
; 0000 0095                         delay_ms(5);
	CALL SUBOPT_0x14
; 0000 0096                         motor_down();
	CALL SUBOPT_0x17
; 0000 0097                         LCD_Pos(1,0);
; 0000 0098                         LCD_Str(Erase);
; 0000 0099                         LCD_Pos(1,0);
	CALL SUBOPT_0x18
; 0000 009A                         LCD_Str("Curtain Down");
	__POINTW1MN _0x5F,0
	CALL SUBOPT_0x19
; 0000 009B                         delay_ms(40);
	LDI  R30,LOW(40)
	LDI  R31,HIGH(40)
	CALL SUBOPT_0x2
; 0000 009C                         motor_stop();
	RCALL _motor_stop
; 0000 009D                     }
; 0000 009E                     else{
	RJMP _0x60
_0x5E:
; 0000 009F                         delay_ms(5);
	CALL SUBOPT_0x14
; 0000 00A0                         LCD_Pos(1,0);
	CALL SUBOPT_0x12
; 0000 00A1                         LCD_Str(Erase);
; 0000 00A2                         LCD_Pos(1,5);
	CALL SUBOPT_0x1A
; 0000 00A3                         LCD_Str("No move");
	__POINTW1MN _0x5F,13
	CALL SUBOPT_0x19
; 0000 00A4                         delay_ms(200);
	CALL SUBOPT_0x10
; 0000 00A5                         break;
	RJMP _0x5D
; 0000 00A6                     }
_0x60:
; 0000 00A7                 }
	RJMP _0x5B
_0x5D:
; 0000 00A8             }
; 0000 00A9             else{
	RJMP _0x61
_0x5A:
; 0000 00AA                 while(1){
_0x62:
; 0000 00AB                     delay_ms(5);
	CALL SUBOPT_0x14
; 0000 00AC                     B_on = 0;
	CALL SUBOPT_0x15
; 0000 00AD                     temp_val = Get_ADC(Up_max);
	CALL SUBOPT_0x1B
	MOVW R18,R30
; 0000 00AE                     B_on = 1;
	CALL SUBOPT_0x13
; 0000 00AF                     if(temp_val > 600){
	__CPWRN 18,19,601
	BRLT _0x65
; 0000 00B0                         delay_ms(5);
	CALL SUBOPT_0x14
; 0000 00B1                         motor_up();
	CALL SUBOPT_0x1C
; 0000 00B2                         LCD_Pos(1,0);
; 0000 00B3                         LCD_Str(Erase);
; 0000 00B4                         LCD_Pos(1,0);
	CALL SUBOPT_0x18
; 0000 00B5                         LCD_Str("Curtain up ");
	__POINTW1MN _0x5F,21
	CALL SUBOPT_0x19
; 0000 00B6                         delay_ms(40);
	LDI  R30,LOW(40)
	LDI  R31,HIGH(40)
	CALL SUBOPT_0x2
; 0000 00B7                         motor_stop();
	RCALL _motor_stop
; 0000 00B8                     }
; 0000 00B9                     else{
	RJMP _0x66
_0x65:
; 0000 00BA                         delay_ms(5);
	CALL SUBOPT_0x14
; 0000 00BB                         LCD_Pos(1,0);
	CALL SUBOPT_0x12
; 0000 00BC                         LCD_Str(Erase);
; 0000 00BD                         LCD_Pos(1,5);
	CALL SUBOPT_0x1A
; 0000 00BE                         LCD_Str("No move");
	__POINTW1MN _0x5F,33
	CALL SUBOPT_0x19
; 0000 00BF                         delay_ms(200);
	CALL SUBOPT_0x10
; 0000 00C0                         break;
	RJMP _0x64
; 0000 00C1                     }
_0x66:
; 0000 00C2                 }
	RJMP _0x62
_0x64:
; 0000 00C3             }
_0x61:
; 0000 00C4         }
; 0000 00C5         if(pre_Bright > Bright_val){
_0x59:
	LDS  R30,_Bright_val
	LDS  R31,_Bright_val+1
	LDS  R26,_pre_Bright
	LDS  R27,_pre_Bright+1
	CP   R30,R26
	CPC  R31,R27
	BRGE _0x67
; 0000 00C6             gab = pre_Bright - Bright_val;
	CALL SUBOPT_0xE
	LDS  R30,_pre_Bright
	LDS  R31,_pre_Bright+1
	RJMP _0x10F
; 0000 00C7         }
; 0000 00C8         else{
_0x67:
; 0000 00C9             gab = Bright_val - pre_Bright;
	LDS  R26,_pre_Bright
	LDS  R27,_pre_Bright+1
	LDS  R30,_Bright_val
	LDS  R31,_Bright_val+1
_0x10F:
	SUB  R30,R26
	SBC  R31,R27
	MOVW R16,R30
; 0000 00CA         }
; 0000 00CB         if(Mode_flag != 1){//Auto Mode 가 아닐시 함수탈출
	LDI  R30,LOW(1)
	LDI  R31,HIGH(1)
	CP   R30,R12
	CPC  R31,R13
	BREQ _0x69
; 0000 00CC             LCD_Clear();
	RCALL _LCD_Clear
; 0000 00CD             Treshold = 170;
	LDI  R30,LOW(170)
	LDI  R31,HIGH(170)
	STS  _Treshold,R30
	STS  _Treshold+1,R31
; 0000 00CE             B_on = 0;
	CALL SUBOPT_0x15
; 0000 00CF             break;
	RJMP _0x58
; 0000 00D0         }
; 0000 00D1      }
_0x69:
	RJMP _0x56
_0x58:
; 0000 00D2 }
	CALL __LOADLOCR4
	ADIW R28,4
	RET

	.DSEG
_0x5F:
	.BYTE 0x29
;
;void Manual_mode(void)
; 0000 00D5 {

	.CSEG
_Manual_mode:
; 0000 00D6      int Switch_val = 0;
; 0000 00D7      LCD_Pos(1,0);
	CALL SUBOPT_0x7
;	Switch_val -> R16,R17
	CALL SUBOPT_0x12
; 0000 00D8      LCD_Str(Erase);
; 0000 00D9      B_on = 0; //조도 꺼주는 옵션
	CALL SUBOPT_0x15
; 0000 00DA      while(1)
_0x6A:
; 0000 00DB      {
; 0000 00DC         Switch_val =  Get_ADC(Switch);
	LDI  R30,LOW(3)
	LDI  R31,HIGH(3)
	CALL SUBOPT_0x1D
	MOVW R16,R30
; 0000 00DD         //Manual_mode
; 0000 00DE         if(Mode_flag != 2)
	LDI  R30,LOW(2)
	LDI  R31,HIGH(2)
	CP   R30,R12
	CPC  R31,R13
	BREQ _0x6D
; 0000 00DF         {
; 0000 00E0             LCD_Clear();
	RCALL _LCD_Clear
; 0000 00E1             break;
	RJMP _0x6C
; 0000 00E2         }
; 0000 00E3         if(Switch_val <300)
_0x6D:
	__CPWRN 16,17,300
	BRGE _0x6E
; 0000 00E4         {
; 0000 00E5             if(Get_ADC(Up_max) > 600){
	CALL SUBOPT_0x1B
	CPI  R30,LOW(0x259)
	LDI  R26,HIGH(0x259)
	CPC  R31,R26
	BRLT _0x6F
; 0000 00E6                 motor_up();
	CALL SUBOPT_0x1C
; 0000 00E7                 LCD_Pos(1,0);
; 0000 00E8                 LCD_Str(Erase);
; 0000 00E9                 LCD_Pos(1,0);
	CALL SUBOPT_0x18
; 0000 00EA                 LCD_Str("Curtain up ");
	__POINTW1MN _0x70,0
	CALL SUBOPT_0x19
; 0000 00EB                 Count = 0;
	CALL SUBOPT_0x1E
; 0000 00EC                 while(Count<125)
_0x71:
	CALL SUBOPT_0x1F
	BRGE _0x73
; 0000 00ED                 {SSound(Buzz);}
	CALL SUBOPT_0x20
	RJMP _0x71
_0x73:
; 0000 00EE                 delay_ms(20);
	CALL SUBOPT_0x21
; 0000 00EF                 motor_stop();
	RCALL _motor_stop
; 0000 00F0             }
; 0000 00F1             else
	RJMP _0x74
_0x6F:
; 0000 00F2             {
; 0000 00F3                 LCD_Pos(1,0);
	CALL SUBOPT_0x12
; 0000 00F4                 LCD_Str(Erase);
; 0000 00F5                 LCD_Pos(1,5);
	CALL SUBOPT_0x1A
; 0000 00F6                 LCD_Str("Up_Max");
	__POINTW1MN _0x70,12
	CALL SUBOPT_0x19
; 0000 00F7                 Count = 0;
	CALL SUBOPT_0x1E
; 0000 00F8                 while(Count<125)
_0x75:
	CALL SUBOPT_0x1F
	BRGE _0x77
; 0000 00F9                 {SSound(warning);}
	CALL SUBOPT_0x22
	RJMP _0x75
_0x77:
; 0000 00FA                 delay_ms(200);
	CALL SUBOPT_0x10
; 0000 00FB             }
_0x74:
; 0000 00FC         }
; 0000 00FD         else if(Switch_val > 1000)
	RJMP _0x78
_0x6E:
	__CPWRN 16,17,1001
	BRLT _0x79
; 0000 00FE         {
; 0000 00FF             if(Get_ADC(Down_max) < 600){
	CALL SUBOPT_0x16
	CPI  R30,LOW(0x258)
	LDI  R26,HIGH(0x258)
	CPC  R31,R26
	BRGE _0x7A
; 0000 0100                 motor_down();
	CALL SUBOPT_0x17
; 0000 0101                 LCD_Pos(1,0);
; 0000 0102                 LCD_Str(Erase);
; 0000 0103                 LCD_Pos(1,0);
	CALL SUBOPT_0x18
; 0000 0104                 LCD_Str("Curtain Down");
	__POINTW1MN _0x70,19
	CALL SUBOPT_0x19
; 0000 0105                 Count = 0;
	CALL SUBOPT_0x1E
; 0000 0106                 while(Count<125)
_0x7B:
	CALL SUBOPT_0x1F
	BRGE _0x7D
; 0000 0107                 {SSound(Buzz);}
	CALL SUBOPT_0x20
	RJMP _0x7B
_0x7D:
; 0000 0108                 delay_ms(20);
	CALL SUBOPT_0x21
; 0000 0109                 motor_stop();
	RCALL _motor_stop
; 0000 010A             }
; 0000 010B             else
	RJMP _0x7E
_0x7A:
; 0000 010C             {
; 0000 010D                 LCD_Pos(1,0);
	CALL SUBOPT_0x12
; 0000 010E                 LCD_Str(Erase);
; 0000 010F                 LCD_Pos(1,5);
	CALL SUBOPT_0x1A
; 0000 0110                 LCD_Str("Down_Max");
	__POINTW1MN _0x70,32
	CALL SUBOPT_0x19
; 0000 0111                 Count = 0;
	CALL SUBOPT_0x1E
; 0000 0112                 while(Count<125)
_0x7F:
	CALL SUBOPT_0x1F
	BRGE _0x81
; 0000 0113                 {SSound(warning);}
	CALL SUBOPT_0x22
	RJMP _0x7F
_0x81:
; 0000 0114                 delay_ms(200);
	CALL SUBOPT_0x10
; 0000 0115             }
_0x7E:
; 0000 0116         }
; 0000 0117         else
	RJMP _0x82
_0x79:
; 0000 0118         {
; 0000 0119             LCD_Pos(1,0);
	CALL SUBOPT_0x18
; 0000 011A             LCD_Str("Curtain NoMov");
	__POINTW1MN _0x70,41
	CALL SUBOPT_0x19
; 0000 011B             delay_ms(100);
	CALL SUBOPT_0x11
; 0000 011C         }
_0x82:
_0x78:
; 0000 011D      }
	RJMP _0x6A
_0x6C:
; 0000 011E }
	RJMP _0x2060002

	.DSEG
_0x70:
	.BYTE 0x37
;
;void Timer_mode(void)
; 0000 0121 {

	.CSEG
_Timer_mode:
; 0000 0122     int hr,min,sec;
; 0000 0123     int Switch_val = 0;
; 0000 0124     char Message[20] = {0,};
; 0000 0125     wrong_flag=0;
	SBIW R28,22
	LDI  R24,22
	LDI  R26,LOW(0)
	LDI  R27,HIGH(0)
	LDI  R30,LOW(_0x83*2)
	LDI  R31,HIGH(_0x83*2)
	CALL __INITLOCB
	CALL __SAVELOCR6
;	hr -> R16,R17
;	min -> R18,R19
;	sec -> R20,R21
;	Switch_val -> Y+26
;	Message -> Y+6
	LDI  R30,LOW(0)
	STS  _wrong_flag,R30
	STS  _wrong_flag+1,R30
; 0000 0126     LCD_Pos(1,0);
	CALL SUBOPT_0x12
; 0000 0127     LCD_Str(Erase);
; 0000 0128     index = 0;
	LDI  R30,LOW(0)
	STS  _index,R30
	STS  _index+1,R30
; 0000 0129     B_on = 0;
	CALL SUBOPT_0x15
; 0000 012A     puts_USART1("\renter hour : ");
	__POINTW1MN _0x84,0
	CALL SUBOPT_0x23
; 0000 012B     while(1)
_0x85:
; 0000 012C     {
; 0000 012D         if(Mode_flag != 3)
	CALL SUBOPT_0x24
	BREQ _0x88
; 0000 012E         {
; 0000 012F             LCD_Clear();
	RCALL _LCD_Clear
; 0000 0130             wrong_flag = 1;
	LDI  R30,LOW(1)
	LDI  R31,HIGH(1)
	STS  _wrong_flag,R30
	STS  _wrong_flag+1,R31
; 0000 0131             break;
	RJMP _0x87
; 0000 0132         }
; 0000 0133         if(index > 5)
_0x88:
	LDS  R26,_index
	LDS  R27,_index+1
	SBIW R26,6
	BRLT _0x85
; 0000 0134         {
; 0000 0135             break;
; 0000 0136         }
; 0000 0137     }
_0x87:
; 0000 0138     puts_USART1("\n\r Time Setting Ok!\n\r");
	__POINTW1MN _0x84,15
	CALL SUBOPT_0x23
; 0000 0139     if(wrong_flag == 0)
	LDS  R30,_wrong_flag
	LDS  R31,_wrong_flag+1
	SBIW R30,0
	BREQ PC+3
	JMP _0x8A
; 0000 013A     {
; 0000 013B         hr = (Time[0]-48)*10 + (Time[1]-48);
	LDS  R30,_Time
	CALL SUBOPT_0x25
	CALL SUBOPT_0x26
	__GETB1MN _Time,1
	CALL SUBOPT_0x25
	ADD  R30,R26
	ADC  R31,R27
	MOVW R16,R30
; 0000 013C         min = (Time[2]-48)*10 + (Time[3]-48);
	__GETB1MN _Time,2
	CALL SUBOPT_0x25
	CALL SUBOPT_0x26
	__GETB1MN _Time,3
	CALL SUBOPT_0x25
	ADD  R30,R26
	ADC  R31,R27
	MOVW R18,R30
; 0000 013D         sec = (Time[4]-48)*10 + (Time[5]-48);
	__GETB1MN _Time,4
	CALL SUBOPT_0x25
	CALL SUBOPT_0x26
	__GETB1MN _Time,5
	CALL SUBOPT_0x25
	ADD  R30,R26
	ADC  R31,R27
	MOVW R20,R30
; 0000 013E         LCD_Pos(0,1);
	CALL SUBOPT_0x5
; 0000 013F         LCD_Str(Erase);
	LDI  R30,LOW(_Erase)
	LDI  R31,HIGH(_Erase)
	CALL SUBOPT_0x6
; 0000 0140         LCD_Pos(0,1);
; 0000 0141         sprintf(Message,"%2dh %2dm %2ds",hr,min,sec);
	CALL SUBOPT_0x27
; 0000 0142         LCD_Str(Message);
; 0000 0143 
; 0000 0144         LCD_Pos(1,0);
	CALL SUBOPT_0x18
; 0000 0145         LCD_Str("select updown");
	__POINTW1MN _0x84,37
	CALL SUBOPT_0x19
; 0000 0146         no_interrupt_flag = 1;
	LDI  R30,LOW(1)
	LDI  R31,HIGH(1)
	STS  _no_interrupt_flag,R30
	STS  _no_interrupt_flag+1,R31
; 0000 0147         reservation_mode = 1;
	STS  _reservation_mode,R30
	STS  _reservation_mode+1,R31
; 0000 0148         Mo_time_count = 3; //맵핑되지 않은 정수 대입
	LDI  R30,LOW(3)
	LDI  R31,HIGH(3)
	STS  _Mo_time_count,R30
	STS  _Mo_time_count+1,R31
; 0000 0149         while(1)
_0x8B:
; 0000 014A         {
; 0000 014B             Switch_val =  Get_ADC(Mode_Sel);
	LDI  R30,LOW(5)
	LDI  R31,HIGH(5)
	CALL SUBOPT_0x1D
	STD  Y+26,R30
	STD  Y+26+1,R31
; 0000 014C             if(Switch_val > 1000)
	LDD  R26,Y+26
	LDD  R27,Y+26+1
	CPI  R26,LOW(0x3E9)
	LDI  R30,HIGH(0x3E9)
	CPC  R27,R30
	BRLT _0x8E
; 0000 014D             {
; 0000 014E                 if(Mo_time_count == 0)
	LDS  R30,_Mo_time_count
	LDS  R31,_Mo_time_count+1
	SBIW R30,0
	BRNE _0x8F
; 0000 014F                 {
; 0000 0150                     Mo_time_count = 1;
	LDI  R30,LOW(1)
	LDI  R31,HIGH(1)
	STS  _Mo_time_count,R30
	STS  _Mo_time_count+1,R31
; 0000 0151                     LCD_Pos(1,0);
	CALL SUBOPT_0x12
; 0000 0152                     LCD_Str(Erase);
; 0000 0153                     LCD_Pos(1,0);
	CALL SUBOPT_0x18
; 0000 0154                     LCD_Str("Up reservation");
	__POINTW1MN _0x84,51
	RJMP _0x110
; 0000 0155                     no_interrupt_flag = 0;
; 0000 0156                 }
; 0000 0157                 else
_0x8F:
; 0000 0158                 {
; 0000 0159                     Mo_time_count = 0;
	LDI  R30,LOW(0)
	STS  _Mo_time_count,R30
	STS  _Mo_time_count+1,R30
; 0000 015A                     LCD_Pos(1,0);
	CALL SUBOPT_0x12
; 0000 015B                     LCD_Str(Erase);
; 0000 015C                     LCD_Pos(1,0);
	CALL SUBOPT_0x18
; 0000 015D                     LCD_Str("Down reservation");
	__POINTW1MN _0x84,66
_0x110:
	ST   -Y,R31
	ST   -Y,R30
	RCALL _LCD_Str
; 0000 015E                     no_interrupt_flag = 0;
	LDI  R30,LOW(0)
	STS  _no_interrupt_flag,R30
	STS  _no_interrupt_flag+1,R30
; 0000 015F                 }
; 0000 0160                 Count = 0;
	CALL SUBOPT_0x1E
; 0000 0161                 while(Count<125)
_0x91:
	CALL SUBOPT_0x1F
	BRGE _0x93
; 0000 0162                 {SSound(Buzz);}
	CALL SUBOPT_0x20
	RJMP _0x91
_0x93:
; 0000 0163                 delay_ms(100);
	CALL SUBOPT_0x11
; 0000 0164             }
; 0000 0165             if(reservation_mode == 0)
_0x8E:
	LDS  R30,_reservation_mode
	LDS  R31,_reservation_mode+1
	SBIW R30,0
	BRNE _0x94
; 0000 0166             {
; 0000 0167                 delay_ms(500);
	LDI  R30,LOW(500)
	LDI  R31,HIGH(500)
	CALL SUBOPT_0x2
; 0000 0168                 break;
	RJMP _0x8D
; 0000 0169             }
; 0000 016A         }
_0x94:
	RJMP _0x8B
_0x8D:
; 0000 016B         Count = 0;
	CALL SUBOPT_0x1E
; 0000 016C         while(1)
_0x95:
; 0000 016D         {
; 0000 016E             if(Count >999)
	CALL SUBOPT_0x28
	CPI  R26,LOW(0x3E8)
	LDI  R30,HIGH(0x3E8)
	CPC  R27,R30
	BRLT _0x98
; 0000 016F             {
; 0000 0170                 Count = 0;
	CALL SUBOPT_0x1E
; 0000 0171                 if(sec == 0)
	MOV  R0,R20
	OR   R0,R21
	BRNE _0x99
; 0000 0172                 {
; 0000 0173                     if(min == 0)
	MOV  R0,R18
	OR   R0,R19
	BRNE _0x9A
; 0000 0174                     {
; 0000 0175                         if(hr == 0)
	MOV  R0,R16
	OR   R0,R17
	BRNE _0x9B
; 0000 0176                         {
; 0000 0177                             hr = 0;
	__GETWRN 16,17,0
; 0000 0178                             min = 0;
	__GETWRN 18,19,0
; 0000 0179                             sec = 0;
	__GETWRN 20,21,0
; 0000 017A                         }
; 0000 017B                         else
	RJMP _0x9C
_0x9B:
; 0000 017C                         {
; 0000 017D                             hr--;
	__SUBWRN 16,17,1
; 0000 017E                             min = 59;
	__GETWRN 18,19,59
; 0000 017F                             sec = 59;
	__GETWRN 20,21,59
; 0000 0180                         }
_0x9C:
; 0000 0181                     }
; 0000 0182                     else
	RJMP _0x9D
_0x9A:
; 0000 0183                     {
; 0000 0184                         min--;
	__SUBWRN 18,19,1
; 0000 0185                         sec = 59;
	__GETWRN 20,21,59
; 0000 0186                     }
_0x9D:
; 0000 0187                 }
; 0000 0188                 else
	RJMP _0x9E
_0x99:
; 0000 0189                 {
; 0000 018A                     sec--;
	__SUBWRN 20,21,1
; 0000 018B                 }
_0x9E:
; 0000 018C                 LCD_Pos(0,1);
	CALL SUBOPT_0x5
; 0000 018D                 sprintf(Message,"%2dh %2dm %2ds",hr,min,sec);
	CALL SUBOPT_0x27
; 0000 018E                 LCD_Str(Message);
; 0000 018F             }
; 0000 0190             if(Mode_flag != 3)
_0x98:
	CALL SUBOPT_0x24
	BREQ _0x9F
; 0000 0191             {
; 0000 0192                 LCD_Clear();
	RCALL _LCD_Clear
; 0000 0193                 break;
	RJMP _0x97
; 0000 0194             }
; 0000 0195             if(hr == 0 && min == 00 && sec == 0)
_0x9F:
	CLR  R0
	CP   R0,R16
	CPC  R0,R17
	BRNE _0xA1
	CLR  R0
	CP   R0,R18
	CPC  R0,R19
	BRNE _0xA1
	CLR  R0
	CP   R0,R20
	CPC  R0,R21
	BREQ _0xA2
_0xA1:
	RJMP _0xA0
_0xA2:
; 0000 0196             {
; 0000 0197                 Mode_flag = 0;
	CLR  R12
	CLR  R13
; 0000 0198                 if(Mo_time_count == 1)
	LDS  R26,_Mo_time_count
	LDS  R27,_Mo_time_count+1
	SBIW R26,1
	BREQ PC+3
	JMP _0xA3
; 0000 0199                 {//up reservation
; 0000 019A                     Count = 0;
	CALL SUBOPT_0x1E
; 0000 019B                     while(Count<250)
_0xA4:
	CALL SUBOPT_0x29
	BRGE _0xA6
; 0000 019C                     {SSound(478);}
	LDI  R30,LOW(478)
	LDI  R31,HIGH(478)
	CALL SUBOPT_0x2A
	RJMP _0xA4
_0xA6:
; 0000 019D                     Count = 0;
	CALL SUBOPT_0x1E
; 0000 019E                     while(Count<250)
_0xA7:
	CALL SUBOPT_0x29
	BRGE _0xA9
; 0000 019F                     {SSound(451);}
	LDI  R30,LOW(451)
	LDI  R31,HIGH(451)
	CALL SUBOPT_0x2A
	RJMP _0xA7
_0xA9:
; 0000 01A0                     Count = 0;
	CALL SUBOPT_0x1E
; 0000 01A1                     while(Count<250)
_0xAA:
	CALL SUBOPT_0x29
	BRGE _0xAC
; 0000 01A2                     {SSound(426);}
	LDI  R30,LOW(426)
	LDI  R31,HIGH(426)
	CALL SUBOPT_0x2A
	RJMP _0xAA
_0xAC:
; 0000 01A3                     while(1)
_0xAD:
; 0000 01A4                     {
; 0000 01A5                         if(Get_ADC(Up_max) > 600)
	CALL SUBOPT_0x1B
	CPI  R30,LOW(0x259)
	LDI  R26,HIGH(0x259)
	CPC  R31,R26
	BRLT _0xB0
; 0000 01A6                         {
; 0000 01A7                             motor_up();
	CALL SUBOPT_0x1C
; 0000 01A8                             LCD_Pos(1,0);
; 0000 01A9                             LCD_Str(Erase);
; 0000 01AA                             LCD_Pos(1,0);
	CALL SUBOPT_0x18
; 0000 01AB                             LCD_Str("Curtain up ");
	__POINTW1MN _0x84,83
	CALL SUBOPT_0x19
; 0000 01AC                             Count = 0;
	CALL SUBOPT_0x1E
; 0000 01AD                             while(Count<125)
_0xB1:
	CALL SUBOPT_0x1F
	BRGE _0xB3
; 0000 01AE                             {SSound(Buzz);}
	CALL SUBOPT_0x20
	RJMP _0xB1
_0xB3:
; 0000 01AF                             delay_ms(20);
	CALL SUBOPT_0x21
; 0000 01B0                             motor_stop();
	RCALL _motor_stop
; 0000 01B1                         }
; 0000 01B2                         else
	RJMP _0xB4
_0xB0:
; 0000 01B3                         {
; 0000 01B4                             LCD_Pos(1,0);
	CALL SUBOPT_0x12
; 0000 01B5                             LCD_Str(Erase);
; 0000 01B6                             LCD_Pos(1,5);
	CALL SUBOPT_0x1A
; 0000 01B7                             LCD_Str("Up_Max");
	__POINTW1MN _0x84,95
	CALL SUBOPT_0x19
; 0000 01B8                             Count = 0;
	CALL SUBOPT_0x1E
; 0000 01B9                             while(Count<125)
_0xB5:
	CALL SUBOPT_0x1F
	BRGE _0xB7
; 0000 01BA                             {SSound(warning);}
	CALL SUBOPT_0x22
	RJMP _0xB5
_0xB7:
; 0000 01BB                             delay_ms(200);
	CALL SUBOPT_0x10
; 0000 01BC                             break;
	RJMP _0xAF
; 0000 01BD                         }
_0xB4:
; 0000 01BE                     }
	RJMP _0xAD
_0xAF:
; 0000 01BF                     if(Mode_flag != 3)
	CALL SUBOPT_0x24
	BREQ _0xB8
; 0000 01C0                     {
; 0000 01C1                         LCD_Clear();
	RCALL _LCD_Clear
; 0000 01C2                         break;
	RJMP _0x97
; 0000 01C3                     }
; 0000 01C4                 }
_0xB8:
; 0000 01C5                 else
	RJMP _0xB9
_0xA3:
; 0000 01C6                 {//down reservation
; 0000 01C7                     Count = 0;
	CALL SUBOPT_0x1E
; 0000 01C8                     while(Count<250)
_0xBA:
	CALL SUBOPT_0x29
	BRGE _0xBC
; 0000 01C9                     {SSound(97.9988);}
	__GETD1N 0x61
	CALL SUBOPT_0x2A
	RJMP _0xBA
_0xBC:
; 0000 01CA                     Count = 0;
	CALL SUBOPT_0x1E
; 0000 01CB                     while(Count<250)
_0xBD:
	CALL SUBOPT_0x29
	BRGE _0xBF
; 0000 01CC                     {SSound(82.407);}
	__GETD1N 0x52
	CALL SUBOPT_0x2A
	RJMP _0xBD
_0xBF:
; 0000 01CD                     Count = 0;
	CALL SUBOPT_0x1E
; 0000 01CE                     while(Count<250)
_0xC0:
	CALL SUBOPT_0x29
	BRGE _0xC2
; 0000 01CF                     {SSound(65.4064);}
	__GETD1N 0x41
	CALL SUBOPT_0x2A
	RJMP _0xC0
_0xC2:
; 0000 01D0                     while(1)
_0xC3:
; 0000 01D1                     {
; 0000 01D2                         if(Get_ADC(Down_max) < 600)
	CALL SUBOPT_0x16
	CPI  R30,LOW(0x258)
	LDI  R26,HIGH(0x258)
	CPC  R31,R26
	BRGE _0xC6
; 0000 01D3                         {
; 0000 01D4                             motor_down();
	CALL SUBOPT_0x17
; 0000 01D5                             LCD_Pos(1,0);
; 0000 01D6                             LCD_Str(Erase);
; 0000 01D7                             LCD_Pos(1,0);
	CALL SUBOPT_0x18
; 0000 01D8                             LCD_Str("Curtain Down");
	__POINTW1MN _0x84,102
	CALL SUBOPT_0x19
; 0000 01D9                             Count = 0;
	CALL SUBOPT_0x1E
; 0000 01DA                             while(Count<125)
_0xC7:
	CALL SUBOPT_0x1F
	BRGE _0xC9
; 0000 01DB                             {SSound(Buzz);}
	CALL SUBOPT_0x20
	RJMP _0xC7
_0xC9:
; 0000 01DC                             delay_ms(20);
	CALL SUBOPT_0x21
; 0000 01DD                             motor_stop();
	RCALL _motor_stop
; 0000 01DE                         }
; 0000 01DF                         else
	RJMP _0xCA
_0xC6:
; 0000 01E0                         {
; 0000 01E1                             LCD_Pos(1,0);
	CALL SUBOPT_0x12
; 0000 01E2                             LCD_Str(Erase);
; 0000 01E3                             LCD_Pos(1,5);
	CALL SUBOPT_0x1A
; 0000 01E4                             LCD_Str("Down_Max");
	__POINTW1MN _0x84,115
	CALL SUBOPT_0x19
; 0000 01E5                             Count = 0;
	CALL SUBOPT_0x1E
; 0000 01E6                             while(Count<125)
_0xCB:
	CALL SUBOPT_0x1F
	BRGE _0xCD
; 0000 01E7                             {SSound(warning);}
	CALL SUBOPT_0x22
	RJMP _0xCB
_0xCD:
; 0000 01E8                             delay_ms(200);
	CALL SUBOPT_0x10
; 0000 01E9                             break;
	RJMP _0xC5
; 0000 01EA                         }
_0xCA:
; 0000 01EB                     }
	RJMP _0xC3
_0xC5:
; 0000 01EC                     if(Mode_flag != 3)
	CALL SUBOPT_0x24
	BREQ _0xCE
; 0000 01ED                     {
; 0000 01EE                         LCD_Clear();
	RCALL _LCD_Clear
; 0000 01EF                         break;
	RJMP _0x97
; 0000 01F0                     }
; 0000 01F1                 }
_0xCE:
_0xB9:
; 0000 01F2             }
; 0000 01F3 
; 0000 01F4         }
_0xA0:
	RJMP _0x95
_0x97:
; 0000 01F5     }
; 0000 01F6     else
	RJMP _0xCF
_0x8A:
; 0000 01F7     {
; 0000 01F8         Mode_flag = 0;
	CLR  R12
	CLR  R13
; 0000 01F9     }
_0xCF:
; 0000 01FA 
; 0000 01FB 
; 0000 01FC }
	CALL __LOADLOCR6
	ADIW R28,28
	RET

	.DSEG
_0x84:
	.BYTE 0x7C
;
;void Mode_select()
; 0000 01FF {

	.CSEG
_Mode_select:
; 0000 0200     int Switch_val = 0;
; 0000 0201     LCD_Pos(1,0);
	CALL SUBOPT_0x7
;	Switch_val -> R16,R17
	CALL SUBOPT_0x12
; 0000 0202     LCD_Str(Erase);
; 0000 0203     while(1)
_0xD0:
; 0000 0204     {
; 0000 0205         Switch_val =  Get_ADC(Mode_Sel);
	LDI  R30,LOW(5)
	LDI  R31,HIGH(5)
	CALL SUBOPT_0x1D
	MOVW R16,R30
; 0000 0206         if(Switch_val > 1000)
	__CPWRN 16,17,1001
	BRLT _0xD3
; 0000 0207         {
; 0000 0208             if(Mo_count >= 3)
	CALL SUBOPT_0x2B
	SBIW R26,3
	BRLT _0xD4
; 0000 0209             {
; 0000 020A                 Mo_count = 1;
	LDI  R30,LOW(1)
	LDI  R31,HIGH(1)
	STS  _Mo_count,R30
	STS  _Mo_count+1,R31
; 0000 020B             }
; 0000 020C             else
	RJMP _0xD5
_0xD4:
; 0000 020D             {
; 0000 020E                 Mo_count++;
	LDI  R26,LOW(_Mo_count)
	LDI  R27,HIGH(_Mo_count)
	CALL SUBOPT_0x2C
; 0000 020F             }
_0xD5:
; 0000 0210             Count = 0;
	CALL SUBOPT_0x1E
; 0000 0211             while(Count<125)
_0xD6:
	CALL SUBOPT_0x1F
	BRGE _0xD8
; 0000 0212             {SSound(Buzz);}
	CALL SUBOPT_0x20
	RJMP _0xD6
_0xD8:
; 0000 0213             delay_ms(100);
	RJMP _0x111
; 0000 0214         }
; 0000 0215         else if(Switch_val < 100)
_0xD3:
	__CPWRN 16,17,100
	BRGE _0xDA
; 0000 0216         {
; 0000 0217             if(Mo_count <= 1)
	CALL SUBOPT_0x2B
	SBIW R26,2
	BRGE _0xDB
; 0000 0218             {
; 0000 0219                 Mo_count = 3;
	LDI  R30,LOW(3)
	LDI  R31,HIGH(3)
	STS  _Mo_count,R30
	STS  _Mo_count+1,R31
; 0000 021A             }
; 0000 021B             else
	RJMP _0xDC
_0xDB:
; 0000 021C             {
; 0000 021D                 Mo_count--;
	LDI  R26,LOW(_Mo_count)
	LDI  R27,HIGH(_Mo_count)
	LD   R30,X+
	LD   R31,X+
	SBIW R30,1
	ST   -X,R31
	ST   -X,R30
; 0000 021E             }
_0xDC:
; 0000 021F             Count = 0;
	CALL SUBOPT_0x1E
; 0000 0220             while(Count<125)
_0xDD:
	CALL SUBOPT_0x1F
	BRGE _0xDF
; 0000 0221             {SSound(Buzz);}
	CALL SUBOPT_0x20
	RJMP _0xDD
_0xDF:
; 0000 0222             delay_ms(100);
_0x111:
	LDI  R30,LOW(100)
	LDI  R31,HIGH(100)
	CALL SUBOPT_0x2
; 0000 0223         }
; 0000 0224         if(Mo_count == 0)
_0xDA:
	LDS  R30,_Mo_count
	LDS  R31,_Mo_count+1
	SBIW R30,0
	BRNE _0xE0
; 0000 0225         {
; 0000 0226             LCD_Pos(1,0);
	CALL SUBOPT_0x18
; 0000 0227             LCD_Str("Start Mode ");
	__POINTW1MN _0xE1,0
	RJMP _0x112
; 0000 0228         }
; 0000 0229         else if(Mo_count == 1)
_0xE0:
	CALL SUBOPT_0x2B
	SBIW R26,1
	BRNE _0xE3
; 0000 022A         {
; 0000 022B             LCD_Pos(1,0);
	CALL SUBOPT_0x18
; 0000 022C             LCD_Str("Auto Mode  ");
	__POINTW1MN _0xE1,12
	RJMP _0x112
; 0000 022D         }
; 0000 022E         else if(Mo_count == 2)
_0xE3:
	CALL SUBOPT_0x2B
	SBIW R26,2
	BRNE _0xE5
; 0000 022F         {
; 0000 0230             LCD_Pos(1,0);
	CALL SUBOPT_0x18
; 0000 0231             LCD_Str("Manual Mode");
	__POINTW1MN _0xE1,24
	RJMP _0x112
; 0000 0232         }
; 0000 0233         else if(Mo_count == 3)
_0xE5:
	CALL SUBOPT_0x2B
	SBIW R26,3
	BRNE _0xE7
; 0000 0234         {
; 0000 0235             LCD_Pos(1,0);
	CALL SUBOPT_0x18
; 0000 0236             LCD_Str("Timer Mode ");
	__POINTW1MN _0xE1,36
_0x112:
	ST   -Y,R31
	ST   -Y,R30
	RCALL _LCD_Str
; 0000 0237         }
; 0000 0238         if(Mo_exit == 1)
_0xE7:
	LDS  R26,_Mo_exit
	LDS  R27,_Mo_exit+1
	SBIW R26,1
	BRNE _0xE8
; 0000 0239         {
; 0000 023A             Mode_flag = Mo_count;
	__GETWRMN 12,13,0,_Mo_count
; 0000 023B             Mo_exit = 0;
	LDI  R30,LOW(0)
	STS  _Mo_exit,R30
	STS  _Mo_exit+1,R30
; 0000 023C             break;
	RJMP _0xD2
; 0000 023D         }
; 0000 023E     }
_0xE8:
	RJMP _0xD0
_0xD2:
; 0000 023F }
_0x2060002:
	LD   R16,Y+
	LD   R17,Y+
	RET

	.DSEG
_0xE1:
	.BYTE 0x30
;
;interrupt [USART1_RXC] void usart1_receive(void)
; 0000 0242 {

	.CSEG
_usart1_receive:
	CALL SUBOPT_0x9
; 0000 0243     unsigned char str;
; 0000 0244     str = UDR1;
	ST   -Y,R17
;	str -> R17
	LDS  R17,156
; 0000 0245     if(str >= 0x30 && str <= 0x39)
	CPI  R17,48
	BRLO _0xEA
	CPI  R17,58
	BRLO _0xEB
_0xEA:
	RJMP _0xE9
_0xEB:
; 0000 0246     {
; 0000 0247         if(index >5)
	CALL SUBOPT_0x2D
	SBIW R26,6
	BRLT _0xEC
; 0000 0248         {
; 0000 0249             index = 6;
	LDI  R30,LOW(6)
	LDI  R31,HIGH(6)
	STS  _index,R30
	STS  _index+1,R31
; 0000 024A         }
; 0000 024B         else
	RJMP _0xED
_0xEC:
; 0000 024C         {
; 0000 024D             if(index == 0 || index == 2 ||index == 4)
	CALL SUBOPT_0x2D
	SBIW R26,0
	BREQ _0xEF
	CALL SUBOPT_0x2D
	SBIW R26,2
	BREQ _0xEF
	CALL SUBOPT_0x2D
	SBIW R26,4
	BRNE _0xEE
_0xEF:
; 0000 024E             {
; 0000 024F                 if(str >= 0x30 && str <= 0x35)
	CPI  R17,48
	BRLO _0xF2
	CPI  R17,54
	BRLO _0xF3
_0xF2:
	RJMP _0xF1
_0xF3:
; 0000 0250                 {
; 0000 0251                     Time[index] = str;
	CALL SUBOPT_0x2E
; 0000 0252                     index++;
; 0000 0253                 }
; 0000 0254                 else
	RJMP _0xF4
_0xF1:
; 0000 0255                 {
; 0000 0256                     puts_USART1("\r\nerror retry\n\r");
	__POINTW1MN _0xF5,0
	CALL SUBOPT_0x23
; 0000 0257                     index = 0;
	LDI  R30,LOW(0)
	STS  _index,R30
	STS  _index+1,R30
; 0000 0258 
; 0000 0259                 }
_0xF4:
; 0000 025A             }
; 0000 025B             else
	RJMP _0xF6
_0xEE:
; 0000 025C             {
; 0000 025D                 Time[index] = str;
	CALL SUBOPT_0x2E
; 0000 025E                 index++;
; 0000 025F             }
_0xF6:
; 0000 0260             if(index == 2 || index == 4)
	CALL SUBOPT_0x2D
	SBIW R26,2
	BREQ _0xF8
	CALL SUBOPT_0x2D
	SBIW R26,4
	BRNE _0xF7
_0xF8:
; 0000 0261             {
; 0000 0262                 putch_USART1(':');
	LDI  R30,LOW(58)
	ST   -Y,R30
	RCALL _putch_USART1
; 0000 0263             }
; 0000 0264         }
_0xF7:
_0xED:
; 0000 0265     }
; 0000 0266 }
_0xE9:
	LD   R17,Y+
	RJMP _0x113

	.DSEG
_0xF5:
	.BYTE 0x10
;
;interrupt [EXT_INT0] void ext_int0_isr(void)
; 0000 0269 {

	.CSEG
_ext_int0_isr:
	CALL SUBOPT_0xF
; 0000 026A     delay_ms(100);
; 0000 026B     Treshold = Get_ADC(Bright_check);
	CALL SUBOPT_0xC
	STS  _Treshold,R30
	STS  _Treshold+1,R31
; 0000 026C     delay_ms(20);
	CALL SUBOPT_0x21
; 0000 026D }
_0x113:
	LD   R30,Y+
	OUT  SREG,R30
	LD   R31,Y+
	LD   R30,Y+
	LD   R27,Y+
	LD   R26,Y+
	LD   R25,Y+
	LD   R24,Y+
	LD   R23,Y+
	LD   R22,Y+
	LD   R15,Y+
	LD   R1,Y+
	LD   R0,Y+
	RETI
;
;void main(void)
; 0000 0270 {
_main:
; 0000 0271     DDRB = 0xff;
	LDI  R30,LOW(255)
	OUT  0x17,R30
; 0000 0272     DDRC = 0xff;
	OUT  0x14,R30
; 0000 0273     PORTC = 0xff;
	OUT  0x15,R30
; 0000 0274     PORTB.0 = 0;
	CBI  0x18,0
; 0000 0275     PORTB.1 = 0;
	CBI  0x18,1
; 0000 0276     PORTB.7 = 0;
	CBI  0x18,7
; 0000 0277     DDRG|=(1<<4);
	LDS  R30,100
	ORI  R30,0x10
	STS  100,R30
; 0000 0278     LCD_Init();
	RCALL _LCD_Init
; 0000 0279     PinE_4_init();
	RCALL _PinE_4_init
; 0000 027A     PinD_0_init();
	RCALL _PinD_0_init
; 0000 027B     Timer2_Init();
	RCALL _Timer2_Init
; 0000 027C     Timer0_Init();
	RCALL _Timer0_Init
; 0000 027D     TCCR2 |= CS21;
	IN   R30,0x25
	ORI  R30,1
	OUT  0x25,R30
; 0000 027E     Init_USART1();
	RCALL _Init_USART1
; 0000 027F     LCD_High_line(Mode_flag);
	CALL SUBOPT_0x2F
; 0000 0280     while(1)
_0x100:
; 0000 0281     {
; 0000 0282         if(Mode_flag == 1)
	LDI  R30,LOW(1)
	LDI  R31,HIGH(1)
	CP   R30,R12
	CPC  R31,R13
	BRNE _0x103
; 0000 0283         {
; 0000 0284             LCD_High_line(Mode_flag);
	CALL SUBOPT_0x2F
; 0000 0285             Auto_mode();
	RCALL _Auto_mode
; 0000 0286         }
; 0000 0287         else if(Mode_flag == 2)
	RJMP _0x104
_0x103:
	LDI  R30,LOW(2)
	LDI  R31,HIGH(2)
	CP   R30,R12
	CPC  R31,R13
	BRNE _0x105
; 0000 0288         {
; 0000 0289             LCD_High_line(Mode_flag);
	CALL SUBOPT_0x2F
; 0000 028A             Manual_mode();
	RCALL _Manual_mode
; 0000 028B         }
; 0000 028C         else if(Mode_flag == 3)
	RJMP _0x106
_0x105:
	CALL SUBOPT_0x24
	BRNE _0x107
; 0000 028D         {
; 0000 028E             LCD_High_line(Mode_flag);
	CALL SUBOPT_0x2F
; 0000 028F             Timer_mode();
	RCALL _Timer_mode
; 0000 0290         }
; 0000 0291         else if(Mode_flag ==0)
	RJMP _0x108
_0x107:
	MOV  R0,R12
	OR   R0,R13
	BRNE _0x109
; 0000 0292         {
; 0000 0293             LCD_High_line(Mode_flag);
	CALL SUBOPT_0x2F
; 0000 0294             Mode_select();
	RCALL _Mode_select
; 0000 0295         }
; 0000 0296     }
_0x109:
_0x108:
_0x106:
_0x104:
	RJMP _0x100
; 0000 0297 }
_0x10A:
	RJMP _0x10A
;
	#ifndef __SLEEP_DEFINED__
	#define __SLEEP_DEFINED__
	.EQU __se_bit=0x20
	.EQU __sm_mask=0x1C
	.EQU __sm_powerdown=0x10
	.EQU __sm_powersave=0x18
	.EQU __sm_standby=0x14
	.EQU __sm_ext_standby=0x1C
	.EQU __sm_adc_noise_red=0x08
	.SET power_ctrl_reg=mcucr
	#endif

	.CSEG
_put_buff_G100:
	ST   -Y,R17
	ST   -Y,R16
	LDD  R26,Y+2
	LDD  R27,Y+2+1
	ADIW R26,2
	CALL __GETW1P
	SBIW R30,0
	BREQ _0x2000010
	LDD  R26,Y+2
	LDD  R27,Y+2+1
	ADIW R26,4
	CALL __GETW1P
	MOVW R16,R30
	SBIW R30,0
	BREQ _0x2000012
	__CPWRN 16,17,2
	BRLO _0x2000013
	MOVW R30,R16
	SBIW R30,1
	MOVW R16,R30
	__PUTW1SNS 2,4
_0x2000012:
	LDD  R26,Y+2
	LDD  R27,Y+2+1
	ADIW R26,2
	CALL SUBOPT_0xA
	LDD  R26,Y+4
	STD  Z+0,R26
	LDD  R26,Y+2
	LDD  R27,Y+2+1
	CALL __GETW1P
	TST  R31
	BRMI _0x2000014
	CALL SUBOPT_0x2C
_0x2000014:
_0x2000013:
	RJMP _0x2000015
_0x2000010:
	LDD  R26,Y+2
	LDD  R27,Y+2+1
	LDI  R30,LOW(65535)
	LDI  R31,HIGH(65535)
	ST   X+,R30
	ST   X,R31
_0x2000015:
	LDD  R17,Y+1
	LDD  R16,Y+0
	ADIW R28,5
	RET
__print_G100:
	SBIW R28,6
	CALL __SAVELOCR6
	LDI  R17,0
	LDD  R26,Y+12
	LDD  R27,Y+12+1
	LDI  R30,LOW(0)
	LDI  R31,HIGH(0)
	ST   X+,R30
	ST   X,R31
_0x2000016:
	LDD  R30,Y+18
	LDD  R31,Y+18+1
	ADIW R30,1
	STD  Y+18,R30
	STD  Y+18+1,R31
	SBIW R30,1
	LPM  R30,Z
	MOV  R18,R30
	CPI  R30,0
	BRNE PC+3
	JMP _0x2000018
	MOV  R30,R17
	CPI  R30,0
	BRNE _0x200001C
	CPI  R18,37
	BRNE _0x200001D
	LDI  R17,LOW(1)
	RJMP _0x200001E
_0x200001D:
	CALL SUBOPT_0x30
_0x200001E:
	RJMP _0x200001B
_0x200001C:
	CPI  R30,LOW(0x1)
	BRNE _0x200001F
	CPI  R18,37
	BRNE _0x2000020
	CALL SUBOPT_0x30
	RJMP _0x20000C9
_0x2000020:
	LDI  R17,LOW(2)
	LDI  R20,LOW(0)
	LDI  R16,LOW(0)
	CPI  R18,45
	BRNE _0x2000021
	LDI  R16,LOW(1)
	RJMP _0x200001B
_0x2000021:
	CPI  R18,43
	BRNE _0x2000022
	LDI  R20,LOW(43)
	RJMP _0x200001B
_0x2000022:
	CPI  R18,32
	BRNE _0x2000023
	LDI  R20,LOW(32)
	RJMP _0x200001B
_0x2000023:
	RJMP _0x2000024
_0x200001F:
	CPI  R30,LOW(0x2)
	BRNE _0x2000025
_0x2000024:
	LDI  R21,LOW(0)
	LDI  R17,LOW(3)
	CPI  R18,48
	BRNE _0x2000026
	ORI  R16,LOW(128)
	RJMP _0x200001B
_0x2000026:
	RJMP _0x2000027
_0x2000025:
	CPI  R30,LOW(0x3)
	BREQ PC+3
	JMP _0x200001B
_0x2000027:
	CPI  R18,48
	BRLO _0x200002A
	CPI  R18,58
	BRLO _0x200002B
_0x200002A:
	RJMP _0x2000029
_0x200002B:
	LDI  R26,LOW(10)
	MUL  R21,R26
	MOV  R21,R0
	MOV  R30,R18
	SUBI R30,LOW(48)
	ADD  R21,R30
	RJMP _0x200001B
_0x2000029:
	MOV  R30,R18
	CPI  R30,LOW(0x63)
	BRNE _0x200002F
	CALL SUBOPT_0x31
	LDD  R30,Y+16
	LDD  R31,Y+16+1
	LDD  R26,Z+4
	ST   -Y,R26
	CALL SUBOPT_0x32
	RJMP _0x2000030
_0x200002F:
	CPI  R30,LOW(0x73)
	BRNE _0x2000032
	CALL SUBOPT_0x31
	CALL SUBOPT_0x33
	CALL _strlen
	MOV  R17,R30
	RJMP _0x2000033
_0x2000032:
	CPI  R30,LOW(0x70)
	BRNE _0x2000035
	CALL SUBOPT_0x31
	CALL SUBOPT_0x33
	CALL _strlenf
	MOV  R17,R30
	ORI  R16,LOW(8)
_0x2000033:
	ORI  R16,LOW(2)
	ANDI R16,LOW(127)
	LDI  R19,LOW(0)
	RJMP _0x2000036
_0x2000035:
	CPI  R30,LOW(0x64)
	BREQ _0x2000039
	CPI  R30,LOW(0x69)
	BRNE _0x200003A
_0x2000039:
	ORI  R16,LOW(4)
	RJMP _0x200003B
_0x200003A:
	CPI  R30,LOW(0x75)
	BRNE _0x200003C
_0x200003B:
	LDI  R30,LOW(_tbl10_G100*2)
	LDI  R31,HIGH(_tbl10_G100*2)
	STD  Y+6,R30
	STD  Y+6+1,R31
	LDI  R17,LOW(5)
	RJMP _0x200003D
_0x200003C:
	CPI  R30,LOW(0x58)
	BRNE _0x200003F
	ORI  R16,LOW(8)
	RJMP _0x2000040
_0x200003F:
	CPI  R30,LOW(0x78)
	BREQ PC+3
	JMP _0x2000071
_0x2000040:
	LDI  R30,LOW(_tbl16_G100*2)
	LDI  R31,HIGH(_tbl16_G100*2)
	STD  Y+6,R30
	STD  Y+6+1,R31
	LDI  R17,LOW(4)
_0x200003D:
	SBRS R16,2
	RJMP _0x2000042
	CALL SUBOPT_0x31
	CALL SUBOPT_0x34
	LDD  R26,Y+11
	TST  R26
	BRPL _0x2000043
	LDD  R30,Y+10
	LDD  R31,Y+10+1
	CALL __ANEGW1
	STD  Y+10,R30
	STD  Y+10+1,R31
	LDI  R20,LOW(45)
_0x2000043:
	CPI  R20,0
	BREQ _0x2000044
	SUBI R17,-LOW(1)
	RJMP _0x2000045
_0x2000044:
	ANDI R16,LOW(251)
_0x2000045:
	RJMP _0x2000046
_0x2000042:
	CALL SUBOPT_0x31
	CALL SUBOPT_0x34
_0x2000046:
_0x2000036:
	SBRC R16,0
	RJMP _0x2000047
_0x2000048:
	CP   R17,R21
	BRSH _0x200004A
	SBRS R16,7
	RJMP _0x200004B
	SBRS R16,2
	RJMP _0x200004C
	ANDI R16,LOW(251)
	MOV  R18,R20
	SUBI R17,LOW(1)
	RJMP _0x200004D
_0x200004C:
	LDI  R18,LOW(48)
_0x200004D:
	RJMP _0x200004E
_0x200004B:
	LDI  R18,LOW(32)
_0x200004E:
	CALL SUBOPT_0x30
	SUBI R21,LOW(1)
	RJMP _0x2000048
_0x200004A:
_0x2000047:
	MOV  R19,R17
	SBRS R16,1
	RJMP _0x200004F
_0x2000050:
	CPI  R19,0
	BREQ _0x2000052
	SBRS R16,3
	RJMP _0x2000053
	LDD  R30,Y+6
	LDD  R31,Y+6+1
	LPM  R18,Z+
	STD  Y+6,R30
	STD  Y+6+1,R31
	RJMP _0x2000054
_0x2000053:
	LDD  R26,Y+6
	LDD  R27,Y+6+1
	LD   R18,X+
	STD  Y+6,R26
	STD  Y+6+1,R27
_0x2000054:
	CALL SUBOPT_0x30
	CPI  R21,0
	BREQ _0x2000055
	SUBI R21,LOW(1)
_0x2000055:
	SUBI R19,LOW(1)
	RJMP _0x2000050
_0x2000052:
	RJMP _0x2000056
_0x200004F:
_0x2000058:
	LDI  R18,LOW(48)
	LDD  R30,Y+6
	LDD  R31,Y+6+1
	CALL __GETW1PF
	STD  Y+8,R30
	STD  Y+8+1,R31
	LDD  R30,Y+6
	LDD  R31,Y+6+1
	ADIW R30,2
	STD  Y+6,R30
	STD  Y+6+1,R31
_0x200005A:
	LDD  R30,Y+8
	LDD  R31,Y+8+1
	LDD  R26,Y+10
	LDD  R27,Y+10+1
	CP   R26,R30
	CPC  R27,R31
	BRLO _0x200005C
	SUBI R18,-LOW(1)
	LDD  R26,Y+8
	LDD  R27,Y+8+1
	LDD  R30,Y+10
	LDD  R31,Y+10+1
	SUB  R30,R26
	SBC  R31,R27
	STD  Y+10,R30
	STD  Y+10+1,R31
	RJMP _0x200005A
_0x200005C:
	CPI  R18,58
	BRLO _0x200005D
	SBRS R16,3
	RJMP _0x200005E
	SUBI R18,-LOW(7)
	RJMP _0x200005F
_0x200005E:
	SUBI R18,-LOW(39)
_0x200005F:
_0x200005D:
	SBRC R16,4
	RJMP _0x2000061
	CPI  R18,49
	BRSH _0x2000063
	LDD  R26,Y+8
	LDD  R27,Y+8+1
	SBIW R26,1
	BRNE _0x2000062
_0x2000063:
	RJMP _0x20000CA
_0x2000062:
	CP   R21,R19
	BRLO _0x2000067
	SBRS R16,0
	RJMP _0x2000068
_0x2000067:
	RJMP _0x2000066
_0x2000068:
	LDI  R18,LOW(32)
	SBRS R16,7
	RJMP _0x2000069
	LDI  R18,LOW(48)
_0x20000CA:
	ORI  R16,LOW(16)
	SBRS R16,2
	RJMP _0x200006A
	ANDI R16,LOW(251)
	ST   -Y,R20
	CALL SUBOPT_0x32
	CPI  R21,0
	BREQ _0x200006B
	SUBI R21,LOW(1)
_0x200006B:
_0x200006A:
_0x2000069:
_0x2000061:
	CALL SUBOPT_0x30
	CPI  R21,0
	BREQ _0x200006C
	SUBI R21,LOW(1)
_0x200006C:
_0x2000066:
	SUBI R19,LOW(1)
	LDD  R26,Y+8
	LDD  R27,Y+8+1
	SBIW R26,2
	BRLO _0x2000059
	RJMP _0x2000058
_0x2000059:
_0x2000056:
	SBRS R16,0
	RJMP _0x200006D
_0x200006E:
	CPI  R21,0
	BREQ _0x2000070
	SUBI R21,LOW(1)
	LDI  R30,LOW(32)
	ST   -Y,R30
	CALL SUBOPT_0x32
	RJMP _0x200006E
_0x2000070:
_0x200006D:
_0x2000071:
_0x2000030:
_0x20000C9:
	LDI  R17,LOW(0)
_0x200001B:
	RJMP _0x2000016
_0x2000018:
	LDD  R26,Y+12
	LDD  R27,Y+12+1
	CALL __GETW1P
	CALL __LOADLOCR6
	ADIW R28,20
	RET
_sprintf:
	PUSH R15
	MOV  R15,R24
	SBIW R28,6
	CALL __SAVELOCR4
	CALL SUBOPT_0x35
	SBIW R30,0
	BRNE _0x2000072
	LDI  R30,LOW(65535)
	LDI  R31,HIGH(65535)
	RJMP _0x2060001
_0x2000072:
	MOVW R26,R28
	ADIW R26,6
	CALL __ADDW2R15
	MOVW R16,R26
	CALL SUBOPT_0x35
	STD  Y+6,R30
	STD  Y+6+1,R31
	LDI  R30,LOW(0)
	STD  Y+8,R30
	STD  Y+8+1,R30
	MOVW R26,R28
	ADIW R26,10
	CALL __ADDW2R15
	CALL __GETW1P
	ST   -Y,R31
	ST   -Y,R30
	ST   -Y,R17
	ST   -Y,R16
	LDI  R30,LOW(_put_buff_G100)
	LDI  R31,HIGH(_put_buff_G100)
	ST   -Y,R31
	ST   -Y,R30
	MOVW R30,R28
	ADIW R30,10
	ST   -Y,R31
	ST   -Y,R30
	RCALL __print_G100
	MOVW R18,R30
	LDD  R26,Y+6
	LDD  R27,Y+6+1
	LDI  R30,LOW(0)
	ST   X,R30
	MOVW R30,R18
_0x2060001:
	CALL __LOADLOCR4
	ADIW R28,10
	POP  R15
	RET

	.CSEG

	.CSEG
_strlen:
    ld   r26,y+
    ld   r27,y+
    clr  r30
    clr  r31
strlen0:
    ld   r22,x+
    tst  r22
    breq strlen1
    adiw r30,1
    rjmp strlen0
strlen1:
    ret
_strlenf:
    clr  r26
    clr  r27
    ld   r30,y+
    ld   r31,y+
strlenf0:
	lpm  r0,z+
    tst  r0
    breq strlenf1
    adiw r26,1
    rjmp strlenf0
strlenf1:
    movw r30,r26
    ret

	.DSEG
_average:
	.BYTE 0x14
_Erase:
	.BYTE 0x11
_bright_count:
	.BYTE 0x2
_Bright_val:
	.BYTE 0x2
_pre_Bright:
	.BYTE 0x2
_Count:
	.BYTE 0x2
_B_on:
	.BYTE 0x2
_Mo_count:
	.BYTE 0x2
_Mo_exit:
	.BYTE 0x2
_Time:
	.BYTE 0x6
_Mo_time_count:
	.BYTE 0x2
_reservation_mode:
	.BYTE 0x2
_wrong_flag:
	.BYTE 0x2
_index:
	.BYTE 0x2
_Treshold:
	.BYTE 0x2
_no_interrupt_flag:
	.BYTE 0x2

	.CSEG
;OPTIMIZER ADDED SUBROUTINE, CALLED 4 TIMES, CODE SIZE REDUCTION:3 WORDS
SUBOPT_0x0:
	STS  101,R30
	LDS  R30,101
	RET

;OPTIMIZER ADDED SUBROUTINE, CALLED 2 TIMES, CODE SIZE REDUCTION:11 WORDS
SUBOPT_0x1:
	ORI  R30,1
	STS  101,R30
	__DELAY_USB 246
	LD   R30,Y
	OUT  0x1B,R30
	__DELAY_USB 246
	LDS  R30,101
	ANDI R30,0xFE
	STS  101,R30
	RET

;OPTIMIZER ADDED SUBROUTINE, CALLED 29 TIMES, CODE SIZE REDUCTION:53 WORDS
SUBOPT_0x2:
	ST   -Y,R31
	ST   -Y,R30
	JMP  _delay_ms

;OPTIMIZER ADDED SUBROUTINE, CALLED 3 TIMES, CODE SIZE REDUCTION:7 WORDS
SUBOPT_0x3:
	ST   -Y,R30
	CALL _LCD_Comm
	LDI  R30,LOW(2)
	ST   -Y,R30
	JMP  _LCD_Delay

;OPTIMIZER ADDED SUBROUTINE, CALLED 3 TIMES, CODE SIZE REDUCTION:9 WORDS
SUBOPT_0x4:
	LDI  R30,LOW(56)
	ST   -Y,R30
	CALL _LCD_Comm
	LDI  R30,LOW(4)
	ST   -Y,R30
	JMP  _LCD_Delay

;OPTIMIZER ADDED SUBROUTINE, CALLED 11 TIMES, CODE SIZE REDUCTION:37 WORDS
SUBOPT_0x5:
	LDI  R30,LOW(0)
	ST   -Y,R30
	LDI  R30,LOW(1)
	ST   -Y,R30
	JMP  _LCD_Pos

;OPTIMIZER ADDED SUBROUTINE, CALLED 5 TIMES, CODE SIZE REDUCTION:13 WORDS
SUBOPT_0x6:
	ST   -Y,R31
	ST   -Y,R30
	CALL _LCD_Str
	RJMP SUBOPT_0x5

;OPTIMIZER ADDED SUBROUTINE, CALLED 3 TIMES, CODE SIZE REDUCTION:1 WORDS
SUBOPT_0x7:
	ST   -Y,R17
	ST   -Y,R16
	__GETWRN 16,17,0
	RET

;OPTIMIZER ADDED SUBROUTINE, CALLED 2 TIMES, CODE SIZE REDUCTION:1 WORDS
SUBOPT_0x8:
	STS  101,R30
	LD   R30,Y
	ST   -Y,R30
	JMP  _myDelay_us

;OPTIMIZER ADDED SUBROUTINE, CALLED 4 TIMES, CODE SIZE REDUCTION:30 WORDS
SUBOPT_0x9:
	ST   -Y,R0
	ST   -Y,R1
	ST   -Y,R15
	ST   -Y,R22
	ST   -Y,R23
	ST   -Y,R24
	ST   -Y,R25
	ST   -Y,R26
	ST   -Y,R27
	ST   -Y,R30
	ST   -Y,R31
	IN   R30,SREG
	ST   -Y,R30
	RET

;OPTIMIZER ADDED SUBROUTINE, CALLED 3 TIMES, CODE SIZE REDUCTION:5 WORDS
SUBOPT_0xA:
	LD   R30,X+
	LD   R31,X+
	ADIW R30,1
	ST   -X,R31
	ST   -X,R30
	SBIW R30,1
	RET

;OPTIMIZER ADDED SUBROUTINE, CALLED 7 TIMES, CODE SIZE REDUCTION:9 WORDS
SUBOPT_0xB:
	LDI  R26,LOW(_average)
	LDI  R27,HIGH(_average)
	LSL  R30
	ROL  R31
	RET

;OPTIMIZER ADDED SUBROUTINE, CALLED 3 TIMES, CODE SIZE REDUCTION:5 WORDS
SUBOPT_0xC:
	LDI  R30,LOW(4)
	LDI  R31,HIGH(4)
	ST   -Y,R31
	ST   -Y,R30
	JMP  _Get_ADC

;OPTIMIZER ADDED SUBROUTINE, CALLED 2 TIMES, CODE SIZE REDUCTION:3 WORDS
SUBOPT_0xD:
	LDS  R30,_Bright_val
	LDS  R31,_Bright_val+1
	STS  _pre_Bright,R30
	STS  _pre_Bright+1,R31
	RET

;OPTIMIZER ADDED SUBROUTINE, CALLED 6 TIMES, CODE SIZE REDUCTION:7 WORDS
SUBOPT_0xE:
	LDS  R26,_Bright_val
	LDS  R27,_Bright_val+1
	RET

;OPTIMIZER ADDED SUBROUTINE, CALLED 2 TIMES, CODE SIZE REDUCTION:1 WORDS
SUBOPT_0xF:
	RCALL SUBOPT_0x9
	LDI  R30,LOW(100)
	LDI  R31,HIGH(100)
	RJMP SUBOPT_0x2

;OPTIMIZER ADDED SUBROUTINE, CALLED 7 TIMES, CODE SIZE REDUCTION:9 WORDS
SUBOPT_0x10:
	LDI  R30,LOW(200)
	LDI  R31,HIGH(200)
	RJMP SUBOPT_0x2

;OPTIMIZER ADDED SUBROUTINE, CALLED 3 TIMES, CODE SIZE REDUCTION:1 WORDS
SUBOPT_0x11:
	LDI  R30,LOW(100)
	LDI  R31,HIGH(100)
	RJMP SUBOPT_0x2

;OPTIMIZER ADDED SUBROUTINE, CALLED 18 TIMES, CODE SIZE REDUCTION:167 WORDS
SUBOPT_0x12:
	LDI  R30,LOW(1)
	ST   -Y,R30
	LDI  R30,LOW(0)
	ST   -Y,R30
	CALL _LCD_Pos
	LDI  R30,LOW(_Erase)
	LDI  R31,HIGH(_Erase)
	ST   -Y,R31
	ST   -Y,R30
	JMP  _LCD_Str

;OPTIMIZER ADDED SUBROUTINE, CALLED 3 TIMES, CODE SIZE REDUCTION:5 WORDS
SUBOPT_0x13:
	LDI  R30,LOW(1)
	LDI  R31,HIGH(1)
	STS  _B_on,R30
	STS  _B_on+1,R31
	RET

;OPTIMIZER ADDED SUBROUTINE, CALLED 6 TIMES, CODE SIZE REDUCTION:7 WORDS
SUBOPT_0x14:
	LDI  R30,LOW(5)
	LDI  R31,HIGH(5)
	RJMP SUBOPT_0x2

;OPTIMIZER ADDED SUBROUTINE, CALLED 5 TIMES, CODE SIZE REDUCTION:9 WORDS
SUBOPT_0x15:
	LDI  R30,LOW(0)
	STS  _B_on,R30
	STS  _B_on+1,R30
	RET

;OPTIMIZER ADDED SUBROUTINE, CALLED 3 TIMES, CODE SIZE REDUCTION:5 WORDS
SUBOPT_0x16:
	LDI  R30,LOW(2)
	LDI  R31,HIGH(2)
	ST   -Y,R31
	ST   -Y,R30
	JMP  _Get_ADC

;OPTIMIZER ADDED SUBROUTINE, CALLED 3 TIMES, CODE SIZE REDUCTION:1 WORDS
SUBOPT_0x17:
	CALL _motor_down
	RJMP SUBOPT_0x12

;OPTIMIZER ADDED SUBROUTINE, CALLED 14 TIMES, CODE SIZE REDUCTION:49 WORDS
SUBOPT_0x18:
	LDI  R30,LOW(1)
	ST   -Y,R30
	LDI  R30,LOW(0)
	ST   -Y,R30
	JMP  _LCD_Pos

;OPTIMIZER ADDED SUBROUTINE, CALLED 16 TIMES, CODE SIZE REDUCTION:27 WORDS
SUBOPT_0x19:
	ST   -Y,R31
	ST   -Y,R30
	JMP  _LCD_Str

;OPTIMIZER ADDED SUBROUTINE, CALLED 6 TIMES, CODE SIZE REDUCTION:17 WORDS
SUBOPT_0x1A:
	LDI  R30,LOW(1)
	ST   -Y,R30
	LDI  R30,LOW(5)
	ST   -Y,R30
	JMP  _LCD_Pos

;OPTIMIZER ADDED SUBROUTINE, CALLED 3 TIMES, CODE SIZE REDUCTION:5 WORDS
SUBOPT_0x1B:
	LDI  R30,LOW(1)
	LDI  R31,HIGH(1)
	ST   -Y,R31
	ST   -Y,R30
	JMP  _Get_ADC

;OPTIMIZER ADDED SUBROUTINE, CALLED 3 TIMES, CODE SIZE REDUCTION:1 WORDS
SUBOPT_0x1C:
	CALL _motor_up
	RJMP SUBOPT_0x12

;OPTIMIZER ADDED SUBROUTINE, CALLED 3 TIMES, CODE SIZE REDUCTION:1 WORDS
SUBOPT_0x1D:
	ST   -Y,R31
	ST   -Y,R30
	JMP  _Get_ADC

;OPTIMIZER ADDED SUBROUTINE, CALLED 19 TIMES, CODE SIZE REDUCTION:51 WORDS
SUBOPT_0x1E:
	LDI  R30,LOW(0)
	STS  _Count,R30
	STS  _Count+1,R30
	RET

;OPTIMIZER ADDED SUBROUTINE, CALLED 11 TIMES, CODE SIZE REDUCTION:47 WORDS
SUBOPT_0x1F:
	LDS  R26,_Count
	LDS  R27,_Count+1
	CPI  R26,LOW(0x7D)
	LDI  R30,HIGH(0x7D)
	CPC  R27,R30
	RET

;OPTIMIZER ADDED SUBROUTINE, CALLED 7 TIMES, CODE SIZE REDUCTION:21 WORDS
SUBOPT_0x20:
	LDI  R30,LOW(195)
	LDI  R31,HIGH(195)
	ST   -Y,R31
	ST   -Y,R30
	JMP  _SSound

;OPTIMIZER ADDED SUBROUTINE, CALLED 5 TIMES, CODE SIZE REDUCTION:5 WORDS
SUBOPT_0x21:
	LDI  R30,LOW(20)
	LDI  R31,HIGH(20)
	RJMP SUBOPT_0x2

;OPTIMIZER ADDED SUBROUTINE, CALLED 4 TIMES, CODE SIZE REDUCTION:9 WORDS
SUBOPT_0x22:
	LDI  R30,LOW(300)
	LDI  R31,HIGH(300)
	ST   -Y,R31
	ST   -Y,R30
	JMP  _SSound

;OPTIMIZER ADDED SUBROUTINE, CALLED 3 TIMES, CODE SIZE REDUCTION:1 WORDS
SUBOPT_0x23:
	ST   -Y,R31
	ST   -Y,R30
	JMP  _puts_USART1

;OPTIMIZER ADDED SUBROUTINE, CALLED 5 TIMES, CODE SIZE REDUCTION:5 WORDS
SUBOPT_0x24:
	LDI  R30,LOW(3)
	LDI  R31,HIGH(3)
	CP   R30,R12
	CPC  R31,R13
	RET

;OPTIMIZER ADDED SUBROUTINE, CALLED 6 TIMES, CODE SIZE REDUCTION:7 WORDS
SUBOPT_0x25:
	LDI  R31,0
	SBIW R30,48
	RET

;OPTIMIZER ADDED SUBROUTINE, CALLED 3 TIMES, CODE SIZE REDUCTION:3 WORDS
SUBOPT_0x26:
	LDI  R26,LOW(10)
	LDI  R27,HIGH(10)
	CALL __MULW12
	MOVW R26,R30
	RET

;OPTIMIZER ADDED SUBROUTINE, CALLED 2 TIMES, CODE SIZE REDUCTION:26 WORDS
SUBOPT_0x27:
	MOVW R30,R28
	ADIW R30,6
	ST   -Y,R31
	ST   -Y,R30
	__POINTW1FN _0x0,168
	ST   -Y,R31
	ST   -Y,R30
	MOVW R30,R16
	CALL __CWD1
	CALL __PUTPARD1
	MOVW R30,R18
	CALL __CWD1
	CALL __PUTPARD1
	MOVW R30,R20
	CALL __CWD1
	CALL __PUTPARD1
	LDI  R24,12
	CALL _sprintf
	ADIW R28,16
	MOVW R30,R28
	ADIW R30,6
	RJMP SUBOPT_0x19

;OPTIMIZER ADDED SUBROUTINE, CALLED 7 TIMES, CODE SIZE REDUCTION:9 WORDS
SUBOPT_0x28:
	LDS  R26,_Count
	LDS  R27,_Count+1
	RET

;OPTIMIZER ADDED SUBROUTINE, CALLED 6 TIMES, CODE SIZE REDUCTION:12 WORDS
SUBOPT_0x29:
	RCALL SUBOPT_0x28
	CPI  R26,LOW(0xFA)
	LDI  R30,HIGH(0xFA)
	CPC  R27,R30
	RET

;OPTIMIZER ADDED SUBROUTINE, CALLED 6 TIMES, CODE SIZE REDUCTION:7 WORDS
SUBOPT_0x2A:
	ST   -Y,R31
	ST   -Y,R30
	JMP  _SSound

;OPTIMIZER ADDED SUBROUTINE, CALLED 5 TIMES, CODE SIZE REDUCTION:5 WORDS
SUBOPT_0x2B:
	LDS  R26,_Mo_count
	LDS  R27,_Mo_count+1
	RET

;OPTIMIZER ADDED SUBROUTINE, CALLED 4 TIMES, CODE SIZE REDUCTION:6 WORDS
SUBOPT_0x2C:
	LD   R30,X+
	LD   R31,X+
	ADIW R30,1
	ST   -X,R31
	ST   -X,R30
	RET

;OPTIMIZER ADDED SUBROUTINE, CALLED 6 TIMES, CODE SIZE REDUCTION:7 WORDS
SUBOPT_0x2D:
	LDS  R26,_index
	LDS  R27,_index+1
	RET

;OPTIMIZER ADDED SUBROUTINE, CALLED 2 TIMES, CODE SIZE REDUCTION:6 WORDS
SUBOPT_0x2E:
	LDS  R30,_index
	LDS  R31,_index+1
	SUBI R30,LOW(-_Time)
	SBCI R31,HIGH(-_Time)
	ST   Z,R17
	LDI  R26,LOW(_index)
	LDI  R27,HIGH(_index)
	RJMP SUBOPT_0x2C

;OPTIMIZER ADDED SUBROUTINE, CALLED 5 TIMES, CODE SIZE REDUCTION:5 WORDS
SUBOPT_0x2F:
	ST   -Y,R13
	ST   -Y,R12
	JMP  _LCD_High_line

;OPTIMIZER ADDED SUBROUTINE, CALLED 5 TIMES, CODE SIZE REDUCTION:21 WORDS
SUBOPT_0x30:
	ST   -Y,R18
	LDD  R30,Y+13
	LDD  R31,Y+13+1
	ST   -Y,R31
	ST   -Y,R30
	LDD  R30,Y+17
	LDD  R31,Y+17+1
	ICALL
	RET

;OPTIMIZER ADDED SUBROUTINE, CALLED 5 TIMES, CODE SIZE REDUCTION:9 WORDS
SUBOPT_0x31:
	LDD  R30,Y+16
	LDD  R31,Y+16+1
	SBIW R30,4
	STD  Y+16,R30
	STD  Y+16+1,R31
	RET

;OPTIMIZER ADDED SUBROUTINE, CALLED 3 TIMES, CODE SIZE REDUCTION:7 WORDS
SUBOPT_0x32:
	LDD  R30,Y+13
	LDD  R31,Y+13+1
	ST   -Y,R31
	ST   -Y,R30
	LDD  R30,Y+17
	LDD  R31,Y+17+1
	ICALL
	RET

;OPTIMIZER ADDED SUBROUTINE, CALLED 2 TIMES, CODE SIZE REDUCTION:6 WORDS
SUBOPT_0x33:
	LDD  R26,Y+16
	LDD  R27,Y+16+1
	ADIW R26,4
	CALL __GETW1P
	STD  Y+6,R30
	STD  Y+6+1,R31
	ST   -Y,R31
	ST   -Y,R30
	RET

;OPTIMIZER ADDED SUBROUTINE, CALLED 2 TIMES, CODE SIZE REDUCTION:2 WORDS
SUBOPT_0x34:
	LDD  R26,Y+16
	LDD  R27,Y+16+1
	ADIW R26,4
	CALL __GETW1P
	STD  Y+10,R30
	STD  Y+10+1,R31
	RET

;OPTIMIZER ADDED SUBROUTINE, CALLED 2 TIMES, CODE SIZE REDUCTION:1 WORDS
SUBOPT_0x35:
	MOVW R26,R28
	ADIW R26,12
	CALL __ADDW2R15
	CALL __GETW1P
	RET


	.CSEG
_delay_ms:
	ld   r30,y+
	ld   r31,y+
	adiw r30,0
	breq __delay_ms1
__delay_ms0:
	__DELAY_USW 0xE66
	wdr
	sbiw r30,1
	brne __delay_ms0
__delay_ms1:
	ret

__ADDW2R15:
	CLR  R0
	ADD  R26,R15
	ADC  R27,R0
	RET

__ANEGW1:
	NEG  R31
	NEG  R30
	SBCI R31,0
	RET

__CWD1:
	MOV  R22,R31
	ADD  R22,R22
	SBC  R22,R22
	MOV  R23,R22
	RET

__MULW12U:
	MUL  R31,R26
	MOV  R31,R0
	MUL  R30,R27
	ADD  R31,R0
	MUL  R30,R26
	MOV  R30,R0
	ADD  R31,R1
	RET

__MULW12:
	RCALL __CHKSIGNW
	RCALL __MULW12U
	BRTC __MULW121
	RCALL __ANEGW1
__MULW121:
	RET

__CHKSIGNW:
	CLT
	SBRS R31,7
	RJMP __CHKSW1
	RCALL __ANEGW1
	SET
__CHKSW1:
	SBRS R27,7
	RJMP __CHKSW2
	COM  R26
	COM  R27
	ADIW R26,1
	BLD  R0,0
	INC  R0
	BST  R0,0
__CHKSW2:
	RET

__GETW1P:
	LD   R30,X+
	LD   R31,X
	SBIW R26,1
	RET

__GETW1PF:
	LPM  R0,Z+
	LPM  R31,Z
	MOV  R30,R0
	RET

__PUTPARD1:
	ST   -Y,R23
	ST   -Y,R22
	ST   -Y,R31
	ST   -Y,R30
	RET

__SAVELOCR6:
	ST   -Y,R21
__SAVELOCR5:
	ST   -Y,R20
__SAVELOCR4:
	ST   -Y,R19
__SAVELOCR3:
	ST   -Y,R18
__SAVELOCR2:
	ST   -Y,R17
	ST   -Y,R16
	RET

__LOADLOCR6:
	LDD  R21,Y+5
__LOADLOCR5:
	LDD  R20,Y+4
__LOADLOCR4:
	LDD  R19,Y+3
__LOADLOCR3:
	LDD  R18,Y+2
__LOADLOCR2:
	LDD  R17,Y+1
	LD   R16,Y
	RET

__INITLOCB:
__INITLOCW:
	ADD  R26,R28
	ADC  R27,R29
__INITLOC0:
	LPM  R0,Z+
	ST   X+,R0
	DEC  R24
	BRNE __INITLOC0
	RET

;END OF CODE MARKER
__END_OF_CODE:
