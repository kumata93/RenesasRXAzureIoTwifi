#include "unity.h"
#include "mock_env.h"
#include "nx_driver_rx_wifi.h"
#include <string.h>

static NX_IP test_ip;
static NX_PACKET_POOL test_pool;

static void init_and_enable_driver(void)
{
    NX_IP_DRIVER req;
    memset(&test_ip, 0, sizeof(test_ip));
    test_ip.nx_ip_thread.tx_thread_priority = 1U;
    test_ip.nx_ip_default_packet_pool = &test_pool;

    memset(&req, 0, sizeof(req));
    req.nx_ip_driver_ptr = &test_ip;
    req.nx_ip_driver_interface = &test_ip.nx_ip_interface[0];
    req.nx_ip_driver_command = NX_LINK_INTERFACE_ATTACH;
    nx_driver_rx_wifi(&req);
    TEST_ASSERT_EQUAL_UINT(NX_SUCCESS, req.nx_ip_driver_status);

    req.nx_ip_driver_command = NX_LINK_INITIALIZE;
    nx_driver_rx_wifi(&req);
    TEST_ASSERT_EQUAL_UINT(NX_SUCCESS, req.nx_ip_driver_status);

    req.nx_ip_driver_command = NX_LINK_ENABLE;
    nx_driver_rx_wifi(&req);
    TEST_ASSERT_EQUAL_UINT(NX_SUCCESS, req.nx_ip_driver_status);
    TEST_ASSERT_NOT_NULL(test_ip.nx_ip_interface[0].nx_interface_tcpip_offload_handler);
}

static void disable_driver(void)
{
    NX_IP_DRIVER req;
    memset(&req, 0, sizeof(req));
    req.nx_ip_driver_ptr = &test_ip;
    req.nx_ip_driver_interface = &test_ip.nx_ip_interface[0];
    req.nx_ip_driver_command = NX_LINK_DISABLE;
    nx_driver_rx_wifi(&req);
}

void test_driver_initialize_sets_thread_priority_below_ip_thread(void)
{
    init_and_enable_driver();
    TEST_ASSERT_EQUAL_UINT(2U, g_mock.created_thread_priority);
    disable_driver();
}

void test_tcp_connect_create_failure_does_not_leak_slot(void)
{
    init_and_enable_driver();

    NX_TCP_SOCKET sock;
    NXD_ADDRESS local_ip = { NX_IP_VERSION_V4, { 0xC0A80164UL } };
    NXD_ADDRESS remote_ip = { NX_IP_VERSION_V4, { 0x0A000001UL } };
    UINT remote_port = 8883U;

    /* Fail CreateSocket 5 times (> NX_DRIVER_SOCKETS_MAXIMUM = 4) */
    for (int k = 0; k < 5; k++) {
        g_mock.create_socket_ret_queue[k] = -1;
    }
    g_mock.create_socket_ret_queue[5] = 2;
    g_mock.create_socket_queue_len = 6;

    for (int k = 0; k < 5; k++) {
        UINT st = test_ip.nx_ip_interface[0].nx_interface_tcpip_offload_handler(
            &test_ip, &test_ip.nx_ip_interface[0], &sock,
            NX_TCPIP_OFFLOAD_TCP_CLIENT_SOCKET_CONNECT, NX_NULL,
            &local_ip, &remote_ip, 1000U, &remote_port, 100U);
        TEST_ASSERT_EQUAL_UINT(NX_NOT_SUCCESSFUL, st);
    }

    /* 6th attempt must succeed (not NX_NO_MORE_ENTRIES) */
    UINT st = test_ip.nx_ip_interface[0].nx_interface_tcpip_offload_handler(
        &test_ip, &test_ip.nx_ip_interface[0], &sock,
        NX_TCPIP_OFFLOAD_TCP_CLIENT_SOCKET_CONNECT, NX_NULL,
        &local_ip, &remote_ip, 1000U, &remote_port, 100U);
    TEST_ASSERT_EQUAL_UINT(NX_SUCCESS, st);

    disable_driver();
}

