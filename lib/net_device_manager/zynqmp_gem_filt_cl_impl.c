/*
 * Copyright (c) 2007-12 ETH Zurich.
 * All rights reserved.
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * ETH Zurich D-INFK, Haldeneggsteig 4, CH-8092 Zurich. Attn: Systems Group.
 */

#include <stdio.h>
#include <string.h>

#include <barrelfish/barrelfish.h>
#include <barrelfish/waitset.h>
#include <barrelfish/nameservice_client.h>
#include <lwip/inet.h>

#include <if/zynqmp_gem_devif_defs.h>

#include "port_management_support.h"
#include "device_manager_debug.h"



/******************************************************************************
 * Local state
 ******************************************************************************/

/** Connection to zynqmp_gem management service */
struct zynqmp_gem_devif_binding *zynqmp_gem_binding = NULL;

/******************************************************************************
 * Operations for filter interface
 ******************************************************************************/

// Callback for bind
static void bind_cb(void *st, errval_t err, struct zynqmp_gem_devif_binding *b)
{
    assert(err_is_ok(err));

    NDM_DEBUG("Sucessfully connected to management interface\n");

/*
    zynqmp_gem_binding = b;
    zynqmp_gem_rpc_client_init(zynqmp_gem_binding);
*/
}

/** Open connection to management interface */
static void connect_to_mngif(char *dev_name)
{
    return ;
    errval_t r;
    iref_t iref;
    const char *suffix = "_devif";
    char name[strlen(dev_name) + strlen(suffix) + 1];

    // Build label for management service
    sprintf(name, "%s%s", dev_name, suffix);

    // Connect to service
    r = nameservice_blocking_lookup(name, &iref);
    assert(err_is_ok(r));

    r = zynqmp_gem_devif_bind(iref, bind_cb, NULL, get_default_waitset(),
            IDC_BIND_FLAGS_DEFAULT);
    assert(err_is_ok(r));
}

/******************************************************************************
 * Operations for filter interface
 ******************************************************************************/

static void init_filters(char *dev_name, qid_t qid)
{
    NDM_DEBUG("zynqmp_gem_flt: init %s %d\n", dev_name, (int) qid);
    return ;

    // Check if we are already initialized from another queue
    if (zynqmp_gem_binding != NULL) {
        return;
    }

    connect_to_mngif(dev_name);

    // waiting for connection to succeed.
    NDM_DEBUG("zynqmp_gem_init_filters: wait connection\n");
    while (zynqmp_gem_binding == NULL) {
        messages_wait_and_handle_next();
    }
}

static void reg_arp_filters(uint64_t id, uint64_t len_rx,
                            uint64_t len_tx)
{
    USER_PANIC("reg_arp_filters() not supported yet in zynqmp_gem filters");
}

static errval_t reg_filters(uint16_t port,
                            port_type_t type,
                            bufid_t buffer_id_rx,
                            bufid_t buffer_id_tx,
                            appid_t appid,
                            qid_t qid,
                            uint64_t *id, errval_t *rerr, uint64_t *filter_id)
{
    NDM_DEBUG("my checkp reg filters\n");
    *filter_id = 0;
    *rerr = SYS_ERR_OK;
    return SYS_ERR_OK;
    /*
    zynqmp_gem_devif_port_type_t t;
    assert(zynqmp_gem_binding != NULL);

    NDM_DEBUG("zynqmp_gem_reg_filters()\n");

    if (type == net_ports_PORT_TCP) {
        t = zynqmp_gem_PORT_TCP;
    } else {
        t = zynqmp_gem_PORT_UDP;
    }
    errval_t err;
    err = zynqmp_gem_binding->rpc_tx_vtbl.register_port_filter(zynqmp_gem_binding, buffer_id_rx, buffer_id_tx, qid, t, port, rerr, filter_id);

    return SYS_ERR_OK;
    */
}

static errval_t unreg_filters(uint64_t filter_id, qid_t qid)
{
    NDM_DEBUG("my checkp unreg filters\n");
    return SYS_ERR_OK;
    /*
    assert(zynqmp_gem_binding != NULL);

    NDM_DEBUG("zynqmp_gem_unreg_filters()\n");
    errval_t err, rerr;
    err = zynqmp_gem_binding->rpc_tx_vtbl.unregister_filter(zynqmp_gem_binding, filter_id, &rerr);
    assert(err_is_ok(err));
    
    return rerr;
    */
}


/******************************************************************************
 * Get signature of this service
 ******************************************************************************/

static struct filters_tx_vtbl zynqmp_gem_filts_mng = {
    .type = "zynqmp_gem_filters",
    .init_filters = init_filters,
    .reg_arp_filters = reg_arp_filters,
    .reg_filters = reg_filters,
    .unreg_filters = unreg_filters,
};

struct filters_tx_vtbl *get_zynqmp_gem_filt_mng_sign(void)
{
    return &zynqmp_gem_filts_mng;
}
