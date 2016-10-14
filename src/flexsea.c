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
	* 2016-10-14 | jfduval | New macros & functions to simplify RX/TX fcts
****************************************************************************/

// FlexSEA: Flexible & Scalable Electronics Architecture

// v0.0 Limitations and known bugs:
// ================================
// - The board config is pretty much fixed, at compile time.
// - Only 1 command per transmission
// - Fixed payload length: ? bytes (allows you to send 1 command with up to
//   ? arguments (uint8) (update this)
// - Fixed comm_str length: 24 bytes (min. to accomodate a payload where all the
//   data bytes need escaping)
// - In comm_str #OfBytes isn't escaped. Ok as long as the count is less than
//   the decimal value of the flags ('a', 'z', 'e') so max 97 bytes.
// - Data transfer could be faster with shorter ACK sequence, more than 1 command
//   per packet and no repackaging on the Manage board (straight pass-through)
//   To be optimized later.

#ifdef __cplusplus
extern "C" {
#endif

//****************************************************************************
// Include(s)
//****************************************************************************

#include "../inc/flexsea.h"
#include "flexsea_system.h"
#include "flexsea_board.h"

//****************************************************************************
// Variable(s)
//****************************************************************************

unsigned char test_payload[PAYLOAD_BUF_LEN];

#ifdef ENABLE_COMM_MANUAL_TEST_FCT
int16_t test_comm_val2_1 = 0, test_comm_val2_2 = 0;
uint8_t test_comm_mod_1 = 0, test_comm_mod_2 = 0;
uint32_t packet_received_1 = 0, packet_received_2 = 0;
#endif	#ifdef ENABLE_COMM_MANUAL_TEST_FCT

//Function pointer array:
void (*flexsea_payload_ptr[MAX_CMD_CODE]) (uint8_t *buf);

//****************************************************************************
// Private Function Prototype(s)
//****************************************************************************

static void clear_rx_command(uint8_t x, uint8_t y, uint8_t rx_cmd[][PACKAGED_PAYLOAD_LEN]);

//****************************************************************************
// Public Function(s)
//****************************************************************************

//When something goes wrong in the code it will land here:
unsigned int flexsea_error(unsigned int err_code)
{
	//ToDo something useful
	return err_code;
}

//From 1 uint32 to 4 uint8
//TODO: being replaced by SPLIT_32(), eliminate once the transition is complete
void uint32_to_bytes(uint32_t x, uint8_t *b0, uint8_t *b1, uint8_t *b2, uint8_t *b3)
{
	*b0 = (uint8_t) ((x >> 24) & 0xFF);
	*b1 = (uint8_t) ((x >> 16) & 0xFF);
	*b2 = (uint8_t) ((x >> 8) & 0xFF);
	*b3 = (uint8_t) (x & 0xFF);
}

//From 1 uint16 to 2 uint8
//TODO: being replaced by SPLIT_16(), eliminate once the transition is complete
void uint16_to_bytes(uint32_t x, uint8_t *b0, uint8_t *b1)
{
	*b0 = (uint8_t) ((x >> 8) & 0xFF);
	*b1 = (uint8_t) (x & 0xFF);
}

//Splits 1 uint16 in 2 bytes, stores them in buf[index] and increments index
inline void SPLIT_16(uint16_t var, uint8_t *buf, uint16_t *index)
{
	buf[*index] = (uint8_t) ((var >> 8) & 0xFF);
	buf[(*index)+1] = (uint8_t) (var & 0xFF);
	(*index) += 2;
}

//Inverse of SPLIT_16()
uint16_t REBUILD_UINT16(uint8_t *buf, uint16_t *index)
{
	uint16_t tmp = 0;

	tmp = (((uint16_t)buf[(*index)] << 8) + ((uint16_t)buf[(*index)+1] ));
	(*index) += 2;
	return tmp;
}

//Splits 1 uint32 in 4 bytes, stores them in buf[index] and increments index
inline void SPLIT_32(uint32_t var, uint8_t *buf, uint16_t *index)
{
	buf[(*index)] = (uint8_t) ((var >> 24) & 0xFF);
	buf[(*index)+1] = (uint8_t) ((var >> 16) & 0xFF);
	buf[(*index)+2] = (uint8_t) ((var >> 8) & 0xFF);
	buf[(*index)+3] = (uint8_t) (var & 0xFF);
	(*index) += 4;
}

//Inverse of SPLIT_32()
uint32_t REBUILD_UINT32(uint8_t *buf, uint16_t *index)
{
	uint32_t tmp = 0;

	tmp = (((uint32_t)buf[(*index)] << 24) + ((uint32_t)buf[(*index)+1] << 16) \
			+ ((uint32_t)buf[(*index)+2] << 8) + ((uint32_t)buf[(*index)+3]));
	(*index) += 4;
	return tmp;
}

//Can be used to fill a buffer of any length with any value
void fill_uint8_buf(uint8_t *buf, uint32_t len, uint8_t filler)
{
	uint32_t i = 0;

	for(i = 0; i < len; i++)
	{
		buf[i] = filler;
	}
}

//****************************************************************************
// Private Function(s)
//****************************************************************************

//Empties the buffer - used by the test function
static void clear_rx_command(uint8_t x, uint8_t y, uint8_t rx_cmd[][PACKAGED_PAYLOAD_LEN])
{
	unsigned char i = 0, j = 0;

	for(i = 0; i < x; i++)
	{
		for(j = 0; j < y; j++)
		{
			rx_cmd[i][j] = 0;
		}
	}
}

#ifdef __cplusplus
}
#endif

