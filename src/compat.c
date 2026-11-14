
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <tx_api.h>

#include <nx_api.h>
#include <nx_ip.h>
#include <nxd_dns.h>
#include <nx_system.h>

#include <r_wifi_sx_ulpgn_if.h>

/* Compatibility functions between NetX and the Wi-Fi driver. */


int32_t az_platform_clock_msec(int64_t* out_clock_msec)
{
    if (out_clock_msec == NULL)
    {
        return -1;
    }

    *out_clock_msec = ((int64_t)tx_time_get() * 1000LL) / (int64_t)TX_TIMER_TICKS_PER_SECOND;
    return 0;
}


int32_t az_platform_sleep_msec(int32_t milliseconds)
{
    if (milliseconds > 0)
    {
        ULONG ticks = (ULONG)(((uint64_t)milliseconds * TX_TIMER_TICKS_PER_SECOND + 999ULL) / 1000ULL);
        if (ticks == 0)
        {
            ticks = 1;
        }
        tx_thread_sleep(ticks);
    }
    return 0;
}

#if 0
UINT  _nxde_dns_host_by_name_get(NX_DNS *dns_ptr, UCHAR *host_name, NXD_ADDRESS *host_address_ptr,
                                 ULONG wait_option, UINT lookup_type)
{
    wifi_err_t wifi_err;
    uint32_t ip_addr;

    wifi_err = R_WIFI_SX_ULPGN_DnsQuery(host_name, &ip_addr);

    if(wifi_err != WIFI_SUCCESS) {
        return NX_DNS_QUERY_FAILED;
    }

    host_address_ptr -> nxd_ip_version = NX_IP_VERSION_V4;
    host_address_ptr -> nxd_ip_address.v4 = ip_addr;

    return NX_SUCCESS;
}
#endif
