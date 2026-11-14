#include "unity.h"
#include "mock_env.h"
#include "demo_printf.h"
#include <string.h>

UINT sample_network_configure(NX_IP *ip_ptr, ULONG *dns_server_address);
int32_t az_platform_clock_msec(int64_t *out_clock_msec);
int32_t az_platform_sleep_msec(int32_t milliseconds);

void test_sample_network_configure_success_returns_nx_success(void)
{
    NX_IP ip;
    ULONG dns = 0U;

    UINT st = sample_network_configure(&ip, &dns);
    TEST_ASSERT_EQUAL_UINT(NX_SUCCESS, st);
    TEST_ASSERT_EQUAL_HEX32(0xC0A80101UL, dns);
}

void test_sample_network_configure_returns_error_on_ip_set_fail(void)
{
    NX_IP ip;
    ULONG dns = 0U;
    g_mock.ip_addr_set_ret = NX_NOT_SUCCESSFUL;

    UINT st = sample_network_configure(&ip, &dns);
    TEST_ASSERT_EQUAL_UINT(NX_NOT_SUCCESSFUL, st);
}

void test_sample_network_configure_returns_error_on_gw_set_fail(void)
{
    NX_IP ip;
    ULONG dns = 0U;
    g_mock.ip_gw_set_ret = NX_NOT_SUCCESSFUL;

    UINT st = sample_network_configure(&ip, &dns);
    TEST_ASSERT_EQUAL_UINT(NX_NOT_SUCCESSFUL, st);
}

void test_compat_clock_and_sleep(void)
{
    int64_t ms = 0;

    TEST_ASSERT_EQUAL_INT32(-1, az_platform_clock_msec(NULL));

    g_mock.fake_time_ticks = 150UL;
    TEST_ASSERT_EQUAL_INT32(0, az_platform_clock_msec(&ms));
    TEST_ASSERT_EQUAL_INT64(1500, ms);

    az_platform_sleep_msec(0);
    TEST_ASSERT_EQUAL_INT(0, g_mock.sleep_calls);

    az_platform_sleep_msec(1);
    TEST_ASSERT_EQUAL_INT(1, g_mock.sleep_calls);
    TEST_ASSERT_EQUAL_UINT(1UL, g_mock.last_sleep_ticks);

    az_platform_sleep_msec(250);
    TEST_ASSERT_EQUAL_INT(2, g_mock.sleep_calls);
    TEST_ASSERT_EQUAL_UINT(25UL, g_mock.last_sleep_ticks);
}

void test_demo_printf_clamps_long_string_and_handles_send_error(void)
{
    demo_printf_init();

    char long_str[400];
    memset(long_str, 'A', sizeof(long_str) - 1);
    long_str[sizeof(long_str) - 1] = '\0';

    demo_printf("%s", long_str);
    TEST_ASSERT_EQUAL_INT(1, g_mock.sci_send_calls);
    TEST_ASSERT_EQUAL_UINT16(256U, g_mock.sci_send_last_len);
    TEST_ASSERT_EQUAL_INT(1, g_mock.sem_get_calls);

    /* When R_SCI_Send fails, semaphore wait must be skipped */
    g_mock.sci_send_ret = SCI_ERR_INSUFFICIENT_SPACE;
    demo_printf("hello");
    TEST_ASSERT_EQUAL_INT(2, g_mock.sci_send_calls);
    TEST_ASSERT_EQUAL_INT(1, g_mock.sem_get_calls);
}

void test_sample_network_configure_handles_wifi_errors_and_null_dns(void)
{
    NX_IP ip;

    /* NULL dns_server_address must succeed safely */
    TEST_ASSERT_EQUAL_UINT(NX_SUCCESS, sample_network_configure(&ip, NULL));

    /* Open failure */
    g_mock.wifi_open_ret = WIFI_ERR_MODULE_COM;
    TEST_ASSERT_NOT_EQUAL_UINT(NX_SUCCESS, sample_network_configure(&ip, NULL));
    g_mock.wifi_open_ret = WIFI_SUCCESS;

    /* Connect failure */
    g_mock.wifi_connect_ret = WIFI_ERR_NOT_CONNECT;
    TEST_ASSERT_NOT_EQUAL_UINT(NX_SUCCESS, sample_network_configure(&ip, NULL));
    g_mock.wifi_connect_ret = WIFI_SUCCESS;

    /* GetIpAddress failure */
    g_mock.wifi_get_ip_ret = WIFI_ERR_MODULE_COM;
    TEST_ASSERT_NOT_EQUAL_UINT(NX_SUCCESS, sample_network_configure(&ip, NULL));
}

void demo_printf_transmit_end(void *pArgs);
void my_sw_charput_function(char c);
extern TX_SEMAPHORE demo_printf_semaphore;

void test_demo_printf_charput_and_tei_callback(void)
{
    demo_printf_init();

    sci_cb_args_t args;
    args.event = SCI_EVT_TEI;
    demo_printf_semaphore.count = 0;
    demo_printf_transmit_end(&args);
    TEST_ASSERT_EQUAL_UINT(1UL, demo_printf_semaphore.count);

    my_sw_charput_function('Z');
    TEST_ASSERT_EQUAL_INT(1, g_mock.sci_send_calls);
    TEST_ASSERT_EQUAL_UINT16(1U, g_mock.sci_send_last_len);
    TEST_ASSERT_EQUAL_INT(1, g_mock.sem_get_calls);

    g_mock.sci_send_ret = SCI_ERR_INSUFFICIENT_SPACE;
    my_sw_charput_function('X');
    TEST_ASSERT_EQUAL_INT(2, g_mock.sci_send_calls);
    TEST_ASSERT_EQUAL_INT(1, g_mock.sem_get_calls);
}
