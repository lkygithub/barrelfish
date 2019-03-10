/**
 * \file
 * \brief Provides a generic startup function for the ARM OMAP platform
 */
/*
 * Copyright (c) 2013, ETH Zurich.
 * Copyright (c) 2015, Hewlett Packard Enterprise Development LP.
 * All rights reserved.
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * ETH Zurich D-INFK, Universitaetstr. 6, CH-8092 Zurich. Attn: Systems Group.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <barrelfish/barrelfish.h>
#include <barrelfish/spawn_client.h>
#include <barrelfish_kpi/platform.h>
#include <if/monitor_blocking_defs.h>
#include <maps/zynqmp_map.h>

#include "kaluga.h"

struct allowed_registers
{
    char* binary;
    lpaddr_t registers[][2];
};

static struct allowed_registers zynqmp_gem = {
    .binary = "zynqmp_gem",
    .registers =
    {
        {ZYNQMP_GEM4_BASEADDR, 0x1000},
        {0x0, 0x0}
    }
};

static struct allowed_registers* zynqmp[] = {
    &zynqmp_gem,
    NULL,
};

/**
 * \brief Startup function for ARMv7 drivers.
 *
 * Makes sure we get the device register capabilities.
 */
errval_t
default_start_function(coreid_t where, struct module_info* driver,
        char* record, struct driver_argument *placeholder)
{
    assert(driver != NULL);
    assert(record != NULL);

    errval_t err;

    struct monitor_blocking_binding *m=
        get_monitor_blocking_binding();
    assert(m != NULL);

    uint32_t arch, platform;
    err = m->rpc_tx_vtbl.get_platform(m, &arch, &platform);
    assert(err_is_ok(err));
    assert(arch == PI_ARCH_ARMV8A);

    struct allowed_registers **regs= NULL;
    switch(platform) {
        case PI_PLATFORM_ZYNQMP:
            regs= zynqmp;
            break;
        default:
            printf("Unrecognised ARMv8 platform\n");
            abort();
    }

    // TODO Request the right set of caps and put in device_range_cap
    struct cnoderef dev_cnode;
    struct capref dev_cnode_cap;
    err = cnode_create_l2(&dev_cnode_cap, &dev_cnode);
    assert(err_is_ok(err));

    struct capref device_cap;
    device_cap.cnode = dev_cnode;
    device_cap.slot = 0;

    char* name;
    err = oct_read(record, "%s", &name);
    assert(err_is_ok(err));
    KALUGA_DEBUG("%s:%d: Starting driver for %s\n", __FUNCTION__, __LINE__, name);
    for (size_t i=0; regs[i] != NULL; i++) {

        if(strcmp(name, regs[i]->binary) != 0) {
            continue;
        }

        // Get the device cap from the managed capability tree
        // put them all in a single cnode
        for (size_t j=0; regs[i]->registers[j][0] != 0x0; j++) {
            struct capref device_frame;
            KALUGA_DEBUG("%s:%d: mapping 0x%"PRIxLPADDR" %"PRIuLPADDR"\n", __FUNCTION__, __LINE__,
                   regs[i]->registers[j][0], regs[i]->registers[j][1]);

            lpaddr_t base = regs[i]->registers[j][0] & ~(BASE_PAGE_SIZE-1);
            err = get_device_cap(base,
                                 regs[i]->registers[j][1],
                                 &device_frame);
            assert(err_is_ok(err));

            err = cap_copy(device_cap, device_frame);
            assert(err_is_ok(err));
            device_cap.slot++;
        }
    }
    free(name);

    err = spawn_program_with_caps(where, driver->path, driver->argv, environ,
            NULL_CAP, dev_cnode_cap, 0, driver->did);
    if (err_is_fail(err)) {
        DEBUG_ERR(err, "Spawning %s failed.", driver->path);
        return err;
    }

    return SYS_ERR_OK;
}


errval_t start_networking(coreid_t core,
                          struct module_info* driver,
                          char* record, struct driver_argument *placeholder)
{
    assert(driver != NULL);
    errval_t err = SYS_ERR_OK;


    if (is_started(driver)) {
        printf("Already started %s\n", driver->binary);
        return KALUGA_ERR_DRIVER_ALREADY_STARTED;
    }

    if (!is_auto_driver(driver)) {
        printf("Not auto %s\n", driver->binary);
        return KALUGA_ERR_DRIVER_NOT_AUTO;
    }


    if (!(strcmp(driver->binary, "net_sockets_server") == 0)) {
        
        driver->allow_multi = 1;
        /*uint64_t vendor_id, device_id, bus, dev, fun;
        err = oct_read(record, "_ { bus: %d, device: %d, function: %d, vendor: %d, device_id: %d }",
                       &bus, &dev, &fun, &vendor_id, &device_id);

        char* pci_arg_str = malloc(26);
        snprintf(pci_arg_str, 26, "%04"PRIx64":%04"PRIx64":%04"PRIx64":%04"
                        PRIx64":%04"PRIx64, vendor_id, device_id, bus, dev, fun);*/

        err = default_start_function(core, driver, record, placeholder);
        if (err_is_fail(err)) {
            DEBUG_ERR(err, "Spawning %s failed.", driver->path);
            return err;
        }

        // cards with driver in seperate process
        struct module_info* net_sockets = find_module("net_sockets_server");
        if (net_sockets == NULL) {
            printf("Net sockets server not found\n");
            return KALUGA_ERR_DRIVER_NOT_AUTO;
        }

        // Spawn net_sockets_server
        net_sockets->argv[0] = "net_sockets_server";
        net_sockets->argv[1] = "auto";
        net_sockets->argv[2] = driver->binary;
        net_sockets->argv[3] = NULL;

        err = spawn_program(core, net_sockets->path, net_sockets->argv, environ, 0,
                            get_did_ptr(net_sockets));
        //free (pci_arg_str);
    } else {
        //driver->allow_multi = 1;
        // TODO currently only for mxl4, might be other cards that 
        // start the driver by creating a queue
        /*if (!(driver->argc > 2)) {
            driver->argv[driver->argc] = "mlx4";        
            driver->argc++;
            driver->argv[driver->argc] = NULL;
        }*/

        // All cards that start the driver by creating a device queue
        err = default_start_function(core, driver, record, placeholder);
    }

    return err;
}

/* place holder function for armv8 without PCI 
 * should be implemented if PCI is applied
 */
errval_t
default_start_function_new(coreid_t where, struct module_info* mi, char* record,
                           struct driver_argument* arg)
{
    return SYS_ERR_OK;
}

errval_t start_networking_new(coreid_t where,
                              struct module_info* driver,
                              char* record, struct driver_argument * int_arg)
{
    return SYS_ERR_OK;
}