//****************************************************************************
// Manual Test Function(s)
//****************************************************************************

#ifdef ENABLE_COMM_MANUAL_TEST_FCT

void test_SPLIT_REBUILD(void)
{
	uint8_t buffer[24];
	uint16_t index = 0;
	uint8_t myVal8[5] = {0,0,0,0,0};
	uint16_t myVal16[5] = {0,0,0,0,0};
	uint32_t myVal32[5] = {0,0,0,0,0};

	//Encoding:
	buffer[index++] = 0x11;
	buffer[index++] = 0x22;
	SPLIT_32(0x33445566, buffer, &index);
	buffer[index++] = 0x77;
	SPLIT_16(0x8899, buffer, &index);
	SPLIT_16(0xAABB, buffer, &index);
	SPLIT_16(0xCCDD, buffer, &index);
	SPLIT_16(0xDDEE, buffer, &index);

	//Decoding
	index = 0;
	myVal8[0] = buffer[index++];
	myVal8[1] = buffer[index++];
	myVal32[0] = REBUILD_UINT32(buffer, &index);
	myVal8[2] = buffer[index++];
	myVal16[0] = REBUILD_UINT16(buffer, &index);
	myVal16[1] = REBUILD_UINT16(buffer, &index);
	myVal16[2] = REBUILD_UINT16(buffer, &index);
	myVal16[3] = REBUILD_UINT16(buffer, &index);
}

