/*
 *	ARP scanner.
 *	Copyright (C) 2017  Valdemar Lindberg
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef _ARPSC_H_
#define _ARPSC_H_ 1
#include "utility.h"

/**
 *	Get version of the program.
 *
 *	@Return non-null terminated string.
 */
extern const char *arpsc_version();

/**
 *	Start listenining onf ARP
 *	for each listening interfaces.
 *
 *	@Return non-zero if successfully.
 */
extern int arpsc_listening(iFaceAttr **ifaces, unsigned int numiface);

#endif
