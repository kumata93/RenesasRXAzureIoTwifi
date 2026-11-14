#include "mock_env.h"
#include <string.h>

mock_env_t g_mock;

void mock_env_reset(void)
{
    memset(&g_mock, 0, sizeof(g_mock));
    g_mock.send_socket_ret_override = -999;
    g_mock.wifi_ip_cfg.ipaddress = 0xC0A80164UL;  /* 192.168.1.100 */
    g_mock.wifi_ip_cfg.subnetmask = 0xFFFFFF00UL; /* 255.255.255.0 */
    g_mock.wifi_ip_cfg.gateway = 0xC0A80101UL;    /* 192.168.1.1 */
}

/* ThreadX mocks */
UINT tx_thread_create(TX_THREAD *thread_ptr, CHAR *name_ptr,
                      VOID (*entry_function)(ULONG), ULONG entry_input,
                      VOID *stack_start, ULONG stack_size,
                      UINT priority, UINT preempt_threshold,
                      ULONG time_slice, UINT auto_start)
{
    (void)entry_function; (void)entry_input; (void)stack_start; (void)stack_size;
    (void)preempt_threshold; (void)time_slice; (void)auto_start;
    if (thread_ptr) {
        thread_ptr->name = name_ptr;
        thread_ptr->tx_thread_priority = priority;
    }
    g_mock.created_thread_priority = priority;
    g_mock.thread_create_calls++;
    return TX_SUCCESS;
}

UINT tx_thread_info_get(TX_THREAD *thread_ptr, CHAR **name, UINT *state,
                        ULONG *run_count, UINT *priority, UINT *preemption_threshold,
                        ULONG *time_slice, TX_THREAD **next_thread,
                        TX_THREAD **next_suspended_thread)
{
    (void)name; (void)state; (void)run_count; (void)preemption_threshold;
    (void)time_slice; (void)next_thread; (void)next_suspended_thread;
    if (thread_ptr == TX_NULL) {
        return 0x0EU; /* TX_THREAD_ERROR */
    }
    if (priority) {
        *priority = thread_ptr->tx_thread_priority;
    }
    return TX_SUCCESS;
}

UINT tx_thread_reset(TX_THREAD *thread_ptr) { (void)thread_ptr; return TX_SUCCESS; }
UINT tx_thread_resume(TX_THREAD *thread_ptr) { (void)thread_ptr; return TX_SUCCESS; }
UINT tx_thread_suspend(TX_THREAD *thread_ptr) { (void)thread_ptr; return TX_SUCCESS; }
UINT tx_thread_terminate(TX_THREAD *thread_ptr) { (void)thread_ptr; return TX_SUCCESS; }

UINT tx_thread_sleep(ULONG timer_ticks)
{
    g_mock.last_sleep_ticks = timer_ticks;
    g_mock.sleep_calls++;
    return TX_SUCCESS;
}

ULONG tx_time_get(VOID)
{
    return g_mock.fake_time_ticks;
}

UINT tx_mutex_create(TX_MUTEX *mutex_ptr, CHAR *name_ptr, UINT inherit)
{
    (void)inherit;
    if (mutex_ptr) {
        mutex_ptr->name = name_ptr;
        mutex_ptr->locked = 0;
    }
    return TX_SUCCESS;
}

UINT tx_mutex_get(TX_MUTEX *mutex_ptr, ULONG wait_option)
{
    (void)wait_option;
    if (mutex_ptr) {
        mutex_ptr->locked++;
    }
    return TX_SUCCESS;
}

UINT tx_mutex_put(TX_MUTEX *mutex_ptr)
{
    if (mutex_ptr && mutex_ptr->locked > 0) {
        mutex_ptr->locked--;
    }
    return TX_SUCCESS;
}

UINT tx_semaphore_create(TX_SEMAPHORE *semaphore_ptr, CHAR *name_ptr, ULONG initial_count)
{
    if (semaphore_ptr) {
        semaphore_ptr->name = name_ptr;
        semaphore_ptr->count = initial_count;
    }
    return TX_SUCCESS;
}

UINT tx_semaphore_get(TX_SEMAPHORE *semaphore_ptr, ULONG wait_option)
{
    (void)semaphore_ptr; (void)wait_option;
    g_mock.sem_get_calls++;
    return TX_SUCCESS;
}

UINT tx_semaphore_put(TX_SEMAPHORE *semaphore_ptr)
{
    if (semaphore_ptr) {
        semaphore_ptr->count++;
    }
    return TX_SUCCESS;
}

/* NetX mocks */
UINT nx_packet_allocate(NX_PACKET_POOL *pool_ptr, NX_PACKET **packet_ptr, ULONG packet_type, ULONG wait_option)
{
    (void)pool_ptr; (void)packet_ptr; (void)packet_type; (void)wait_option;
    return NX_NOT_SUCCESSFUL;
}