void test_tcp_connect_failure_closes_hw_socket_and_frees_slot(void)
{
    init_and_enable_driver();

    NX_TCP_SOCKET sock;
    NXD_ADDRESS local_ip = { NX_IP_VERSION_V4, { 0xC0A80164UL } };
    NXD_ADDRESS remote_ip = { NX_IP_VERSION_V4, { 0x0A000001UL } };
    UINT remote_port = 443U;

    g_mock.connect_socket_ret = WIFI_ERR_MODULE_COM;

    for (int k = 0; k < 5; k++) {
        UINT st = test_ip.nx_ip_interface[0].nx_interface_tcpip_offload_handler(
            &test_ip, &test_ip.nx_ip_interface[0], &sock,
            NX_TCPIP_OFFLOAD_TCP_CLIENT_SOCKET_CONNECT, NX_NULL,
            &local_ip, &remote_ip, 1000U, &remote_port, 100U);
        TEST_ASSERT_EQUAL_UINT(NX_NOT_SUCCESSFUL, st);
    }
    TEST_ASSERT_EQUAL_INT(5, g_mock.close_socket_calls);

    g_mock.connect_socket_ret = WIFI_SUCCESS;
    UINT st = test_ip.nx_ip_interface[0].nx_interface_tcpip_offload_handler(
        &test_ip, &test_ip.nx_ip_interface[0], &sock,
        NX_TCPIP_OFFLOAD_TCP_CLIENT_SOCKET_CONNECT, NX_NULL,
        &local_ip, &remote_ip, 1000U, &remote_port, 100U);
    TEST_ASSERT_EQUAL_UINT(NX_SUCCESS, st);

    disable_driver();
}

