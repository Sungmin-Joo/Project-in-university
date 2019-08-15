/* This file is FPGA-DOT MATRIX arrow file.
   FILE : fpga_dot_arrow.h
   AUTH : Hong, Sung-Hyun
          Huins, Inc. */

#ifndef __FPGA_NUMBER__
#define __FPGA_NUMBER__

unsigned char fpga_number[10][10] = {
	
	{0b00000000,
	 0b00001000,
	 0b00001100,
	 0b01111110,
	 0b01111111,
	 0b01111110,
	 0b00001100,
	 0b00001000,
	 0b00000000,
	 0b00000000}, // ->

	{0b00000000,
	 0b00000000,
	 0b00011111,
	 0b00001111,
	 0b00011111,
	 0b00111111,
	 0b01111101,
	 0b00111000,
	 0b00010000,
	 0b00000000},  // ->^

	{0b00001000,
	 0b00011100,
	 0b00111110,
	 0b01111111,
	 0b00011100,
	 0b00011100,
	 0b00011100,
	 0b00011100,
	 0b00011100,
	 0b00000000},  // ^

	{0b00000000,
	 0b00000000,
	 0b01111100,
	 0b01111000,
	 0b01111100,
	 0b01111110,
	 0b01011111,
	 0b00001110,
	 0b00000100,
	 0b00000000},  // ^<-

	{0b00000000,
	 0b00001000,
	 0b00011000,
	 0b00111111,
	 0b01111111,
	 0b00111111,
	 0b00011000,
	 0b00001000,
	 0b00000000,
	 0b00000000},  // <-  complete
};

unsigned char fpga_set_full[10] = {
	// memset(array,0x7e,sizeof(array));
	0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f
};

unsigned char fpga_set_blank[10] = {
	// memset(array,0x00,sizeof(array));
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

#endif
