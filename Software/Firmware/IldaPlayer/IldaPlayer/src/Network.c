/*
 Network.c
 Ethernet Control

 Copyright 2020 Scrootch.me!

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include "diag/Trace.h"

#include "Network.h"

#include "lwip/tcpip.h"
#include "lwip/netif.h"
#include "lwip/ip_addr.h"
#include "lwip/dhcp.h"
#include "ethernetif.h"
#include "httpserver-socket.h"
#include "stm32f7xx_hal.h"

#define USE_DHCP       /* enable DHCP, if disabled static address is used*/

/*Static IP ADDRESS*/
#define IP_ADDR0   192
#define IP_ADDR1   168
#define IP_ADDR2   1
#define IP_ADDR3   100

/*NETMASK*/
#define NETMASK_ADDR0   255
#define NETMASK_ADDR1   255
#define NETMASK_ADDR2   255
#define NETMASK_ADDR3   0

/*Gateway Address*/
#define GW_ADDR0   192
#define GW_ADDR1   168
#define GW_ADDR2   1
#define GW_ADDR3   10

#define DHCP_OFF                   (uint8_t) 0
#define DHCP_START                 (uint8_t) 1
#define DHCP_WAIT_ADDRESS          (uint8_t) 2
#define DHCP_ADDRESS_ASSIGNED      (uint8_t) 3
#define DHCP_TIMEOUT               (uint8_t) 4
#define DHCP_LINK_DOWN             (uint8_t) 5

#ifdef USE_DHCP
#define MAX_DHCP_TRIES  4
__IO uint8_t DHCP_state = DHCP_OFF;
#endif

static struct netif gnetif;	// Our interface info/status

static void Netif_Config(void);
static void DHCP_thread(void const *argument);

void network_Init()
{
	tcpip_init(NULL, NULL);
	Netif_Config();
	http_server_socket_init();
}

uint8_t network_GetIP (char *outstr)
{
	if (netif_is_up(&gnetif))
	{
		ip4addr_ntoa_r(&(gnetif.ip_addr), outstr, IP4ADDR_STRLEN_MAX);
		return 1;
	}

	return 0;
}

static void Netif_Config(void)
{
	ip_addr_t ipaddr;
	ip_addr_t netmask;
	ip_addr_t gw;

#ifdef USE_DHCP
	ip_addr_set_zero_ip4(&ipaddr);
	ip_addr_set_zero_ip4(&netmask);
	ip_addr_set_zero_ip4(&gw);
#else
  IP_ADDR4(&ipaddr,IP_ADDR0,IP_ADDR1,IP_ADDR2,IP_ADDR3);
  IP_ADDR4(&netmask,NETMASK_ADDR0,NETMASK_ADDR1,NETMASK_ADDR2,NETMASK_ADDR3);
  IP_ADDR4(&gw,GW_ADDR0,GW_ADDR1,GW_ADDR2,GW_ADDR3);
#endif /* USE_DHCP */

	netif_add(&gnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init,
			&tcpip_input);

	/*  Registers the default network interface. */
	netif_set_default(&gnetif);

	if (netif_is_link_up(&gnetif))
	{
		/* When the netif is fully configured this function must be called.*/
		netif_set_up(&gnetif);
	}
	else
	{
		/* When the netif link is down this function must be called */
		netif_set_down(&gnetif);
	}

#ifdef USE_DHCP
	/* Start DHCPClient */
	osThreadDef(DHCP, DHCP_thread, osPriorityBelowNormal, 0,
			configMINIMAL_STACK_SIZE * 2);
	osThreadCreate(osThread(DHCP), &gnetif);

	if (netif_is_up(&gnetif)) DHCP_state = DHCP_START;
#endif
}

#ifdef USE_DHCP
/**
 * @brief  DHCP Process
 * @param  argument: network interface
 * @retval None
 */
static void DHCP_thread(void const *argument)
{
	struct netif *netif = (struct netif*) argument;
	ip_addr_t ipaddr;
	ip_addr_t netmask;
	ip_addr_t gw;
	struct dhcp *dhcp;
	uint8_t iptxt[20];

	for (;;)
	{
		switch (DHCP_state)
		{
		case DHCP_START :
		{
			ip_addr_set_zero_ip4(&netif->ip_addr);
			ip_addr_set_zero_ip4(&netif->netmask);
			ip_addr_set_zero_ip4(&netif->gw);
			dhcp_start(netif);
			DHCP_state = DHCP_WAIT_ADDRESS;
			trace_puts("State: Looking for DHCP server ...");
		}
			break;

		case DHCP_WAIT_ADDRESS :
		{
			if (dhcp_supplied_address(netif))
			{
				DHCP_state = DHCP_ADDRESS_ASSIGNED;

				sprintf((char*) iptxt, "%s",
						ip4addr_ntoa((const ip4_addr_t*) &netif->ip_addr));
				trace_printf("IP address assigned by a DHCP server: %s\n",
						iptxt);
			}
			else
			{
				dhcp = (struct dhcp*) netif_get_client_data(netif,
						LWIP_NETIF_CLIENT_DATA_INDEX_DHCP);

				/* DHCP timeout */
				if (dhcp->tries > MAX_DHCP_TRIES)
				{
					DHCP_state = DHCP_TIMEOUT;

					/* Stop DHCP */
					dhcp_stop(netif);

					/* Static address used */
					IP_ADDR4(&ipaddr, IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
					IP_ADDR4(&netmask, NETMASK_ADDR0, NETMASK_ADDR1,
							NETMASK_ADDR2, NETMASK_ADDR3);
					IP_ADDR4(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
					netif_set_addr(netif, ip_2_ip4(&ipaddr), ip_2_ip4(&netmask),
							ip_2_ip4(&gw));

					sprintf((char*) iptxt, "%s",
							ip4addr_ntoa((const ip4_addr_t*) &netif->ip_addr));
					trace_puts("DHCP Timeout !!");
					trace_printf("Static IP address: %s\n", iptxt);
				}
			}
		}
			break;
		case DHCP_LINK_DOWN :
		{
			/* Stop DHCP */
			dhcp_stop(netif);
			DHCP_state = DHCP_OFF;
		}
			break;
		default:
			break;
		}

		/* wait 250 ms */
		osDelay(250);
	}
}
#endif  /* USE_DHCP */
