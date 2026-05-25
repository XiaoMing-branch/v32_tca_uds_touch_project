#include "unity.h"
#include "fff.h"
#include "diagnosticIII.h"

/**
 * @brief FFF global variable definition
 */
DEFINE_FFF_GLOBALS;

void setUp(void)
{
    FFF_RESET_HISTORY();
}

void tearDown(void)
{
}

void test_soc_reg_write()
{
    uint8_t data1[] = {
        0x7E,                   /* ptr[0]: NAD */
        0x2E,                   /* ptr[1]: SID */
        0x10, 0x00, 0x00, 0x00, /* ptr[2]~[5]: Address */
        0xAA, 0xBB, 0xCC, 0xDD, /* ptr[6]~[9]: Value to write */
        0x00, 0x00              /* 冗余填充 */
    };
    soc_reg_write(data1, 10);
}

/**
 * @brief Unit test main entry
 * @details Execute all test cases and print result
 * @retval Test status code
 */
int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_soc_reg_write);
    
    return UNITY_END();
}
