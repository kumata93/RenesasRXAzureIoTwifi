#include "unity.h"
#include "mock_env.h"

void test_driver_initialize_sets_thread_priority_below_ip_thread(void);
void test_tcp_connect_create_failure_does_not_leak_slot(void);
void test_tcp_connect_failure_closes_hw_socket_and_frees_slot(void);
void test_udp_send_preserves_context_and_reuses_socket_for_same_remote(void);
void test_tcp_send_timeout_conversion(void);
void test_driver_status_capability_multicast_and_raw_packet_send(void);
void test_tcp_disconnect_and_udp_unbind_free_slots_and_close_hw_sockets(void);
void test_tcp_send_chained_packets_and_partial_send_failure(void);

void test_sample_network_configure_success_returns_nx_success(void);
void test_sample_network_configure_returns_error_on_ip_set_fail(void);
void test_sample_network_configure_returns_error_on_gw_set_fail(void);
void test_compat_clock_and_sleep(void);
void test_demo_printf_clamps_long_string_and_handles_send_error(void);
void test_sample_network_configure_handles_wifi_errors_and_null_dns(void);
void test_demo_printf_charput_and_tei_callback(void);

void setUp(void)
{
    mock_env_reset();
}

void tearDown(void)
{
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_driver_initialize_sets_thread_priority_below_ip_thread);
    RUN_TEST(test_tcp_connect_create_failure_does_not_leak_slot);
    RUN_TEST(test_tcp_connect_failure_closes_hw_socket_and_frees_slot);
    RUN_TEST(test_udp_send_preserves_context_and_reuses_socket_for_same_remote);
    RUN_TEST(test_tcp_send_timeout_conversion);
    RUN_TEST(test_driver_status_capability_multicast_and_raw_packet_send);
    RUN_TEST(test_tcp_disconnect_and_udp_unbind_free_slots_and_close_hw_sockets);
    RUN_TEST(test_tcp_send_chained_packets_and_partial_send_failure);

    RUN_TEST(test_sample_network_configure_success_returns_nx_success);
    RUN_TEST(test_sample_network_configure_returns_error_on_ip_set_fail);
    RUN_TEST(test_sample_network_configure_returns_error_on_gw_set_fail);
    RUN_TEST(test_compat_clock_and_sleep);
    RUN_TEST(test_demo_printf_clamps_long_string_and_handles_send_error);
    RUN_TEST(test_sample_network_configure_handles_wifi_errors_and_null_dns);
    RUN_TEST(test_demo_printf_charput_and_tei_callback);

    return UNITY_END();
}
