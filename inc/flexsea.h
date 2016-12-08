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
	[This file] flexsea: Master file for the FlexSEA stack.
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2016-09-09 | jfduval | Initial GPL-3.0 release
	*
****************************************************************************/

#ifndef INC_FLEXSEA_H_
#define INC_FLEXSEA_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

//#define USE_DEBUG_PRINTF			//Enable this to debug with the terminal

//****************************************************************************
// Prototype(s):
//****************************************************************************

unsigned int flexsea_error(unsigned int err_code);
void SPLIT_16(uint16_t var, uint8_t *buf, uint16_t *index);
uint16_t REBUILD_UINT16(uint8_t *buf, uint16_t *index);
void SPLIT_32(uint32_t var, uint8_t *buf, uint16_t *index);
uint32_t REBUILD_UINT32(uint8_t *buf, uint16_t *index);

void fill_uint8_buf(uint8_t *buf, uint32_t len, uint8_t filler);

//****************************************************************************
// Definition(s):
//****************************************************************************

//Buffers and packets:
#define RX_BUF_LEN						100		//Reception buffer (flexsea_comm)
#define PAYLOAD_BUF_LEN					36		//Number of bytes in a payload string
#define PAYLOAD_BYTES					(PAYLOAD_BUF_LEN - 4)
#define COMM_STR_BUF_LEN				48		//Number of bytes in a comm. string
#define PACKAGED_PAYLOAD_LEN			48		//Temporary
#define PAYLOAD_BUFFERS					4		//Max # of payload strings we expect to find
#define MAX_CMD_CODE					127
//ToDo: Should be in 'system'

//Packet types:
#define RX_PTYPE_READ					0
#define RX_PTYPE_WRITE					1
#define RX_PTYPE_REPLY					2
#define RX_PTYPE_INVALID				3
#define RX_PTYPE_MAX_INDEX				2

//Board ID related defines:
#define ID_MATCH						1		//Addressed to me
#define ID_SUB1_MATCH					2		//Addressed to a board on slave bus #1
#define ID_SUB2_MATCH					3		//Addressed to a board on slave bus #2
#define ID_UP_MATCH						4		//Addressed to my master
#define ID_NO_MATCH						0

//Communication ports:
#define PORT_485_1						0
#define PORT_485_2						1
#define PORT_SPI						2
#define PORT_USB						3
#define PORT_SUB1						PORT_485_1
#define PORT_SUB2						PORT_485_2

//Communication protocol payload fields:
#define P_XID							0		//Emitter ID
#define P_RID							1		//Receiver ID
#define P_CMDS							2		//Number of Commands sent
#define P_CMD1							3		//First command
#define P_DATA1							4		//First data

//Parser definitions:
#define PARSE_DEFAULT					0
#define PARSE_ID_NO_MATCH				1
#define PARSE_SUCCESSFUL				2
#define PARSE_UNKNOWN_CMD				3

#define CMD_READ						1
#define CMD_WRITE						2

#define KEEP							0
#define CHANGE							1

//****************************************************************************
// Shared variable(s)
//****************************************************************************

//Function pointer array:
extern void (*flexsea_payload_ptr[MAX_CMD_CODE][RX_PTYPE_MAX_INDEX+1]) \
				(uint8_t *buf, uint8_t *info);

//****************************************************************************
// Macro(s):
//****************************************************************************

//Min and Max
#ifndef MIN
	#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#endif
#ifndef MAX
	#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#endif

//Reassembles a uint32 from 4 bytes. b0 is the MSB.
#define BYTES_TO_UINT32(b0,b1,b2,b3)	(((uint32_t)b0 << 24) + \
										((uint32_t)b1 << 16) + \
										((uint32_t)b2 << 8) + \
										((uint32_t)b3))

//Reassembles a uint16 from 2 bytes. b0 is the MSB.
#define BYTES_TO_UINT16(b0,b1)			(((uint16_t)b0 << 8) + \
										((uint16_t)b1))

//Macros to deal with the 7 bits addresses and the R/W bit
#define CMD_W(x)		(x << 1)			//LSB = 0
#define CMD_R(x)		((x << 1) | 1)		//LSB = 1
#define CMD_7BITS(x)	((x & 0xFF)>>1)
#define IS_CMD_RW(x)	(x & 0x01)

//Read, Write, or Read&Write?
#define WRITE			0
#define READ			1

//Conditional printf() statement - debugging only
#ifdef USE_DEBUG_PRINTF
	#define DEBUG_PRINTF(...) printf(__VA_ARGS__)
#else
	#define DEBUG_PRINTF(...) do {} while (0)
#endif	//USE_DEBUG_PRINTF

//Conditional printf() statement - use this one for Plan Console
#ifdef USE_PRINTF
	#define _USE_PRINTF(...) printf(__VA_ARGS__)
#else
	#define _USE_PRINTF(...) do {} while (0)
#endif	//USE__PRINTF

//****************************************************************************
// Include(s) - at the end to make sure that the included files can access
// all the project wide #define.
//****************************************************************************

//All the FlexSEA stack includes:

#include "flexsea_buffers.h"
#include "flexsea_comm.h"
#include "flexsea_payload.h"

#ifdef __cplusplus
}
#endif

#endif
