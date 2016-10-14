/****************************************************************************
	[Project] FlexSEA: Flexible & Scalable Electronics Architecture
	[Sub-project] 'flexsea-comm' Communication stack
	Copyright (C) 2016 Dephy, Inc. <http://dephy.com/>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************
	[Lead developper] Jean-Francois (JF) Duval, jfduval at dephy dot com.
	[Origin] Based on Jean-Francois Duval's work at the MIT Media Lab
	Biomechatronics research group <http://biomech.media.mit.edu/>
	[Contributors]
*****************************************************************************
	[This file] flexsea_comm: Data-Link layer of the FlexSEA protocol
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2016-09-09 | jfduval | Initial GPL-3.0 release
	*
****************************************************************************/

#ifndef INC_FX_COMM_H
#define INC_FX_COMM_H

#ifdef __cplusplus
extern "C" {
#endif

//****************************************************************************
// Include(s)
//****************************************************************************

#include "flexsea.h"
#include "flexsea_board.h"
#include "flexsea_system.h"

//****************************************************************************
// Shared variable(s)
//****************************************************************************

extern uint8_t comm_str_tmp[COMM_STR_BUF_LEN];

#ifdef ENABLE_FLEXSEA_BUF_1
extern uint8_t comm_str_1[COMM_STR_BUF_LEN];
extern uint8_t rx_command_1[PAYLOAD_BUFFERS][PACKAGED_PAYLOAD_LEN];
#endif	//ENABLE_FLEXSEA_BUF_1
#ifdef ENABLE_FLEXSEA_BUF_2
extern uint8_t comm_str_2[COMM_STR_BUF_LEN];
extern uint8_t rx_command_2[PAYLOAD_BUFFERS][PACKAGED_PAYLOAD_LEN];
#endif	//ENABLE_FLEXSEA_BUF_2
#ifdef ENABLE_FLEXSEA_BUF_3
extern uint8_t comm_str_3[COMM_STR_BUF_LEN];
extern uint8_t rx_command_3[PAYLOAD_BUFFERS][PACKAGED_PAYLOAD_LEN];
#endif	//ENABLE_FLEXSEA_BUF_3
#ifdef ENABLE_FLEXSEA_BUF_4
extern uint8_t comm_str_4[COMM_STR_BUF_LEN];
extern uint8_t rx_command_4[PAYLOAD_BUFFERS][PACKAGED_PAYLOAD_LEN];
#endif	//ENABLE_FLEXSEA_BUF_4

//ToDo: this is project specific! Eliminate or use generic names!
extern struct slave_comm_s slaves_485_1, slaves_485_2;

//****************************************************************************
// Public Function Prototype(s):
//****************************************************************************

uint8_t comm_gen_str(uint8_t payload[], uint8_t *cstr, uint8_t bytes);

#ifdef ENABLE_FLEXSEA_BUF_1
uint8_t unpack_payload_1(void);
#endif	//ENABLE_FLEXSEA_BUF_1
#ifdef ENABLE_FLEXSEA_BUF_2
uint8_t unpack_payload_2(void);
#endif	//ENABLE_FLEXSEA_BUF_2
#ifdef ENABLE_FLEXSEA_BUF_3
uint8_t unpack_payload_3(void);
#endif	//ENABLE_FLEXSEA_BUF_3
#ifdef ENABLE_FLEXSEA_BUF_4
uint8_t unpack_payload_4(void);
#endif	//ENABLE_FLEXSEA_BUF_4

//****************************************************************************
// Definition(s):
//****************************************************************************

//Framing:
#define HEADER  				0xED	//237d
#define FOOTER  				0xEE	//238d
#define ESCAPE  				0xE9	//233d

#define SC_TRANSPARENT			0
#define SC_AUTOSAMPLING			1

//Enable this to debug with the terminal:
//#define DEBUG_COMM_PRINTF_

//Conditional printf() statement:
#ifdef DEBUG_COMM_PRINTF_
	#define DEBUG_COMM_PRINTF(...) printf(__VA_ARGS__)
#else
	#define DEBUG_COMM_PRINTF(...) do {} while (0)
#endif	//DEBUG_COMM_PRINTF_

//****************************************************************************
// Structure(s):
//****************************************************************************

//ToDo: should this be here? Not sure this is used anymore... FIX

struct sc_data_s
{
	uint8_t flag;						//1 when new data ready to be transmitted
	uint8_t str[COMM_STR_BUF_LEN];		//Data to be transmitted
	uint8_t length;						//Number of bytes to be sent
	uint8_t cmd;						//What's the command? (used to know if we will get an answer)
	uint8_t listen;						//1 when we expect an answer
};

struct slave_comm_s
{
	uint8_t mode;						//SC_TRANSPARENT or SC_AUTOSAMPLING
	uint8_t port;						//PORT_RS485_1 or PORT_RS485_2
	uint8_t bytes_ready;
	uint8_t cmd_ready;

	//We use 2 structures to avoid confusion in the data if the master was to request
	//a Read while we are auto-sampling:

	struct sc_data_s xmit;				//For the Transparent mode
	struct sc_data_s autosample;		//For the Autosampling mode
};

#ifdef __cplusplus
}
#endif

#endif