UINT nx_packet_release(NX_PACKET *packet_ptr) { (void)packet_ptr; return NX_SUCCESS; }

UINT nx_packet_transmit_release(NX_PACKET *packet_ptr)
{
    (void)packet_ptr;
    g_mock.packet_transmit_release_calls++;
    return NX_SUCCESS;
}

VOID _nx_tcp_socket_driver_packet_receive(NX_TCP_SOCKET *socket_ptr, NX_PACKET *packet_ptr)
{
    (void)socket_ptr; (void)packet_ptr;
}

VOID _nx_udp_socket_driver_packet_receive(NX_UDP_SOCKET *socket_ptr, NX_PACKET *packet_ptr,
                                          NXD_ADDRESS *local_ip, NXD_ADDRESS *remote_ip, UINT remote_port)
{
    (void)socket_ptr; (void)packet_ptr; (void)local_ip; (void)remote_ip; (void)remote_port;
}

UINT nx_ip_address_set(NX_IP *ip_ptr, ULONG ip_address, ULONG network_mask)
{
    (void)ip_ptr; (void)ip_address; (void)network_mask;
    return g_mock.ip_addr_set_ret;
}

UINT nx_ip_gateway_address_set(NX_IP *ip_ptr, ULONG ip_address)
{
    (void)ip_ptr; (void)ip_address;
    return g_mock.ip_gw_set_ret;
}

/* Wi-Fi mocks */
wifi_err_t R_WIFI_SX_ULPGN_Open(void) { return g_mock.wifi_open_ret; }

wifi_err_t R_WIFI_SX_ULPGN_Connect(const char *ssid, const char *pass, int security, int dhcp, wifi_ip_configuration_t *ip_cfg)
{
    (void)ssid; (void)pass; (void)security; (void)dhcp; (void)ip_cfg;
    return g_mock.wifi_connect_ret;
}

wifi_err_t R_WIFI_SX_ULPGN_GetIpAddress(wifi_ip_configuration_t *ip_cfg)
{
    if (g_mock.wifi_get_ip_ret == WIFI_SUCCESS && ip_cfg) {
        *ip_cfg = g_mock.wifi_ip_cfg;
    }
    return g_mock.wifi_get_ip_ret;
}

int32_t R_WIFI_SX_ULPGN_CreateSocket(uint32_t type, uint32_t ip_version)
{
    (void)type; (void)ip_version;
    int idx = g_mock.create_socket_calls++;
    if (idx < g_mock.create_socket_queue_len) {
        return g_mock.create_socket_ret_queue[idx];
    }
    return idx % 4;
}

wifi_err_t R_WIFI_SX_ULPGN_ConnectSocket(uint8_t socket_number, uint32_t ip_address, uint16_t port, char *destination)
{
    (void)socket_number; (void)ip_address; (void)port; (void)destination;
    g_mock.connect_socket_calls++;
    return g_mock.connect_socket_ret;
}

int32_t R_WIFI_SX_ULPGN_SendSocket(uint8_t socket_number, uint8_t *data, uint32_t length, uint32_t timeout_ms)
{
    (void)data;
    g_mock.send_socket_calls++;
    g_mock.last_send_socket_id = socket_number;
    g_mock.last_send_timeout_ms = timeout_ms;
    if (g_mock.send_socket_ret_override != -999) {
        return g_mock.send_socket_ret_override;
    }
    return (int32_t)length;
}

int32_t R_WIFI_SX_ULPGN_ReceiveSocket(uint8_t socket_number, uint8_t *data, uint32_t length, uint32_t timeout_ms)
{
    (void)socket_number; (void)data; (void)length; (void)timeout_ms;
    return 0;
}

wifi_err_t R_WIFI_SX_ULPGN_CloseSocket(uint8_t socket_number)
{
    g_mock.close_socket_calls++;
    g_mock.last_closed_socket_id = socket_number;
    return WIFI_SUCCESS;
}

/* SCI mocks */
sci_err_t R_SCI_Open(uint8_t chan, sci_mode_t mode, sci_cfg_t *p_cfg, void (*p_callback)(void *p_args), sci_hdl_t *p_hdl)
{
    (void)chan; (void)mode; (void)p_cfg; (void)p_callback;
    if (p_hdl) {
        *p_hdl = (sci_hdl_t)0x1;
    }
    return SCI_SUCCESS;
}

void R_SCI_PinSet_SCI5(void) {}

sci_err_t R_SCI_Send(sci_hdl_t hdl, uint8_t *p_src, uint16_t length)
{
    (void)hdl; (void)p_src;
    g_mock.sci_send_calls++;
    g_mock.sci_send_last_len = length;
    return g_mock.sci_send_ret;
}
