#ifndef TEST_MOCK_ENV_H
#define TEST_MOCK_ENV_H

#include "tx_api.h"
#include "nx_api.h"
#include "r_wifi_sx_ulpgn_if.h"
#include "r_sci_rx_if.h"

typedef struct {
    /* ThreadX */
    ULONG fake_time_ticks;
    ULONG last_sleep_ticks;
    int   sleep_calls;
    UINT  created_thread_priority;
    int   thread_create_calls;
    int   sem_get_calls;

    /* NetX */
    UINT  ip_addr_set_ret;
    UINT  ip_gw_set_ret;
    int   packet_transmit_release_calls;

    /* Wi-Fi */
    wifi_err_t wifi_open_ret;
    wifi_err_t wifi_connect_ret;
    wifi_err_t wifi_get_ip_ret;
    wifi_ip_configuration_t wifi_ip_cfg;

    int32_t    create_socket_ret_queue[16];
    int        create_socket_queue_len;
    int        create_socket_calls;
    wifi_err_t connect_socket_ret;
    int        connect_socket_calls;
    int        close_socket_calls;
    uint8_t    last_closed_socket_id;

    int        send_socket_calls;
    uint8_t    last_send_socket_id;
    uint32_t   last_send_timeout_ms;
    int32_t    send_socket_ret_override; /* -999 = return length */

    /* SCI */
    sci_err_t  sci_send_ret;
    int        sci_send_calls;
    uint16_t   sci_send_last_len;
} mock_env_t;

extern mock_env_t g_mock;

void mock_env_reset(void);

#endif
