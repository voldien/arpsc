#include"utility.h"

#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <net/if.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netpacket/packet.h>
#include <net/if_arp.h>
#include <errno.h>
#include <sys/select.h>
#include <signal.h>

unsigned int g_verbose = 0;
unsigned int g_debug = 0;
unsigned int g_sleepmicro = 1000;

static int arpsc_cmp_ifaces(const char** __restrict__ ifacearr,
        const char* __restrict__ iface) {

	char* cur;

	/*	*/
	if(ifacearr == NULL){
		return 1;
	}

	/*	*/
	cur = *ifacearr++;
	while(cur){
		if(strcmp(cur, iface) == 0){
			return 1;
		}
		cur = *ifacearr++;
	}

	return 0;
}

int arpsc_init_ifaces(char** ifacearr, iFaceAttr** pifaces){

	struct ifaddrs* addrs = NULL;			/*	*/
	struct ifaddrs* tmp = NULL;				/*	*/
	int numiface = 0;						/*	*/
	iFaceAttr* ifaces = NULL;				/*	*/
	iFaceAttr* iface;						/*	*/
	pthread_attr_t pattr;					/*	*/
	struct sched_param schparam;			/*	*/
	int perr = 0;							/*	*/

	/*	Get interfaces.	*/
	if( getifaddrs(&addrs) < 0){
		fprintf(stderr, "%s.\n", strerror(errno));
		return 0;
	}

	/*	Iterate through all interface on the device.	*/
	tmp = addrs;
	do{

		/*	Device needs an address and be part of packet family.	*/
		if(tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_PACKET){

			/*	Exclude localhost.	*/
			if(strcmp(tmp->ifa_name, "lo") != 0){

				/*	Check if interface name is specified.	*/
				if(!arpsc_cmp_ifaces((const char**)ifacearr, tmp->ifa_name)){
					goto next;
				}

				/*	Allocate interface for next interface.	*/
				*pifaces = realloc(*pifaces, sizeof(iFaceAttr) * (numiface + 1));
				ifaces = *pifaces;
				memset(&ifaces[numiface], 0, sizeof(iFaceAttr));

				/*	Get interface attributes.	*/
				iface = &ifaces[numiface];
				if(arpsc_init_iface_attrb(tmp, iface)){
					iface->sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
					if(iface->sock < 0){
						fprintf(stderr, "Failed to create raw packet socket, %s.\n", strerror(errno));
						return 0;
					}

					/*	Init the pthread attribute.	*/
					if(pthread_attr_init(&pattr) != 0){
						fprintf(stderr, "%s.\n", strerror(errno));
					}

					/*	Set schedular attribute.	*/
					schparam.__sched_priority = 0;
					perr = pthread_attr_setschedparam(&pattr, &schparam);
					if(perr != 0){
						fprintf(stderr, "%s.\n",  strerror(perr));
					}

					/*	Create broadcast thread.	*/
					if(!g_listenonly){
						perr = pthread_create(&iface->arpthread, &pattr, arpsc_arp_thread_func, iface);
						if( perr == 0){
							arpsc_verbose_printf("Created thread for %s interface.\n", ifaces->name);
						}

						if(pthread_attr_destroy(&pattr) != 0){
							fprintf(stderr, "%s.\n", strerror(errno));
						}
					}

					/*	Update number of interface is everything succeeded.	*/
					numiface++;
				}
				else{
					/*	Failed to init interface. cleaning up.	*/
					memset(&ifaces[numiface], 0, sizeof(ifaces[0]));
				}
			}
		}

		next:
		tmp = tmp->ifa_next;
	}while(tmp);

	/*	Free	*/
	freeifaddrs(addrs);

	return numiface;
}