void test_udp_send_preserves_context_and_reuses_socket_for_same_remote(void)
{
    init_and_enable_driver();

    NX_UDP_SOCKET udp0, udp1;
    NXD_ADDRESS local_ip = { NX_IP_VERSION_V4, { 0xC0A80164UL } };
    NXD_ADDRESS remote_ip = { NX_IP_VERSION_V4, { 0x08080808UL } };
    NXD_ADDRESS remote_ip2 = { NX_IP_VERSION_V4, { 0x01010101UL } };
    UINT remote_port = 53U;
    NX_PACKET pkt;

    /* Bind two UDP sockets -> udp0 gets slot 0, udp1 gets slot 1 */
    TEST_ASSERT_EQUAL_UINT(NX_SUCCESS, test_ip.nx_ip_interface[0].nx_interface_tcpip_offload_handler(
        &test_ip, &test_ip.nx_ip_interface[0], &udp0,
        NX_TCPIP_OFFLOAD_UDP_SOCKET_BIND, NX_NULL, &local_ip, &remote_ip, 5000U, &remote_port, 0U));
    TEST_ASSERT_EQUAL_UINT(NX_SUCCESS, test_ip.nx_ip_interface[0].nx_interface_tcpip_offload_handler(
        &test_ip, &test_ip.nx_ip_interface[0], &udp1,
        NX_TCPIP_OFFLOAD_UDP_SOCKET_BIND, NX_NULL, &local_ip, &remote_ip, 5001U, &remote_port, 0U));
    TEST_ASSERT_EQUAL_PTR((VOID *)1, udp1.nx_udp_socket_tcpip_offload_context);

    g_mock.create_socket_ret_queue[0] = 3;
    g_mock.create_socket_ret_queue[1] = 2;
    g_mock.create_socket_queue_len = 2;

    /* Send twice on udp1 to same destination */
    for (int k = 0; k < 2; k++) {
        memset(&pkt, 0, sizeof(pkt));
        pkt.nx_packet_prepend_ptr = pkt.buffer;
        pkt.nx_packet_append_ptr = pkt.buffer + 16;
        pkt.nx_packet_length = 16;
        TEST_ASSERT_EQUAL_UINT(NX_SUCCESS, test_ip.nx_ip_interface[0].nx_interface_tcpip_offload_handler(
            &test_ip, &test_ip.nx_ip_interface[0], &udp1,
            NX_TCPIP_OFFLOAD_UDP_SOCKET_SEND, &pkt, &local_ip, &remote_ip, 5001U, &remote_port, 50U));
    }

    /* Context must remain 1, CreateSocket called only once, timeout = 500ms */
    TEST_ASSERT_EQUAL_PTR((VOID *)1, udp1.nx_udp_socket_tcpip_offload_context);
    TEST_ASSERT_EQUAL_INT(1, g_mock.create_socket_calls);
    TEST_ASSERT_EQUAL_UINT(3U, g_mock.last_send_socket_id);
    TEST_ASSERT_EQUAL_UINT(500U, g_mock.last_send_timeout_ms);

    /* Send on udp1 to a different destination -> must close old hw socket (3) first */
    memset(&pkt, 0, sizeof(pkt));
    pkt.nx_packet_prepend_ptr = pkt.buffer;
    pkt.nx_packet_append_ptr = pkt.buffer + 16;
    pkt.nx_packet_length = 16;
    TEST_ASSERT_EQUAL_UINT(NX_SUCCESS, test_ip.nx_ip_interface[0].nx_interface_tcpip_offload_handler(
        &test_ip, &test_ip.nx_ip_interface[0], &udp1,
        NX_TCPIP_OFFLOAD_UDP_SOCKET_SEND, &pkt, &local_ip, &remote_ip2, 5001U, &remote_port, 0U));

    TEST_ASSERT_EQUAL_INT(1, g_mock.close_socket_calls);
    TEST_ASSERT_EQUAL_UINT(3U, g_mock.last_closed_socket_id);
    TEST_ASSERT_EQUAL_INT(2, g_mock.create_socket_calls);
    TEST_ASSERT_EQUAL_UINT(1U, g_mock.last_send_timeout_ms);

    disable_driver();
}

void test_tcp_send_timeout_conversion(void)
{
    init_and_enable_driver();

    NX_TCP_SOCKET sock;
    NXD_ADDRESS local_ip = { NX_IP_VERSION_V4, { 0xC0A80164UL } };
    NXD_ADDRESS remote_ip = { NX_IP_VERSION_V4, { 0x0A000001UL } };
    UINT remote_port = 8883U;
    NX_PACKET pkt;

    TEST_ASSERT_EQUAL_UINT(NX_SUCCESS, test_ip.nx_ip_interface[0].nx_interface_tcpip_offload_handler(
        &test_ip, &test_ip.nx_ip_interface[0], &sock,
        NX_TCPIP_OFFLOAD_TCP_CLIENT_SOCKET_CONNECT, NX_NULL,
        &local_ip, &remote_ip, 1000U, &remote_port, 100U));

    /* 50 ticks (< 1s) -> 500ms (not 0) */
    memset(&pkt, 0, sizeof(pkt));
    pkt.nx_packet_prepend_ptr = pkt.buffer;
    pkt.nx_packet_append_ptr = pkt.buffer + 32;
    pkt.nx_packet_length = 32;
    TEST_ASSERT_EQUAL_UINT(NX_SUCCESS, test_ip.nx_ip_interface[0].nx_interface_tcpip_offload_handler(
        &test_ip, &test_ip.nx_ip_interface[0], &sock,
        NX_TCPIP_OFFLOAD_TCP_SOCKET_SEND, &pkt, &local_ip, &remote_ip, 1000U, &remote_port, 50U));
    TEST_ASSERT_EQUAL_UINT(500U, g_mock.last_send_timeout_ms);

    /* 0 ticks -> clamped to 1ms */
    memset(&pkt, 0, sizeof(pkt));
    pkt.nx_packet_prepend_ptr = pkt.buffer;
    pkt.nx_packet_append_ptr = pkt.buffer + 32;
    pkt.nx_packet_length = 32;
    TEST_ASSERT_EQUAL_UINT(NX_SUCCESS, test_ip.nx_ip_interface[0].nx_interface_tcpip_offload_handler(
        &test_ip, &test_ip.nx_ip_interface[0], &sock,
        NX_TCPIP_OFFLOAD_TCP_SOCKET_SEND, &pkt, &local_ip, &remote_ip, 1000U, &remote_port, 0U));
    TEST_ASSERT_EQUAL_UINT(1U, g_mock.last_send_timeout_ms);

    /* 1000 ticks -> clamped to 3000ms */
    memset(&pkt, 0, sizeof(pkt));
    pkt.nx_packet_prepend_ptr = pkt.buffer;
    pkt.nx_packet_append_ptr = pkt.buffer + 32;
    pkt.nx_packet_length = 32;
    TEST_ASSERT_EQUAL_UINT(NX_SUCCESS, test_ip.nx_ip_interface[0].nx_interface_tcpip_offload_handler(
        &test_ip, &test_ip.nx_ip_interface[0], &sock,
        NX_TCPIP_OFFLOAD_TCP_SOCKET_SEND, &pkt, &local_ip, &remote_ip, 1000U, &remote_port, 1000U));
    TEST_ASSERT_EQUAL_UINT(3000U, g_mock.last_send_timeout_ms);

    disable_driver();
}

