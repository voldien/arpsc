#include "arpsc.h"
#include "utility.h"
#include <errno.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/select.h>
#include <time.h>

const char *arpsc_version(void) { return ARPSC_STR_VERSION; }

int arpsc_listening(iFaceAttr **ifaces, unsigned int numiface) {

	int i;	 /*	*/
	int ret; /*	*/
	int len; /*	*/

	/*	*/
	int ifaceindex; /*	*/
	long int tim;	/*	*/

	/*	*/
	short unsigned int hash[FD_SETSIZE]; /*	maps socket to ifaces.	*/
	ARPRESPPACK replay;					 /*	*/
	iFaceAttr *piface;					 /*	*/

	/*	*/
	unsigned int numfd = 0; /*	*/
	fd_set fd_read;			/*	*/
	fd_set fd_active;		/*	*/

	/*	*/
	FD_ZERO(&fd_read);	 /*	*/
	FD_ZERO(&fd_active); /*	*/

	/*	Setup iface lookup table with file descriptor.	*/
	arpsc_verbose_printf("%d interfaces.\n", numiface);
	memset(&hash, 0, sizeof(hash));
	for (i = 0; i < numiface; i++) {
		hash[ifaces[i]->sock] = i;
		FD_SET(ifaces[i]->sock, &fd_active);
		numfd = (ifaces[i]->sock + 1) > numfd ? (ifaces[i]->sock + 1) : numfd;
	}

	/*	Loop forever.	*/
	for (;;) {

		/*	Wait intill incoming response.	*/
		fd_read = fd_active;
		ret = select(numfd, &fd_read, NULL, NULL, NULL);

		if (ret < 0) {
			perror("select.\n");
			return EXIT_FAILURE;
		} else {
			for (i = 0; i < numfd; i++) {
				if (FD_ISSET(i, &fd_read)) {
					ifaceindex = hash[i];
					piface = ifaces[ifaceindex];

					/*	receive.	*/
					memset(&replay, 0, sizeof(replay));
					if ((len = recvfrom(i, &replay, sizeof(replay), 0, NULL, NULL)) < 0) {
						fprintf(stderr, "recvfrom, %s.\n", strerror(errno));
						continue;
					}

					/*	Get timestamp for when the package arrived.	*/
					tim = time(NULL);

					/*	Prevents the interface to fetch data when not active.	 */
					if ((piface->flag & 0x1) == 0 || len != sizeof(replay))
						continue;

					/*	Check if the packets is correct.	*/
					if (htons(replay.ethheader.ether_type) == ETHERTYPE_ARP && htons(replay.arp.ar_hrd) == 1 &&
						htons(replay.arp.ar_op) == ARPOP_REPLY && htons(replay.arp.ar_pro) == ETHERTYPE_IP) {

						/*	Print out response formated.	*/
						arpsc_print_arp_format(g_format, piface, &replay, tim);
					}
				}
			}
		}
	} /*	Main loop.*/

	return 0;
}
