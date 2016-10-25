#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "unity.h"
#include "flexsea-comm_test-all.h"

//****************************************************************************
// Variables used for these sets of tests:
//****************************************************************************

//****************************************************************************
// Helper function(s):
//****************************************************************************


//****************************************************************************
// Main test function:
//****************************************************************************

//Call this function to test the 'flexsea-comm' stack:
int flexsea_comm_test(void)
{
	UNITY_BEGIN();

	//One call per file here:
	test_flexsea();
	
	return UNITY_END();
}

#ifdef __cplusplus
}
#endif