int arpsc_init_iface_attrb(const struct ifaddrs* tmp, iFaceAttr* attr){

	int tmpsock;                /*	*/
	struct ifreq ifr;           /*	*/
	unsigned int* netmask;      /*	*/

	/*	Temporarily socket in order to get information about interface.	*/
	tmpsock = socket(AF_INET, SOCK_STREAM, 0);
	if( tmpsock < 0 ){
		fprintf(stderr, "Failed to create socket, %s.\n", strerror(errno));
		return 0;
	}

	/*	Copy interface name.	*/
	strcat(attr->name, tmp->ifa_name);
	arpsc_verbose_printf("Initializing %s interface attribute.\n", tmp->ifa_name);

	/*	Get interface index.	*/
	memcpy(ifr.ifr_ifrn.ifrn_name, attr->name, IF_NAMESIZE);
	if( ioctl(tmpsock, SIOCGIFINDEX, &ifr) < 0 ){
		close(tmpsock);
		fprintf(stderr, "%s.\n", strerror(errno));
		return 0;
	}
	attr->ifaceindex = ifr.ifr_ifindex;

	/*	Get interface network netmask.	*/
	if (ioctl(tmpsock, SIOCGIFNETMASK, &ifr) < 0) {
		close(tmpsock);
		printf("%s: Could not get the subnet mask. Errorcode : %d %s\n", attr->name, errno,
				strerror(errno));
		return 0;
	}
	netmask = (unsigned int*)(&ifr.ifr_ifru.ifru_netmask.sa_data[2]);
	attr->netmask = htonl(*netmask);
	attr->numIPinsubnet = ( ~attr->netmask ) - 2;

	/*	Get interface IP address.	*/
	ifr.ifr_addr.sa_family = AF_INET;
	memcpy(ifr.ifr_ifrn.ifrn_name, attr->name, IF_NAMESIZE);
	if( ioctl(tmpsock, SIOCGIFADDR, &ifr) < 0 ){
		close(tmpsock);
		fprintf(stderr, "%s.\n", strerror(errno));
		return 0;
	}
	attr->ip = ( (struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr;

	/*	Get interface's hardware address (MAC).	*/
	if( ioctl(tmpsock, SIOCGIFHWADDR, &ifr, sizeof(ifr)) < 0 ){
		close(tmpsock);
		fprintf(stderr, "%s.\n", strerror(errno));
		return 0;
	}

	/*	Copy MAC address.	*/
	const unsigned char* mac = (const unsigned char*)ifr.ifr_hwaddr.sa_data;
	memcpy(attr->mac, mac, sizeof(attr->mac));

	/*	*/
	close(tmpsock);
	return 1;
}

void arpsc_release_iface(iFaceAttr* iface){

	if(iface->sock > 0){
		if(iface->arpthread){
			pthread_kill(iface->arpthread, SIGUSR1);	/*	Inform the thread to terminate.	*/
			pthread_join(iface->arpthread, NULL);
		}

		close(iface->sock);
		memset(iface, 0, sizeof(iFaceAttr));
	}
}

int arpsc_parse_ifaces_cmd(const char* p, char*** arr){

	const char* tmpstr;	/*	*/
	char* comstr;		/*	*/
	int len;			/*	*/
	int numifaces;		/*	*/

	tmpstr = p;
	numifaces = 0;

	/*	Iterate through each comma.	*/
	do{
		numifaces++;
		*arr = realloc(*arr, (numifaces + 1) * sizeof(char**));
		comstr = strchr(tmpstr, ',');

		/*	Compute the length of the attribute value.	*/
		if(comstr == NULL)
			len = strlen(tmpstr);
		else
			len = comstr - tmpstr;

		/*	Extract interface name.	*/
		(*arr)[numifaces - 1] = malloc(len + 1);
		memcpy(( *arr)[numifaces - 1], tmpstr, len);
		(*arr)[numifaces - 1][len] = '\0';

		/*	*/
		tmpstr = comstr;
		tmpstr++;
	}while(comstr);

	/*	*/
	(*arr)[numifaces] = NULL;

	return numifaces;
}

void arpsc_get_host_ip_v4(unsigned int ipv4, char* host, unsigned int len){

	struct sockaddr_in sa;

	/*	*/
	sa.sin_addr.s_addr = ipv4;
	sa.sin_family = AF_INET;
	if( getnameinfo((struct sockaddr *)&sa, sizeof(sa), host, len, NULL, 0, 0) < 0){
		fprintf(stderr, "%s.\n", strerror(errno));
	}

}

void arpsc_get_host_ip_v6(unsigned int ipv6, char* host, unsigned int len){
	struct sockaddr_in sa;

	/*	*/
	sa.sin_addr.s_addr = ipv6;
	sa.sin_family = AF_INET6;
	if( getnameinfo((struct sockaddr *)&sa, sizeof(sa), host, len, NULL, 0, 0) < 0){
		fprintf(stderr, "%s.\n", strerror(errno));
	}
}

void* arpsc_arp_thread_func(void* p){

	unsigned int i;				/*	*/
	int arp_fd;					/*	*/
	ARPREQPACK arppack;			/*	*/
	struct sockaddr_ll sa;		/*	*/
	iFaceAttr* iface = (iFaceAttr*)p;

	const int hwethlen = 6;
	const int hwiplen = 4;

	/*	Cache iface	socket.	*/
	arp_fd = iface->sock;

	/* ARP Header	*/
	arppack.arp.ar_hrd = htons(ETH_P_802_3);
	arppack.arp.ar_pro = htons(ETHERTYPE_IP);
	arppack.arp.ar_hln = hwethlen;
	arppack.arp.ar_pln = hwiplen;
	arppack.arp.ar_op = htons(ARPOP_REQUEST);
	memcpy(arppack.sender_mac, iface->mac, sizeof(iface->mac));
	memset(arppack.target_mac, 0 , (6 * sizeof(uint8_t)));

	/*	Ethernet frame.	*/
	arppack.ethheader.ether_type = htons(ETHERTYPE_ARP);
	memcpy(arppack.ethheader.ether_shost, iface->mac, sizeof(iface->mac));
	memset(arppack.ethheader.ether_dhost, 0xffffffff, sizeof(arppack.ethheader.ether_dhost));

	/*	Address family.	*/
	bzero(&sa, sizeof(sa));
	sa.sll_family = AF_PACKET;
	sa.sll_ifindex = iface->ifaceindex;
	sa.sll_protocol = htons(ETH_P_ARP);

	/*	TODO Use signal or something for checking the life the thread.	*/
	while(1){

		iface->flag = 1;
		for(i = 1; i < iface->numIPinsubnet; i++){

			/*	*/
			arppack.sender_ip = (iface->ip);
			arppack.target_ip = htonl( ( (htonl(iface->ip) & iface->netmask)) + i);
			if( sendto(arp_fd, &arppack, sizeof(arppack), 0, (struct sockaddr*)&sa, sizeof(sa)) < 0){
				fprintf(stderr, "sendto failed, %s.\n", strerror(errno));
				return NULL;
			}
			usleep(1000);
		}

		/*	Sleep and disable listening on this interface.	*/
		iface->flag = 0;
		usleep(g_sleepmicro);

	}

	return NULL;
}

void arpsc_print_arp_format(const char* __restrict__ format,
        iFaceAttr* __restrict__ iface, ARPRESPPACK* __restrict__ replay,
        unsigned int time) {

	struct in_addr senderip;			/*	*/
	struct in_addr recvip;				/*	*/

	char host[128];						/*	*/
	char recip[32];						/*	*/
	char senip[32];						/*	*/

	char* tmpformatstr;
	char format;
	char endof = ' ';

	tmpformatstr = (char*)format;
	format = *tmpformatstr++;

	/*	Iterate through each print format string option.	*/
	do{
		switch(format){
		case 'i':
			printf("iface='%s'%c", iface->name, endof);
			break;
		case 't':
			printf("time='%d'%c", time, endof);
			break;
		case 's':
			senderip.s_addr = (in_addr_t)replay->sender_ip;
			memset(senip, '\0', sizeof(senip));
			strcat(senip, inet_ntoa(senderip));
			printf("sip='%s'%c", senip, endof);
			/*			*/
			break;
		case 'r':
			recvip.s_addr = (in_addr_t)replay->target_ip;
			memset(recip, '\0', sizeof(senip));
			strcat(recip, inet_ntoa(recvip));
			printf("tip='%s'%c", recip, endof);
			/*			*/
			break;
		case 'M':
			printf("smac='%x:%x:%x:%x:%x:%x'%c",
			replay->sender_mac[0],
			replay->sender_mac[1],
			replay->sender_mac[2],
			replay->sender_mac[3],
			replay->sender_mac[4],
			replay->sender_mac[5], endof);
			break;
		case 'm':
			printf("tmac='%x:%x:%x:%x:%x:%x'%c",
			replay->target_mac[0],
			replay->target_mac[1],
			replay->target_mac[2],
			replay->target_mac[3],
			replay->target_mac[4],
			replay->target_mac[5], endof);
			break;
		case 'h':
			/*	*/
			arpsc_get_host_ip_v4(replay->sender_ip, host, sizeof(host));
			printf("hostname='%s'%c", host, endof);
			break;
		default:
			break;
		}

		format = *tmpformatstr++;
	}while(format != '\0');

	/*	*/
	fprintf(stdout, "\n");
}

int arpsc_verbose_printf(const char* __restrict__ fmt,...){

	va_list va;
	int ret = 0;

	if(g_verbose){
		va_start(va,fmt);
		ret = vprintf(fmt, va);
		va_end(va);
	}

	return ret;
}