/*
 * TODO: testing a fake command isn't very useful, what I need is to test the
 * real ones. This code will likely be removed soon, and automated tests will
 * do what it did.
 *
//Quick way to debug the comm functions with the debugger and the terminal.
//Make sure to enable the printf statements.
void test_flexsea_stack(void)
{
	uint8_t i = 0, bytes = 0;
	uint8_t res = 0;

	//We are using a command that Plan can receive to test the parser too:
	bytes = tx_cmd_test(FLEXSEA_PLAN_1, CMD_WRITE, test_payload, PAYLOAD_BUF_LEN, 100, 200);
	//(this fills payload_str[])

	DEBUG_PRINTF("bytes = %i\n", bytes);

	//Clear current payload:
	clear_rx_command(PAYLOAD_BUFFERS, PACKAGED_PAYLOAD_LEN, rx_command_1);

	//Build comm_str
	res = comm_gen_str(test_payload, comm_str_1, bytes);

	DEBUG_PRINTF("comm_str[]: >> %s <<\n", (char*)comm_str_spi);
	DEBUG_PRINTF("res = %i\n", res);

	DEBUG_PRINTF("\nrx_buf_spi[]: >> %s <<\n", (char*)rx_buf_spi);

	//Feed it to the input buffer
	for(i = 0; i < PACKAGED_PAYLOAD_LEN; i++)
	{
		update_rx_buf_byte_1(comm_str_1[i]);
	}

	DEBUG_PRINTF("rx_buf_spi[]: >> %s <<\n", (char*)rx_buf_spi);

	//Try to decode
	res = unpack_payload_1();

	DEBUG_PRINTF("Found %i payload(s).\n", res);

	//Can we parse it?

	res = payload_parse_str(rx_command_1[0]);

	//If it works, the console/terminal should display
	//"Received CMD_TEST. Val1 = 200, Val2 = 100."
}

//Transmission of a TEST command
uint32_t tx_cmd_test(uint8_t receiver, uint8_t cmd_type, uint8_t *buf, uint32_t len, int16_t val1, int16_t val2)
{
	uint8_t tmp0 = 0, tmp1 = 0;
	uint32_t bytes = 0;

	//Fresh payload string:
	prepare_empty_payload(board_id, receiver, buf, len);

	//Command:
	buf[P_CMDS] = 1;                     //1 command in string

	if(cmd_type == CMD_READ)
	{
		buf[P_CMD1] = CMD_R(CMD_TEST);

		//Arguments - user defined:
		uint16_to_bytes(val1, &tmp0, &tmp1);
		buf[P_DATA1] = tmp0;
		buf[P_DATA1 + 1] = tmp1;
		uint16_to_bytes(val2, &tmp0, &tmp1);
		buf[P_DATA1 + 2] = tmp0;
		buf[P_DATA1 + 3] = tmp1;

		//Arguments - fixed:
		buf[P_DATA1 + 4] = 50;
		buf[P_DATA1 + 5] = 100;
		buf[P_DATA1 + 6] = 150;
		buf[P_DATA1 + 7] = 200;
		buf[P_DATA1 + 8] = 250;
		buf[P_DATA1 + 9] = 255;
		buf[P_DATA1 + 10] = 0;
		buf[P_DATA1 + 11] = 0;

		//Argument - modulo:
		buf[P_DATA1 + 12] = ((buf[P_DATA1] + buf[P_DATA1 + 1] + buf[P_DATA1 + 4] + buf[P_DATA1 + 6]) % 256);

		bytes = P_DATA1 + 13;     //Bytes is always last+1

	}
	else if(cmd_type == CMD_WRITE)
	{
		buf[P_CMD1] = CMD_W(CMD_TEST);

		//Arguments - user defined:
		uint16_to_bytes(val1, &tmp0, &tmp1);
		buf[P_DATA1] = tmp0;
		buf[P_DATA1 + 1] = tmp1;
		uint16_to_bytes(val2, &tmp0, &tmp1);
		buf[P_DATA1 + 2] = tmp0;
		buf[P_DATA1 + 3] = tmp1;

		//Arguments - fixed:
		buf[P_DATA1 + 4] = 50;
		buf[P_DATA1 + 5] = 100;
		buf[P_DATA1 + 6] = 150;
		buf[P_DATA1 + 7] = 200;
		buf[P_DATA1 + 8] = 250;
		buf[P_DATA1 + 9] = 255;
		buf[P_DATA1 + 10] = 0;
		buf[P_DATA1 + 11] = 0;

		//Argument - modulo:
		buf[P_DATA1 + 12] = ((buf[P_DATA1] + buf[P_DATA1 + 1] + buf[P_DATA1 + 4] + buf[P_DATA1 + 6]) % 256);

		bytes = P_DATA1 + 13;     //Bytes is always last+1
	}
	else
	{
		//Invalid
		flexsea_error(SE_INVALID_READ_TYPE);
		bytes = 0;
	}

	return bytes;
}

//Reception of a TEST command
void rx_cmd_test(uint8_t *buf)
{
	uint32_t numb  = 0;
	int16_t tmp_val1 = 0, tmp_val2 = 0;
	uint8_t mod = 0;

	if(IS_CMD_RW(buf[P_CMD1]) == READ)
	{
		//Received a Read command from our master, prepare a reply:

		//Decode user data:
		tmp_val1 = (int16_t) (BYTES_TO_UINT16(buf[P_DATA1], buf[P_DATA1+1]));
		tmp_val2 = (int16_t) (BYTES_TO_UINT16(buf[P_DATA1+2], buf[P_DATA1+3]));

		//Modulo
		mod = ((buf[P_DATA1] + buf[P_DATA1 + 1] + buf[P_DATA1 + 4] + buf[P_DATA1 + 6]) % 256);
		if(mod == buf[P_DATA1 + 12])
		{
			//We received a valid command, almost certain it has no errors

			#ifdef BOARD_TYPE_FLEXSEA_EXECUTE

			numb = tx_cmd_test(buf[P_XID], CMD_WRITE, test_payload, \
								PAYLOAD_BUF_LEN, mod, tmp_val2);
			numb = comm_gen_str(test_payload, comm_str_485_1, numb);
			numb = COMM_STR_BUF_LEN;
			rs485_reply_ready(comm_str_485_1, numb);

			#endif //BOARD_TYPE_FLEXSEA_EXECUTE

		}
		else
		{
			//Modulo didn't match... send a constant, 0xAA

			#ifdef BOARD_TYPE_FLEXSEA_EXECUTE

			numb = tx_cmd_test(buf[P_XID], CMD_WRITE, test_payload, \
								PAYLOAD_BUF_LEN, 123, 234);
			numb = comm_gen_str(test_payload, comm_str_485_1, numb);
			numb = COMM_STR_BUF_LEN;
			rs485_reply_ready(comm_str_485_1, numb);

			#endif //BOARD_TYPE_FLEXSEA_EXECUTE

		}

	}
	else if(IS_CMD_RW(buf[P_CMD1]) == WRITE)
	{
		//Two options: from Master of from slave (a read reply)

		//Decode data:
		tmp_val1 = (int16_t) (BYTES_TO_UINT16(buf[P_DATA1], buf[P_DATA1+1]));
		tmp_val2 = (int16_t) (BYTES_TO_UINT16(buf[P_DATA1+2], buf[P_DATA1+3]));
		//ToDo store that value somewhere useful

		if(sent_from_a_slave(buf))
		{
			//We received a reply to our read request

			if(buf[P_XID] == FLEXSEA_EXECUTE_1)
			{
				//Increment received packet counter:
				packet_received_1++;

				//Store values
				test_comm_mod_1 = buf[P_DATA1];
				test_comm_val2_1 = tmp_val2;
			}
			else if(buf[P_XID] == FLEXSEA_EXECUTE_2)
			{
				//Increment received packet counter:
				packet_received_2++;

				//Store values
				test_comm_mod_2 = buf[P_DATA1];
				test_comm_val2_2 = tmp_val2;
			}

			//Store the reply:

			DEBUG_PRINTF("Received CMD_TEST_REPLY. Val1 = %i, Val2 = %i.\n", tmp_val1, tmp_val2);
		}
		else
		{
			//Master is writing a value to this board

			DEBUG_PRINTF("Received CMD_TEST. Val1 = %i, Val2 = %i.\n", tmp_val1, tmp_val2);
		}
	}
}
*/

#endif ENABLE_COMM_MANUAL_TEST_FCT
