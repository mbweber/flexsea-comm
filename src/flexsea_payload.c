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
	[This file] flexsea_payload: deals with the "intelligent" data packaged
	in a comm_str
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2016-09-09 | jfduval | Initial GPL-3.0 release
	*
****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

//****************************************************************************
// Include(s)
//****************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../inc/flexsea.h"
#include "../../flexsea-system/inc/flexsea_system.h"
#include "flexsea_board.h"

//****************************************************************************
// Variable(s)
//****************************************************************************

uint8_t payload_str[PAYLOAD_BUF_LEN];

//****************************************************************************
// Private Function Prototype(s):
//****************************************************************************

static uint8_t get_rid(uint8_t *pldata);
static void route_to_slave(uint8_t port, uint8_t *buf, uint32_t len);

//****************************************************************************
// Public Function(s):
//****************************************************************************

//Decode/parse received string
//ToDo improve: for now, supports only one command per string
uint8_t payload_parse_str(uint8_t *cp_str, uint8_t *info)
{
	uint8_t cmd = 0, cmd_7bits = 0;
	unsigned int id = 0;
	uint8_t pType = RX_PTYPE_INVALID;

	//Command
	cmd = cp_str[P_CMD1];		//CMD w/ R/W bit
	cmd_7bits = CMD_7BITS(cmd);	//CMD code, no R/W information

	//First, get RID code
	id = get_rid(cp_str);
	if(id == ID_MATCH)
	{
		pType = packetType(cp_str);

		//It's addressed to me. Function pointer array will call
		//the appropriate handler (as defined in flexsea_system):
		if((cmd_7bits <= MAX_CMD_CODE) && (pType <= RX_PTYPE_MAX_INDEX))
		{
			(*flexsea_payload_ptr[cmd_7bits][pType]) (cp_str, info);

			return PARSE_SUCCESSFUL;
		}
		else
		{
			return PARSE_DEFAULT;
		}
	}
	else if(id == ID_SUB1_MATCH)
	{
		//For a slave on bus #1:
		route_to_slave(PORT_SUB1, cp_str, PAYLOAD_BUF_LEN);
		//ToDo compute length rather then sending the max
	}
	else if(id == ID_SUB2_MATCH)
	{
		//For a slave on bus #2:
		route_to_slave(PORT_SUB2, cp_str, PAYLOAD_BUF_LEN);
		//ToDo compute length rather then sending the max
	}
	else if(id == ID_UP_MATCH)
	{
		//For my master:

		#ifdef BOARD_TYPE_FLEXSEA_MANAGE

		uint8_t numb  = 0;

		//Manage is the only board that can receive a package destined to his master

		//Repackages the payload. ToDo: would be more efficient to just resend the comm_str, but it's not passed
		//to this function (cp_str is a payload, not a comm_str)
		numb = comm_gen_str(cp_str, comm_str_usb, PAYLOAD_BUF_LEN);		//ToDo: shouldn't be fixed at spi or usb
		numb = COMM_STR_BUF_LEN;    //Fixed length for now
		flexsea_send_serial_master(PORT_USB, comm_str_usb, numb);	//Same comment here - ToDo fix
		//(the SPI driver will grab comm_str_spi directly)

		#endif	//BOARD_TYPE_FLEXSEA_MANAGE
	}
	else
	{
		return PARSE_ID_NO_MATCH;
	}

	//Shouldn't get here...
	return PARSE_DEFAULT;
}

//Start a new payload string
void prepare_empty_payload(uint8_t from, uint8_t to, uint8_t *buf, uint32_t len)
{
	//Start fresh:
	memset(buf, 0, len);

	//Addresses:
	buf[P_XID] = from;
	buf[P_RID] = to;
}

//Returns one if it was sent from a slave, 0 otherwise
uint8_t sent_from_a_slave(uint8_t *buf)
{
	//Hint: slaves have higher addresses than their master
	return ((buf[P_XID] > buf[P_RID])? 1 : 0);
}

//We received a packet. Is it a Read, a Reply or a Write?
uint8_t packetType(uint8_t *buf)
{
	//Logic behind this code: slaves have higher addresses than their master.

	//From a Master:
	if(buf[P_XID] < buf[P_RID])
	{
		//Master is writing. Write or Read?
		return ((IS_CMD_RW(buf[P_CMD1]) == READ) ? \
					RX_PTYPE_READ : RX_PTYPE_WRITE);
	}

	//From a Slave:
	if(buf[P_XID] > buf[P_RID])
	{
		//The only thing we can get from a slave is a Reply
		return ((IS_CMD_RW(buf[P_CMD1]) == WRITE) ? \
					RX_PTYPE_REPLY : RX_PTYPE_INVALID);
	}

	//Equal addresses, shouldn't happen
	return RX_PTYPE_INVALID;
}

//****************************************************************************
// Private Function(s):
//****************************************************************************

//ToDo not the greatest function...
static void route_to_slave(uint8_t port, uint8_t *buf, uint32_t len)
{
	#ifdef BOARD_TYPE_FLEXSEA_MANAGE

		uint32_t numb = 0;
		uint8_t *comm_str_ptr = slaves_485_1.xmit.str;

		//Repackages the payload. ToDo: would be more efficient to just resend the comm_str,
		//but it's not passed to this function
		numb = comm_gen_str(buf, comm_str_tmp, len);
		//numb = COMM_STR_BUF_LEN;    //Fixed length for now

		//Port specific flags and buffer:
		if(port == PORT_RS485_1)
		{
			comm_str_ptr = slaves_485_1.xmit.str;
			slaves_485_1.xmit.cmd = buf[P_CMD1];
			slaves_485_1.xmit.flag = 1;
		}
		else if(port == PORT_RS485_2)
		{
			comm_str_ptr = slaves_485_2.xmit.str;
			slaves_485_2.xmit.cmd = buf[P_CMD1];
			slaves_485_2.xmit.flag = 1;
		}

		//Copy string:
		memcpy(comm_str_ptr, comm_str_tmp, numb+1);

	#else

		(void)port;
		(void)buf;
		(void) len;

	#endif 	//BOARD_TYPE_FLEXSEA_MANAGE
}

//Is it addressed to me? To a board "below" me? Or to my Master?
static uint8_t get_rid(uint8_t *pldata)
{
	uint8_t cp_rid = pldata[P_RID];
	uint8_t i = 0;

	if(cp_rid == board_id)				//This board?
	{
		return ID_MATCH;
	}
	else if(cp_rid == board_up_id)		//Master?
	{
		return ID_UP_MATCH;
	}
	else
	{
		//Can be on a slave bus, or can be invalid.

		//Search on slave bus #1:
		for(i = 0; i < SLAVE_BUS_1_CNT; i++)
		{
			if(cp_rid == board_sub1_id[i])
			{
				return ID_SUB1_MATCH;
			}
		}

		//Then on bus #2:
		for(i = 0; i < SLAVE_BUS_1_CNT; i++)
		{
			if(cp_rid == board_sub2_id[i])
			{
				return ID_SUB2_MATCH;
			}
		}
	}

	//If we end up here it's because we didn't get a match:
	return ID_NO_MATCH;
}

#ifdef __cplusplus
}
#endif
