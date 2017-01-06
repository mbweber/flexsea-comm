#ifdef __cplusplus
extern "C" {
#endif

#include "../inc/flexsea.h"
#include "flexsea-comm_test-all.h"

//Definitions and variables used by some/all tests:
uint8_t fakePayload[PAYLOAD_BUF_LEN];
uint8_t fakeCommStr[COMM_STR_BUF_LEN];
uint8_t retVal = 0;

void resetCommStats(void)
{
	//All stats to 0:
	commSpy1.bytes = 0;
	commSpy1.total_bytes = 0;
	commSpy1.checksum = 0;
	commSpy1.retVal = 0;
	commSpy1.escapes = 0;
}

void test_comm_gen_str_simple(void)
{
	int i = 0;
	uint8_t checksum = 0;

	//Empty strings:
	memset(fakePayload, 0, PAYLOAD_BUF_LEN);
	memset(fakeCommStr, 0, COMM_STR_BUF_LEN);

	//We build a fake "Read All" command:
	fakePayload[P_XID] = FLEXSEA_PLAN_1;
	fakePayload[P_RID] = FLEXSEA_MANAGE_1;
	fakePayload[P_CMDS] = 1;
	fakePayload[P_DATA1] = CMD_R(CMD_READ_ALL);

	resetCommStats();

	//Gen comm str:
	retVal = comm_gen_str(fakePayload, fakeCommStr, 4);

	//Recalculate the checksum here:
	for(i = 0; i < commSpy1.total_bytes; i++)
	{
		checksum += fakeCommStr[2+i];
	}

	//Tests:

	TEST_ASSERT_EQUAL_MESSAGE(commSpy1.retVal, retVal, "retval");
	TEST_ASSERT_EQUAL(7, retVal);
	TEST_ASSERT_EQUAL_MESSAGE(commSpy1.checksum, checksum, "checksum");

	for(i = retVal+1; i < COMM_STR_BUF_LEN; i++)
	{
		TEST_ASSERT_EQUAL_MESSAGE(0xAA, fakeCommStr[i], "filler");
	}
}

//Too long because I'm feeding it 48 bytes:
void test_comm_gen_str_tooLong1(void)
{
	int i = 0;
	uint8_t checksum = 0;

	//Empty strings:
	memset(fakePayload, 0, PAYLOAD_BUF_LEN);
	memset(fakeCommStr, 0, COMM_STR_BUF_LEN);

	//We build a fake "Read All" command:
	fakePayload[P_XID] = FLEXSEA_PLAN_1;
	fakePayload[P_RID] = FLEXSEA_MANAGE_1;
	fakePayload[P_CMDS] = 1;
	fakePayload[P_DATA1] = CMD_R(CMD_READ_ALL);

	resetCommStats();

	//Gen comm str, with 48 bytes:
	retVal = comm_gen_str(fakePayload, fakeCommStr, 48);

	//Recalculate the checksum here:
	for(i = 0; i < commSpy1.total_bytes; i++)
	{
		checksum += fakeCommStr[2+i];
	}

	//Tests:

	TEST_ASSERT_EQUAL_MESSAGE(commSpy1.retVal, retVal, "retval");
	TEST_ASSERT_EQUAL(0, retVal);

	for(i = 0; i < COMM_STR_BUF_LEN; i++)
	{
		TEST_ASSERT_EQUAL_MESSAGE(0, fakeCommStr[i], "filler");
	}
}

//Too long because I'm feeding it chars that need escaping:
void test_comm_gen_str_tooLong2(void)
{
	int i = 0;
	uint8_t checksum = 0;

	//Empty strings:
	memset(fakePayload, 0, PAYLOAD_BUF_LEN);
	memset(fakeCommStr, 0, COMM_STR_BUF_LEN);

	//We build a fake "Read All" command:
	fakePayload[P_XID] = FLEXSEA_PLAN_1;
	fakePayload[P_RID] = FLEXSEA_MANAGE_1;
	fakePayload[P_CMDS] = 1;
	fakePayload[P_DATA1] = CMD_R(CMD_READ_ALL);

	//24 bytes that will need escaping:
	for(i = 0; i < 24; i++)
	{
		fakePayload[P_DATA1+i] = HEADER;
	}

	resetCommStats();

	//Gen comm str, with 48 bytes:
	retVal = comm_gen_str(fakePayload, fakeCommStr, 28);

	//Recalculate the checksum here:
	for(i = 0; i < commSpy1.total_bytes; i++)
	{
		checksum += fakeCommStr[2+i];
	}

	//Tests:

	TEST_ASSERT_EQUAL_MESSAGE(commSpy1.retVal, retVal, "retval");
	TEST_ASSERT_EQUAL(0, retVal);
	TEST_ASSERT_EQUAL_MESSAGE(24, commSpy1.escapes, "Escape characters");

	for(i = 0; i < COMM_STR_BUF_LEN; i++)
	{
		TEST_ASSERT_EQUAL_MESSAGE(0, fakeCommStr[i], "filler");
	}
}

void test_flexsea_comm(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_comm_gen_str_simple);
	RUN_TEST(test_comm_gen_str_tooLong1);
	RUN_TEST(test_comm_gen_str_tooLong2);
	UNITY_END();
}

#ifdef __cplusplus
}
#endif
