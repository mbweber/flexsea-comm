#ifdef __cplusplus
extern "C" {
#endif
#include <flexsea_buffers.h>
#include "flexsea-comm_test-all.h"
#include <stdio.h>

#include <time.h>
#include <stdlib.h>

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
    uint8_t buf[RX_BUF_LEN];

	for(i = 0; i < RX_BUF_LEN; i++)
	{
		circ_buff_write(&cb, &i, 1);
	}

    circ_buff_read(&cb, buf, RX_BUF_LEN);

    int fail = 0;
    for(i = 0; i < RX_BUF_LEN; i++)
    {
        fail = i != buf[i];
        if(fail) break;
    }
    TEST_ASSERT_FALSE_MESSAGE(fail, "READ/WRITE fail");

	for(i = 0; i < RX_BUF_LEN/2; i++)
	{
		circ_buff_write(&cb, &i, 1);
	}

	int expectedSize = RX_BUF_LEN;
	int actualSize = circ_buff_get_size(&cb);
	TEST_ASSERT_EQUAL(expectedSize, actualSize);


	TEST_ASSERT_EQUAL(0, circ_buff_read(&cb, buf, RX_BUF_LEN));
    int firstVal = RX_BUF_LEN/2;
    int lastVal = i-1;
    TEST_ASSERT_EQUAL(firstVal, buf[0]);
    TEST_ASSERT_EQUAL(lastVal, buf[RX_BUF_LEN-1]);

	circ_buff_init(&cb);
	//random tests
	srand(time(NULL));
	int lastSize = 0, lastHead = -1, lastTail = 0, firstTime = 1;
	expectedSize = 0;
	int expectedHead = -1, expectedTail = 0;
	for(i = 0; i < 5000; i++)
	{
		int shouldWrite = rand() % 2;
		int l = rand() % (RX_BUF_LEN + 10);
		int result;
		if(shouldWrite)
		{
			result = circ_buff_write(&cb, buf, l);
			if(l > RX_BUF_LEN)
			{ //expect
				TEST_ASSERT_EQUAL_MESSAGE(1, result, "Expected to fail on write of size larger than buffer");
			}
			else
			{
				expectedSize += l;
				expectedTail = (expectedTail + l) % RX_BUF_LEN;
				if(expectedSize > RX_BUF_LEN)
				{
					expectedHead = expectedTail;
					expectedSize = RX_BUF_LEN;
					TEST_ASSERT_EQUAL_MESSAGE(2, result, "Expected overwrite");
				}
				else
				{
					expectedHead = expectedHead < 0 ? 0 : expectedHead;
					if(result != 0)
					{
						printf("ASdf");
					}
					TEST_ASSERT_EQUAL_MESSAGE(0, result, "Expected success");
				}
			}
		}
		else
		{
			result = circ_buff_move_head(&cb, l);
			if(l > RX_BUF_LEN)
			{ //expect to fail

				TEST_ASSERT_EQUAL_MESSAGE(1, result, "Expected to over reset buffer");
				expectedHead = -1;
				expectedTail = 0;
				expectedSize = 0;
			}
			else
			{
				TEST_ASSERT_EQUAL_MESSAGE(0, result, "Expected success");
				expectedSize -= l;
				if(expectedSize < 1)
				{
					expectedHead = -1;
					expectedTail = 0;
					expectedSize = 0;

					if(expectedHead != cb.head || expectedTail != cb.tail || expectedSize != cb.size)
					{
						printf("AsdfSD");
					}
				}
				else
				{
					expectedHead = (expectedHead + l) % RX_BUF_LEN;
				}
			}
		}



		if(expectedHead != cb.head || expectedTail != cb.tail || expectedSize != cb.size)
		{
			printf("AsdfSD");
		}


		TEST_ASSERT_EQUAL_MESSAGE(expectedHead, cb.head, "Heads did not match");
		TEST_ASSERT_EQUAL_MESSAGE(expectedTail, cb.tail, "Tails did not match");
		TEST_ASSERT_EQUAL_MESSAGE(expectedSize, cb.size, "Sizes did not match");
		lastSize = expectedSize;
		lastHead = expectedHead;
		lastTail = expectedTail;
		firstTime = 0;
	}
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