void test_driver_status_capability_multicast_and_raw_packet_send(void)
{
    NX_IP_DRIVER req;
    ULONG link_status = 0UL;
    ULONG cap = 0UL;
    NX_PACKET pkt;

    init_and_enable_driver();

    /* GET_STATUS returns NX_SUCCESS and interface link_up is NX_TRUE when enabled */
    memset(&req, 0, sizeof(req));
    req.nx_ip_driver_ptr = &test_ip;
    req.nx_ip_driver_interface = &test_ip.nx_ip_interface[0];
    req.nx_ip_driver_command = NX_LINK_GET_STATUS;
    req.nx_ip_driver_return_ptr = &link_status;
    nx_driver_rx_wifi(&req);
    TEST_ASSERT_EQUAL_UINT(NX_SUCCESS, req.nx_ip_driver_status);
    TEST_ASSERT_EQUAL_UINT(NX_TRUE, test_ip.nx_ip_interface[0].nx_interface_link_up);

    /* CAPABILITY_GET / SET */
    req.nx_ip_driver_command = NX_INTERFACE_CAPABILITY_GET;
    req.nx_ip_driver_return_ptr = &cap;
    nx_driver_rx_wifi(&req);
    TEST_ASSERT_EQUAL_UINT(NX_SUCCESS, req.nx_ip_driver_status);
    TEST_ASSERT_EQUAL_HEX32(NX_INTERFACE_CAPABILITY_TCPIP_OFFLOAD, cap);

    req.nx_ip_driver_command = NX_INTERFACE_CAPABILITY_SET;
    nx_driver_rx_wifi(&req);
    TEST_ASSERT_EQUAL_UINT(NX_SUCCESS, req.nx_ip_driver_status);

    /* MULTICAST_JOIN / LEAVE return NX_DRIVER_ERROR in offload mode */
    req.nx_ip_driver_command = NX_LINK_MULTICAST_JOIN;
    nx_driver_rx_wifi(&req);
    TEST_ASSERT_EQUAL_UINT(NX_DRIVER_ERROR, req.nx_ip_driver_status);

    req.nx_ip_driver_command = NX_LINK_MULTICAST_LEAVE;
    nx_driver_rx_wifi(&req);
    TEST_ASSERT_EQUAL_UINT(NX_DRIVER_ERROR, req.nx_ip_driver_status);

    /* Raw packet send is unsupported in offload mode and must release packet */
    memset(&pkt, 0, sizeof(pkt));
    req.nx_ip_driver_command = NX_LINK_PACKET_SEND;
    req.nx_ip_driver_packet = &pkt;
    nx_driver_rx_wifi(&req);
    TEST_ASSERT_NOT_EQUAL_UINT(NX_SUCCESS, req.nx_ip_driver_status);
    TEST_ASSERT_EQUAL_INT(1, g_mock.packet_transmit_release_calls);

    disable_driver();

    /* After disable, interface link_up is NX_FALSE */
    memset(&req, 0, sizeof(req));
    req.nx_ip_driver_ptr = &test_ip;
    req.nx_ip_driver_interface = &test_ip.nx_ip_interface[0];
    req.nx_ip_driver_command = NX_LINK_GET_STATUS;
    req.nx_ip_driver_return_ptr = &link_status;
    nx_driver_rx_wifi(&req);
    TEST_ASSERT_EQUAL_UINT(NX_SUCCESS, req.nx_ip_driver_status);
    TEST_ASSERT_EQUAL_UINT(NX_FALSE, test_ip.nx_ip_interface[0].nx_interface_link_up);
}

