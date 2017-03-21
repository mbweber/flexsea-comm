#ifdef __cplusplus
extern "C" {
#endif
#include <flexsea_buffers.h>
#include "flexsea-comm_test-all.h"
#include <stdio.h>

//Definitions and variables used by some/all tests:
//...

#define TEST_LEN	26
void test_update_rx_buf_byte_1(void)
{
	int i;
	uint8_t inputStr[TEST_LEN];
	for(i = 0; i < TEST_LEN; i++)
	{
		inputStr[i] = 'a'+i;
	}
	//printf("Test val: %s.\n", inputStr);
	//fflush(stdout);

	//Start with all 0
	memset(rx_buf_1, 0, RX_BUF_LEN);

	//Fill buffer:
	for(i = 0; i < TEST_LEN; i++)
	{
		update_rx_buf_byte_1(inputStr[i]);
	}

	TEST_ASSERT_EQUAL_STRING_LEN(inputStr, rx_buf_1, TEST_LEN);
	//printf("rx_buf_1: %s.\n", rx_buf_1);
	//fflush(stdout);

	//Now we keep filling it:

	//Fill buffer:
	for(i = 0; i < TEST_LEN; i++)
	{
		update_rx_buf_byte_1(inputStr[i]);
	}
	for(i = 0; i < TEST_LEN; i++)
	{
		update_rx_buf_byte_1(inputStr[i]);
	}
	for(i = 0; i < TEST_LEN; i++)
	{
		update_rx_buf_byte_1(inputStr[i]);
	}

	//printf("rx_buf_1: %s.\n", rx_buf_1);
	//fflush(stdout);

	//Final test:
	for(i = 0; i < RX_BUF_LEN - 1; i++)
	{
		if(rx_buf_1[i] != 'z')
			TEST_ASSERT_EQUAL_UINT8(rx_buf_1[i], rx_buf_1[i+1]-1);
		else
			TEST_ASSERT_EQUAL_UINT8('a', rx_buf_1[i+1]);
	}
}

void test_update_rx_buf_array_1(void)
{
	int i;
	uint8_t inputStr[TEST_LEN];
	for(i = 0; i < TEST_LEN; i++)
	{
		inputStr[i] = 'a'+i;
	}
	//printf("Test val: %s.\n", inputStr);
	//fflush(stdout);

	//Start with all 0
	memset(rx_buf_1, 0, RX_BUF_LEN);

	//Fill buffer:
	update_rx_buf_array_1(inputStr, TEST_LEN);
	//printf("rx_buf_1: %s.\n", rx_buf_1);
	//fflush(stdout);

	TEST_ASSERT_EQUAL_STRING_LEN_MESSAGE(inputStr, rx_buf_1, TEST_LEN, "First update");


	//Now we keep filling it:

	//Fill buffer:
	update_rx_buf_array_1(inputStr, TEST_LEN);
	//printf("rx_buf_1: %s.\n", rx_buf_1);
	//fflush(stdout);

	update_rx_buf_array_1(inputStr, TEST_LEN);
	//printf("rx_buf_1: %s.\n", rx_buf_1);
	//fflush(stdout);

	update_rx_buf_array_1(inputStr, TEST_LEN);
	//printf("rx_buf_1: %s.\n", rx_buf_1);
	//fflush(stdout);

	//Final test:
	for(i = 0; i < RX_BUF_LEN - 1; i++)
	{
		if(rx_buf_1[i] != 'z')
			TEST_ASSERT_EQUAL_UINT8(rx_buf_1[i], rx_buf_1[i+1]-1);
		else
			TEST_ASSERT_EQUAL_UINT8('a', rx_buf_1[i+1]);
	}
}

void test_buffer_stack(void)
{
	int i;

	//Start with all 0
	memset(rx_buf_1, 0, RX_BUF_LEN);

	for(i = 0; i < 100; i++)
	{
		update_rx_buf_byte_1(i);
	}

	for(i = 0; i < 50; i++)
	{
		update_rx_buf_byte_1(i);
	}

	TEST_ASSERT_EQUAL(50, rx_buf_1[0]);
	TEST_ASSERT_EQUAL(49, rx_buf_1[99]);
}

void test_buffer_circular(void)
{
	int i;

	circularBuffer_t cb;
	circ_buff_init(&cb);

	for(i = 0; i < RX_BUF_LEN; i++)
	{
		circ_buff_write(&cb, &i, 1);
	}

	for(i = 0; i < RX_BUF_LEN/2; i++)
	{
		circ_buff_write(&cb, &i, 1);
	}

	int expectedSize = RX_BUF_LEN;
	int actualSize = circ_buff_get_size(&cb);
	TEST_ASSERT_EQUAL(expectedSize, actualSize);

	uint8_t buf[RX_BUF_LEN];

	TEST_ASSERT_EQUAL(0, circ_buff_read(&cb, buf, RX_BUF_LEN));
	TEST_ASSERT_EQUAL(50, buf[0]);
	TEST_ASSERT_EQUAL(49, buf[RX_BUF_LEN-1]);
}

void test_flexsea_buffers(void)
{
	UNITY_BEGIN();
	//RUN_TEST(test_update_rx_buf_byte_1);
	RUN_TEST(test_update_rx_buf_array_1);
	RUN_TEST(test_buffer_stack);
	RUN_TEST(test_buffer_circular);
	UNITY_END();
}


#ifdef __cplusplus
}
#endif
