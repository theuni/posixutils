/*
 * Copyright 2004-2006 Jeff Garzik <jgarzik@pobox.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */


#ifndef HAVE_CONFIG_H
#error missing autoconf-generated config.h.
#endif
#include "posixutils-config.h"

#include <libgen.h>
#include <stdio.h>
#include <string>
#include <libpu.h>


int main (int argc, char *argv[])
{
	pu_init();

	if (argc != 2) {
		fprintf(stderr, _("invalid number of arguments\n"));
		return 1;
	}

	std::string pathbuf(argv[1]);
	std::string dn(dirname(&pathbuf[0]));

	printf("%s\n", dn.c_str());

	return 0;
}