void test_tcp_disconnect_and_udp_unbind_free_slots_and_close_hw_sockets(void)
{
    init_and_enable_driver();

    NX_TCP_SOCKET tcp_socks[4];
    NX_UDP_SOCKET udp_sock;
    NXD_ADDRESS local_ip = { NX_IP_VERSION_V4, { 0xC0A80164UL } };
    NXD_ADDRESS remote_ip = { NX_IP_VERSION_V4, { 0x0A000001UL } };
    UINT remote_port = 8883U;
    NX_PACKET pkt;

    /* Fill all 4 slots with TCP sockets */
    for (int k = 0; k < 4; k++) {
        TEST_ASSERT_EQUAL_UINT(NX_SUCCESS, test_ip.nx_ip_interface[0].nx_interface_tcpip_offload_handler(
            &test_ip, &test_ip.nx_ip_interface[0], &tcp_socks[k],
            NX_TCPIP_OFFLOAD_TCP_CLIENT_SOCKET_CONNECT, NX_NULL,
            &local_ip, &remote_ip, (UINT)(1000 + k), &remote_port, 100U));
    }

    /* 5th bind must fail with NX_NO_MORE_ENTRIES */
    TEST_ASSERT_EQUAL_UINT(NX_NO_MORE_ENTRIES, test_ip.nx_ip_interface[0].nx_interface_tcpip_offload_handler(
        &test_ip, &test_ip.nx_ip_interface[0], &udp_sock,
        NX_TCPIP_OFFLOAD_UDP_SOCKET_BIND, NX_NULL,
        &local_ip, &remote_ip, 5000U, &remote_port, 0U));

    /* Disconnect one TCP socket -> closes hw socket and frees slot */
    TEST_ASSERT_EQUAL_UINT(NX_SUCCESS, test_ip.nx_ip_interface[0].nx_interface_tcpip_offload_handler(
        &test_ip, &test_ip.nx_ip_interface[0], &tcp_socks[1],
        NX_TCPIP_OFFLOAD_TCP_SOCKET_DISCONNECT, NX_NULL,
        &local_ip, &remote_ip, 1001U, &remote_port, 0U));
    TEST_ASSERT_EQUAL_INT(1, g_mock.close_socket_calls);

    /* Now UDP bind, send, and unbind must succeed */
    TEST_ASSERT_EQUAL_UINT(NX_SUCCESS, test_ip.nx_ip_interface[0].nx_interface_tcpip_offload_handler(
        &test_ip, &test_ip.nx_ip_interface[0], &udp_sock,
        NX_TCPIP_OFFLOAD_UDP_SOCKET_BIND, NX_NULL,
        &local_ip, &remote_ip, 5000U, &remote_port, 0U));

    memset(&pkt, 0, sizeof(pkt));
    pkt.nx_packet_prepend_ptr = pkt.buffer;
    pkt.nx_packet_append_ptr = pkt.buffer + 10;
    pkt.nx_packet_length = 10;
    TEST_ASSERT_EQUAL_UINT(NX_SUCCESS, test_ip.nx_ip_interface[0].nx_interface_tcpip_offload_handler(
        &test_ip, &test_ip.nx_ip_interface[0], &udp_sock,
        NX_TCPIP_OFFLOAD_UDP_SOCKET_SEND, &pkt,
        &local_ip, &remote_ip, 5000U, &remote_port, 100U));

    TEST_ASSERT_EQUAL_UINT(NX_SUCCESS, test_ip.nx_ip_interface[0].nx_interface_tcpip_offload_handler(
        &test_ip, &test_ip.nx_ip_interface[0], &udp_sock,
        NX_TCPIP_OFFLOAD_UDP_SOCKET_UNBIND, NX_NULL,
        &local_ip, &remote_ip, 5000U, &remote_port, 0U));
    TEST_ASSERT_EQUAL_INT(2, g_mock.close_socket_calls);

    disable_driver();
}

