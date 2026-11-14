#ifndef TEST_MOCK_NX_API_H
#define TEST_MOCK_NX_API_H

#include "tx_api.h"

#define NX_ENABLE_TCPIP_OFFLOAD
#define NX_ENABLE_INTERFACE_CAPABILITY
#define NX_IP_PERIODIC_RATE             100U

#define NX_SUCCESS                      0x00U
#define NX_NOT_SUCCESSFUL               0x01U
#define NX_NO_MORE_ENTRIES              0x17U
#define NX_ALREADY_ENABLED              0x15U
#define NX_UNHANDLED_COMMAND            0x44U

#define NX_TRUE                         1U
#define NX_FALSE                        0U
#define NX_NULL                         ((void *)0)
#define NX_NO_WAIT                      0U
#define NX_WAIT_FOREVER                 0xFFFFFFFFUL

#define NX_IP_VERSION_V4                4U
#define NX_PROTOCOL_TCP                 6U
#define NX_PROTOCOL_UDP                 17U
#define NX_TCP_PACKET                   1U
#define NX_UDP_PACKET                   2U
#define NX_PHYSICAL_HEADER              16U

#define NX_PARAMETER_NOT_USED(p)        ((void)(p))

#define NX_LINK_PACKET_SEND             0U
#define NX_LINK_INITIALIZE              1U
#define NX_LINK_ENABLE                  2U
#define NX_LINK_DISABLE                 3U
#define NX_LINK_PACKET_BROADCAST        4U
#define NX_LINK_ARP_SEND                5U
#define NX_LINK_ARP_RESPONSE_SEND       6U
#define NX_LINK_RARP_SEND               7U
#define NX_LINK_MULTICAST_JOIN          8U
#define NX_LINK_MULTICAST_LEAVE         9U
#define NX_LINK_GET_STATUS              10U
#define NX_LINK_DEFERRED_PROCESSING     16U
#define NX_LINK_INTERFACE_ATTACH        17U
#define NX_INTERFACE_CAPABILITY_GET     19U
#define NX_INTERFACE_CAPABILITY_SET     20U

#define NX_INTERFACE_CAPABILITY_TCPIP_OFFLOAD 0x00000100UL

#define NX_TCPIP_OFFLOAD_TCP_CLIENT_SOCKET_CONNECT  1U
#define NX_TCPIP_OFFLOAD_TCP_SOCKET_DISCONNECT      2U
#define NX_TCPIP_OFFLOAD_TCP_SOCKET_SEND            3U
#define NX_TCPIP_OFFLOAD_UDP_SOCKET_BIND            4U
#define NX_TCPIP_OFFLOAD_UDP_SOCKET_UNBIND          5U
#define NX_TCPIP_OFFLOAD_UDP_SOCKET_SEND            6U

typedef struct NXD_ADDRESS_STRUCT {
    UINT nxd_ip_version;
    union {
        ULONG v4;
    } nxd_ip_address;
} NXD_ADDRESS;

struct NX_IP_STRUCT;
struct NX_INTERFACE_STRUCT;

typedef struct NX_PACKET_STRUCT {
    UCHAR *nx_packet_prepend_ptr;
    UCHAR *nx_packet_append_ptr;
    UCHAR *nx_packet_data_end;
    ULONG  nx_packet_length;
    struct NX_INTERFACE_STRUCT *nx_packet_ip_interface;
    struct NX_PACKET_STRUCT *nx_packet_next;
    UCHAR  buffer[1536];
} NX_PACKET;

typedef struct NX_PACKET_POOL_STRUCT {
    UINT dummy;
} NX_PACKET_POOL;

typedef struct NX_INTERFACE_STRUCT {
    CHAR  *nx_interface_name;
    ULONG  nx_interface_ip_mtu_size;
    ULONG  nx_interface_physical_address_msw;
    ULONG  nx_interface_physical_address_lsw;
    UCHAR  nx_interface_address_mapping_needed;
    UCHAR  nx_interface_link_up;
    ULONG  nx_interface_capability_flag;
    UINT (*nx_interface_tcpip_offload_handler)(struct NX_IP_STRUCT *ip_ptr,
                                               struct NX_INTERFACE_STRUCT *interface_ptr,
                                               VOID *socket_ptr, UINT operation, NX_PACKET *packet_ptr,
                                               NXD_ADDRESS *local_ip, NXD_ADDRESS *remote_ip,
                                               UINT local_port, UINT *remote_port, UINT wait_option);
} NX_INTERFACE;

typedef struct NX_IP_STRUCT {
    TX_THREAD nx_ip_thread;
    TX_MUTEX  nx_ip_protection;
    NX_PACKET_POOL *nx_ip_default_packet_pool;
    NX_INTERFACE nx_ip_interface[1];
} NX_IP;

typedef struct NX_IP_DRIVER_STRUCT {
    UINT          nx_ip_driver_command;
    UINT          nx_ip_driver_status;
    ULONG         nx_ip_driver_physical_address_msw;
    ULONG         nx_ip_driver_physical_address_lsw;
    NX_PACKET    *nx_ip_driver_packet;
    ULONG        *nx_ip_driver_return_ptr;
    NX_IP        *nx_ip_driver_ptr;
    NX_INTERFACE *nx_ip_driver_interface;
} NX_IP_DRIVER;

typedef struct NX_TCP_SOCKET_STRUCT {
    VOID *nx_tcp_socket_tcpip_offload_context;
} NX_TCP_SOCKET;

typedef struct NX_UDP_SOCKET_STRUCT {
    VOID *nx_udp_socket_tcpip_offload_context;
} NX_UDP_SOCKET;

UINT nx_packet_allocate(NX_PACKET_POOL *pool_ptr, NX_PACKET **packet_ptr, ULONG packet_type, ULONG wait_option);
UINT nx_packet_release(NX_PACKET *packet_ptr);
UINT nx_packet_transmit_release(NX_PACKET *packet_ptr);
VOID _nx_tcp_socket_driver_packet_receive(NX_TCP_SOCKET *socket_ptr, NX_PACKET *packet_ptr);
VOID _nx_udp_socket_driver_packet_receive(NX_UDP_SOCKET *socket_ptr, NX_PACKET *packet_ptr,
                                          NXD_ADDRESS *local_ip, NXD_ADDRESS *remote_ip, UINT remote_port);
UINT nx_ip_address_set(NX_IP *ip_ptr, ULONG ip_address, ULONG network_mask);
UINT nx_ip_gateway_address_set(NX_IP *ip_ptr, ULONG ip_address);

#endif
