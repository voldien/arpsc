/**
	ARP scanner.
    Copyright (C) 2017  Valdemar Lindberg

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/
#ifndef _ARPSC_UTILITY_H_
#define _ARPSC_UTILITY_H_ 1
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <net/ethernet.h>
#include <net/if_arp.h>
#include <ifaddrs.h>
#include <pthread.h>

/**
 *	Global program variable states.
 */
extern unsigned int g_verbose;			/*	Verbose mode.	*/
extern unsigned int g_debug;			/*	*/
extern unsigned int g_sleepmicro;		/*	Sleep time inbetween ARP request broadcast.	*/
extern unsigned int g_listenonly;		/*	Listen only for arp response mode.	*/

/**
 *	Interface attributes.
 */
typedef struct arpsc_iface_attr_t{
	unsigned int ifaceindex;		/*	Physical interface index.	*/
	unsigned int ip;				/*	IPv4.	*/
	unsigned int ip6;				/*	IPv6.	*/
	char name[32];					/*	Name of the interface.	*/
	char mac[6];					/*	MAC address of the interface in binary.	*/
	unsigned int netmask;			/*	Interface subnet mask.	*/
	unsigned int numIPinsubnet;		/*	Number of unique IP.	*/
	int sock;						/*	ARP packet socket.	*/
	unsigned int flag;				/*	status.	*/
	pthread_t arpthread;			/*	ARP request thread.	*/
	/*sem_t* sem;*/					/*	*/
	/*void* mutex;	*/
}iFaceAttr;

/**
 *	ARP request packet.
 */
typedef struct arp_ipv4_request_packet_t{
	struct ether_header ethheader;		/*	Ethernet frame.	*/
	struct arphdr arp;					/*	ARP header.	*/
	uint8_t sender_mac[6];				/*	Sender's MAC address.	*/
	uint32_t sender_ip;					/*	Sender's IP address.	*/
	uint8_t target_mac[6];				/*	Broadcast MAC address.	*/
	uint32_t target_ip;					/*	The seeking IP address.	*/
} __attribute__ ((__packed__)) ARPREQPACK;

/**
 *	ARP response packet.
 */
typedef struct arp_ipv4_response_packet_t{
	struct ether_header ethheader;		/*	Ethernet frame.	*/
	struct arphdr arp;					/*	ARP header.	*/
	uint8_t sender_mac[6];				/*	*/
	uint32_t sender_ip;					/*	*/
	uint8_t target_mac[6];				/*	*/
	uint32_t target_ip;					/*	*/
	/*uint8_t padding[18];*/			/*	*/
} __attribute__ ((__packed__)) ARPRESPPACK;

/**
 *
 */
extern iFaceAttr* piface;				/*	*/
extern char* g_format;					/*	*/

/**
 *	Initialize interfaces on the device.
 *
 *	\ifacearr sequence of interface with a comma
 *	seperating them.
 *
 *	\pifaces [out] list of interface attributes for each
 *	interface device specified by \ifacearr\.
 *
 *	@Return non-zero if successfully.
 */
extern int arpsc_init_ifaces(char** __restrict__ ifacearr, iFaceAttr** __restrict__ pifaces);

/**
 *	Initialize iFaceAttr structure.
 *	@Return non zero if successfully.
 */
extern int arpsc_init_iface_attrb(const struct ifaddrs* __restrict__ tmp,
		iFaceAttr* __restrict__ attr);

/**
 *	Release resources associated
 *	with the interface.
 */
extern void arpsc_release_iface(iFaceAttr* iface);

/**
 *	Parse iface select input.
 *
 *	@Return number of ifaces if succesfully, otherwise 0.
 */
extern int arpsc_parse_ifaces_cmd(const char* p, char*** arr);

/**
 *	Create IPv4 DNS query.
 *
 *	\ipv4
 *
 *	\host
 *
 *	\len
 */
extern void arpsc_get_host_ip_v4(unsigned int ipv4, char* host, unsigned int len);

/**
 *	Create IPv6 DNS query.
 *
 *	\ipv6
 *
 *	\host
 *
 *	\len
 */
extern void arpsc_get_host_ip_v6(unsigned int ipv6, char* host, unsigned int len);

/**
 *	Thread function for broadcasting ARP request.
 *
 *	@Return NULL if exit succesfully.
 */
extern void* arpsc_arp_thread_func(void* p);

/**
 *	Print ARP response formated.
 *
 *	\format
 *
 *	\iface
 *
 *	\replay
 *
 *	\time timestamp
 *
 */
extern void arpsc_print_arp_format(const char* __restrict__ format,
		iFaceAttr* __restrict__ iface, ARPRESPPACK* __restrict__ replay,
		unsigned int time);

/**
 *	Verbose print format.
 *
 *	@Return
 */
extern int arpsc_verbose_printf(const char* __restrict__ fmt,...);

#endif
