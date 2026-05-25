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

void test_soc_reg_read()
{

    uint8_t data1[] = {0x7E, 0x22, 0x12, 0x34, 0x56, 0x78};
    soc_reg_read(data1, 6);
}
/**
 * @brief Unit test main entry
 * @details Execute all test cases and print result
 * @retval Test status code
 */
int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_soc_reg_read);

    return UNITY_END();
}