void test_tcp_send_chained_packets_and_partial_send_failure(void)
{
    init_and_enable_driver();

    NX_TCP_SOCKET sock;
    NXD_ADDRESS local_ip = { NX_IP_VERSION_V4, { 0xC0A80164UL } };
    NXD_ADDRESS remote_ip = { NX_IP_VERSION_V4, { 0x0A000001UL } };
    UINT remote_port = 8883U;
    NX_PACKET pkt1, pkt2;

    TEST_ASSERT_EQUAL_UINT(NX_SUCCESS, test_ip.nx_ip_interface[0].nx_interface_tcpip_offload_handler(
        &test_ip, &test_ip.nx_ip_interface[0], &sock,
        NX_TCPIP_OFFLOAD_TCP_CLIENT_SOCKET_CONNECT, NX_NULL,
        &local_ip, &remote_ip, 1000U, &remote_port, 100U));

    memset(&pkt1, 0, sizeof(pkt1));
    memset(&pkt2, 0, sizeof(pkt2));
    pkt1.nx_packet_prepend_ptr = pkt1.buffer;
    pkt1.nx_packet_append_ptr = pkt1.buffer + 20;
    pkt1.nx_packet_length = 40;
    pkt1.nx_packet_next = &pkt2;

    pkt2.nx_packet_prepend_ptr = pkt2.buffer;
    pkt2.nx_packet_append_ptr = pkt2.buffer + 20;

    TEST_ASSERT_EQUAL_UINT(NX_SUCCESS, test_ip.nx_ip_interface[0].nx_interface_tcpip_offload_handler(
        &test_ip, &test_ip.nx_ip_interface[0], &sock,
        NX_TCPIP_OFFLOAD_TCP_SOCKET_SEND, &pkt1,
        &local_ip, &remote_ip, 1000U, &remote_port, 100U));
    TEST_ASSERT_EQUAL_INT(2, g_mock.send_socket_calls);
    TEST_ASSERT_EQUAL_INT(1, g_mock.packet_transmit_release_calls);

    /* Partial/failed send must return NX_NOT_SUCCESSFUL */
    g_mock.send_socket_ret_override = 5;
    TEST_ASSERT_EQUAL_UINT(NX_NOT_SUCCESSFUL, test_ip.nx_ip_interface[0].nx_interface_tcpip_offload_handler(
        &test_ip, &test_ip.nx_ip_interface[0], &sock,
        NX_TCPIP_OFFLOAD_TCP_SOCKET_SEND, &pkt1,
        &local_ip, &remote_ip, 1000U, &remote_port, 100U));

    disable_driver();
}
