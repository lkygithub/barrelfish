/** \file
 *  \brief Hello World application
 */

/*
 * Copyright (c) 2010, 2011, 2012, ETH Zurich.
 * All rights reserved.
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * ETH Zurich D-INFK, Haldeneggsteig 4, CH-8092 Zurich. Attn: Systems Group.
 */

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) 
{
	if (argc < 2) printf("missing argument.\n");
	while (1) {
		printf("test_sched: %s\n", argv[1]);
	}
	return EXIT_SUCCESS;
}
