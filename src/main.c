/**
    ARP scanner program.
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
#include"arpsc.h"
#include"utility.h"
#include<getopt.h>
#include<sys/socket.h>
#include<sys/select.h>
#include<pthread.h>
#include<signal.h>
#include<stdlib.h>

unsigned int g_numiface = 0;				/*	*/
iFaceAttr* g_pifaces = NULL;				/*	*/
char* g_format = "itsrMmh";					/*	*/
unsigned int g_listenonly = 0;				/*	*/

static void arpsc_release_ifaces(void){

	int i;
	iFaceAttr* piface;
	/*	Clean up code.	*/
	for(i = 0; i < g_numiface; i++){
		piface = &g_pifaces[i];
		arpsc_release_iface(piface);

	}
}

int main(int argc, const char** argv){

	/*	*/
	int i;
	int c;							/*	*/
	const char* shortopt = "vVl46s:S:i:f:";			/*	*/

	char** ifacelist = NULL;				/*	*/
	int listcount = 0;					/*	*/
	iFaceAttr* ifaces = NULL;				/*	*/

	/*	Long options.*/
	static const struct option longoption[] = {
		{"version",     no_argument, 		NULL,	'v'},	/*	*/
		{"verbose",     no_argument, 		NULL,	'V'},	/*	*/
		{"debug",	no_argument, 		NULL,	'd'},	/*	*/
		{"listen-only", no_argument, 		NULL,	'l'},	/*	*/
		{"ipv4",        no_argument, 		NULL,	'4'},	/*	*/
		{"ipv6",        no_argument, 		NULL,	'6'},	/*	*/
		{"sleep",       required_argument,	NULL,	's'},	/*	*/
		{"scan",        required_argument, 	NULL,	'S'},	/*	*/
		{"interface",   required_argument,	NULL,	'i'},	/*	*/
		{"format",      required_argument,	NULL,	'f'},	/*	*/
		{NULL, 0, NULL, 0}
	};


	/*	Iterate through each supported options.	*/
	while( (c = getopt_long(argc, (char *const *)argv, shortopt, longoption, NULL)) != EOF){
		switch(c){
		case 'v':
			printf("version %s\n", arpsc_version());
			exit(EXIT_SUCCESS);
			break;
		case 'V':
			g_verbose = 1;
			arpsc_verbose_printf("Enable verbose.\n");
			break;
		case 'd':
			g_debug = 1;
			break;
		case 'l':
			g_listenonly = 1;
			break;
		default:
			break;
		}
	}

	/*	Reset getopt.	*/
	optind = 0;
	optopt = 0;
	opterr = 0;

	/*	Check privilege level.	*/
	if( getuid() && geteuid()){
		fprintf(stderr, "Has to be execute as root.\n");
		return EXIT_FAILURE;
	}


	/*	*/
	while( (c = getopt_long(argc, (char *const *)argv, shortopt, longoption, NULL)) != EOF){
		switch(c){
		case 's':
			if(optarg){
				g_sleepmicro = strtof(optarg, NULL) * 1E6;
			}
			break;
		case 'S':

			break;
		case 'i':
			if(optarg){
				listcount = arpsc_parse_ifaces_cmd(optarg, &ifacelist);
				arpsc_verbose_printf("%d interface.\n", listcount);
			}
			break;
		case 'f':
			if(optarg){
				g_format = optarg;
			}
			break;
		case '4':
			/*	Enable IPv4.	*/
			break;
		case '6':
			/*	Enable IPv6.	*/
			break;
		default:
			break;
		}
	}

	/*	Make sure the resources gets released.	*/
	atexit(arpsc_release_ifaces);

	/*	Get interfaces.	*/
	g_numiface = arpsc_init_ifaces(ifacelist, &ifaces);
	if(g_numiface < 1){
		fprintf(stderr, "Failed creating interface for scanning.\n");
		return EXIT_FAILURE;
	}

	/*	Release resources.	*/
	for(i = 0; i < listcount; i++){
		free(ifacelist[i]);
	}
	free(ifacelist);

	/*	Main loop for listing to response.	*/
	return arpsc_listening(&ifaces, g_numiface);
}
