#ifdef __cplusplus
extern "C" {
#endif

#include "flexsea-comm_test-all.h"

//Definitions and variables used by some/all tests:
//...

void test_update_rx_buf_byte_1(void)
{
	//ToDo
}

void test_update_rx_buf_array_1(void)
{
	//ToDo
}

void test_buffer_stack(void)
{
	int i;

	//Start will all 0
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

	//Start will all 0
	memset(rx_buf_T, 0, RX_BUF_LEN);

	for(i = 0; i < 100; i++)
	{
		update_rx_buf_byte_T(i);
	}

	for(i = 0; i < 50; i++)
	{
		update_rx_buf_byte_T(i);
	}

	TEST_ASSERT_EQUAL(50, get_rx_buf_T(0));
	TEST_ASSERT_EQUAL(49, get_rx_buf_T(99));
}

void test_flexsea_buffers(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_buffer_stack);
	RUN_TEST(test_buffer_circular);
	UNITY_END();
}


#ifdef __cplusplus
}
#endif
