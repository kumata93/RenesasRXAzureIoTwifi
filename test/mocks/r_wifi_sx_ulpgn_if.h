#ifndef TEST_MOCK_R_WIFI_SX_ULPGN_IF_H
#define TEST_MOCK_R_WIFI_SX_ULPGN_IF_H

#include <stdint.h>

typedef enum {
    WIFI_SUCCESS = 0,
    WIFI_ERR_PARAMETER = -1,
    WIFI_ERR_NOT_OPEN = -2,
    WIFI_ERR_NOT_CONNECT = -3,
    WIFI_ERR_SOCKET_CREATE = -4,
    WIFI_ERR_MODULE_COM = -5,
    WIFI_ERR_TAKE_MUTEX = -6
} wifi_err_t;

#define WIFI_SOCKET_IP_PROTOCOL_TCP 0
#define WIFI_SOCKET_IP_PROTOCOL_UDP 1
#define WIFI_SOCKET_IP_VERSION_4    4
#define WIFI_SECURITY_WPA2          2

typedef struct {
    uint32_t ipaddress;
    uint32_t subnetmask;
    uint32_t gateway;
} wifi_ip_configuration_t;

wifi_err_t R_WIFI_SX_ULPGN_Open(void);
wifi_err_t R_WIFI_SX_ULPGN_Connect(const char *ssid, const char *pass, int security, int dhcp, wifi_ip_configuration_t *ip_cfg);
wifi_err_t R_WIFI_SX_ULPGN_GetIpAddress(wifi_ip_configuration_t *ip_cfg);
int32_t    R_WIFI_SX_ULPGN_CreateSocket(uint32_t type, uint32_t ip_version);
wifi_err_t R_WIFI_SX_ULPGN_ConnectSocket(uint8_t socket_number, uint32_t ip_address, uint16_t port, char *destination);
int32_t    R_WIFI_SX_ULPGN_SendSocket(uint8_t socket_number, uint8_t *data, uint32_t length, uint32_t timeout_ms);
int32_t    R_WIFI_SX_ULPGN_ReceiveSocket(uint8_t socket_number, uint8_t *data, uint32_t length, uint32_t timeout_ms);
wifi_err_t R_WIFI_SX_ULPGN_CloseSocket(uint8_t socket_number);

#endif
