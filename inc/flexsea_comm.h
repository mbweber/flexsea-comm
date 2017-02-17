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
#include "flexsea_buffers.h"
#include "flexsea_system.h"
#include <flexsea_board.h>
#include  <fm_block_allocator.h>

//****************************************************************************
// Public Function Prototype(s):
//****************************************************************************

uint8_t comm_gen_str(uint8_t payload[], uint8_t *cstr, uint8_t bytes);

#ifdef ENABLE_FLEXSEA_BUF_1
int8_t unpack_payload_1(void);
#endif	//ENABLE_FLEXSEA_BUF_1
#ifdef ENABLE_FLEXSEA_BUF_2
int8_t unpack_payload_2(void);
#endif	//ENABLE_FLEXSEA_BUF_2
#ifdef ENABLE_FLEXSEA_BUF_3
int8_t unpack_payload_3(void);
#endif	//ENABLE_FLEXSEA_BUF_3
#ifdef ENABLE_FLEXSEA_BUF_4
int8_t unpack_payload_4(void);
#endif	//ENABLE_FLEXSEA_BUF_4
int8_t unpack_payload_test(uint8_t *buf, uint8_t rx_cmd[PACKAGED_PAYLOAD_LEN]);

//Random numbers and arrays:
void initRandomGenerator(int seed);
uint8_t generateRandomUint8(void);
void generateRandomUint8Array(uint8_t *arr, uint8_t size);

//****************************************************************************
// Definition(s):
//****************************************************************************

//Framing:
#define HEADER  				0xED	//237d
#define FOOTER  				0xEE	//238d
#define ESCAPE  				0xE9	//233d

//Return codes:
#define UNPACK_ERR_HEADER		-1
#define UNPACK_ERR_FOOTER		-2
#define UNPACK_ERR_LEN			-3
#define UNPACK_ERR_CHECKSUM		-4

//Generic transceiver state:
#define TRANS_STATE_UNKNOWN		0
#define TRANS_STATE_TX			1
#define TRANS_STATE_TX_THEN_RX	2
#define TRANS_STATE_PREP_RX		3
#define TRANS_STATE_RX			4
//Pure write: stays TRANS_STATE_TX
//Read: TRANS_STATE_TX_THEN_RX => TRANS_STATE_PREP_RX => TRANS_STATE_RX

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

struct commSpy_s
{
	uint8_t counter;
	uint8_t bytes;
	uint8_t total_bytes;
	uint8_t escapes;
	uint8_t checksum;
	uint8_t retVal;
	uint8_t error;
};

struct comm_rx_s
{
	int8_t cmdReady;
	uint16_t bytesReady;

	//Pointers to buffers:
	uint8_t *rxBuf;
	uint8_t *commStr;
	uint8_t *rxCmd;
};

struct comm_tx_s
{
	//ToDo: this is a copy of what I had before. I'm expecting that it will
	//be reworked soon

	uint8_t txBuf[COMM_STR_BUF_LEN];
	uint8_t cmd;
	uint8_t len;
	uint8_t inject;

};

struct comm_s
{
	uint8_t port;
	uint8_t reply_port;
	int8_t transceiverState;

	//Reception:
	struct comm_rx_s rx;

	//Transmission:
	struct comm_tx_s tx;
};


//****************************************************************************
// Shared variable(s)
//****************************************************************************

extern uint8_t comm_str_tmp[COMM_STR_BUF_LEN];

#ifdef ENABLE_FLEXSEA_BUF_1
extern uint8_t comm_str_1[COMM_STR_BUF_LEN];
extern uint8_t rx_command_1[PACKAGED_PAYLOAD_LEN];
#endif	//ENABLE_FLEXSEA_BUF_1

#ifdef ENABLE_FLEXSEA_BUF_2
extern uint8_t comm_str_2[COMM_STR_BUF_LEN];
extern uint8_t rx_command_2[PACKAGED_PAYLOAD_LEN];
#endif	//ENABLE_FLEXSEA_BUF_2

#ifdef ENABLE_FLEXSEA_BUF_3
extern uint8_t comm_str_3[COMM_STR_BUF_LEN];
extern uint8_t rx_command_3[PACKAGED_PAYLOAD_LEN];
#endif	//ENABLE_FLEXSEA_BUF_3

#ifdef ENABLE_FLEXSEA_BUF_4
extern uint8_t comm_str_4[COMM_STR_BUF_LEN];
extern uint8_t rx_command_4[PACKAGED_PAYLOAD_LEN];
#endif	//ENABLE_FLEXSEA_BUF_4

#ifdef ENABLE_FLEXSEA_BUF_5
extern uint8_t comm_str_5[COMM_STR_BUF_LEN];
extern uint8_t rx_command_5[PACKAGED_PAYLOAD_LEN];
#endif	//ENABLE_FLEXSEA_BUF_5

extern struct comm_s slaveComm[COMM_SLAVE_BUS];
extern struct comm_s masterComm[COMM_MASTERS];

extern struct commSpy_s commSpy1;

extern MsgQueue unpacked_packet_queue;

//Overload buffer & function names (for user convenience):

#ifdef BOARD_TYPE_FLEXSEA_EXECUTE

#define comm_str_485_1 					comm_str_1
#define unpack_payload_485 				unpack_payload_1
#define rx_command_485 					rx_command_1
#define update_rx_buf_byte_485 			update_rx_buf_byte_1
#define update_rx_buf_array_485 		update_rx_buf_array_1

#define comm_str_usb 					comm_str_2
#define unpack_payload_usb			 	unpack_payload_2
#define rx_command_usb 					rx_command_2
#define update_rx_buf_byte_usb 			update_rx_buf_byte_2
#define update_rx_buf_array_usb 		update_rx_buf_array_2

#define comm_str_wireless				comm_str_3
#define unpack_payload_wireless			unpack_payload_3
#define rx_command_wireless				rx_command_3
#define update_rx_buf_byte_wireless		update_rx_buf_byte_3
#define update_rx_buf_array_wireless	update_rx_buf_array_3

#endif
	
#ifdef __cplusplus
}
#endif

#endif	//INC_FX_COMM_H
