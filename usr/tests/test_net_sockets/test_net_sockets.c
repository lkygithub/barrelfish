/**
 * \file
 * \brief TFTP library
 */

/*
 * Copyright (c) 2015 ETH Zurich.
 * All rights reserved.
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * ETH Zurich D-INFK, Universitaetsstrasse 6, CH-8092 Zurich. Attn: Systems Group.
 */

#include <stdlib.h>
#include <stdio.h>

#include <barrelfish/barrelfish.h>
#include <barrelfish/waitset.h>
#include <barrelfish/nameservice_client.h>

#include <barrelfish/deferred.h>
#include <net_sockets/net_sockets.h>
#include <arpa/inet.h>

#ifdef CLIENT
#undef CLIENT
#endif
#define CLIENT
struct in_addr dest_ip;
uint16_t udp_port = 7;
struct net_socket *pcb;
void * payload;
int echo_counter = 10;

static errval_t send_message(struct net_socket *socket, char* message, struct in_addr addr, uint16_t port)
{
    errval_t err;
    err = net_send_to(socket, message, strlen(message) + 1, addr, port);
    return err;
}

static void recv_handler(void *user_state, struct net_socket *socket,
    void *data, size_t size, struct in_addr addr, uint16_t port)
{
    debug_printf("message received:%s.\n", data);
    if (echo_counter < 0) return ;
    echo_counter--;
    strcpy(payload, "world.\n");
    send_message(socket, payload, addr, port);
}

static errval_t init(char *ip)
{
    net_sockets_init_with_card("zynqmp_gem");
    pcb = net_udp_socket();
    if (pcb == NULL) {
        return LIB_ERR_MALLOC_FAIL;
    }

    int ret = inet_aton(ip, &dest_ip);
    printf("my dbg ip = %x.\n", dest_ip.s_addr);
    if (ret == 0) {
        debug_printf("Invalid IP addr: %s\n", ip);
        return 1;
    }

    debug_printf("connecting to %s:%" PRIu16 "\n", ip, udp_port);

    errval_t r;
    r = net_bind(pcb, (struct in_addr){(INADDR_ANY)}, udp_port);
    if (r != SYS_ERR_OK) {
        USER_PANIC("UDP bind failed");
    }
    debug_printf("bound to %d\n", pcb->bound_port);
    debug_printf("registering recv handler\n");
    net_set_on_received(pcb, recv_handler);
    payload = net_alloc(64);
    return SYS_ERR_OK;
}

/*
static errval_t cleanup(void)
{
    net_close(pcb);
    return SYS_ERR_OK;
}
 */

#pragma GCC push_options
#pragma GCC optimize("O0")

int main(int argc, char *argv[])
{
    errval_t err;
    if (argc < 2) {
        debug_printf("%s: missing arguments!\n", argv[0]);
    }
    debug_printf("test net sockets initializing.\n");
    err = init(argv[1]);
    assert(err_is_ok(err));
#ifdef CLIENT
    while (1) {
        int i, j;
        i = -50000000;
        j = -3;
        while (j < 0) {
            i++;
            if (i > 0) {
                j++;
                i = -50000000;
            }
        }
        debug_printf("test net sockets start sending.\n");
        strcpy(payload, "hello.\n");
        err = send_message(pcb, payload, dest_ip, udp_port);
        assert(err_is_ok(err));
    }
#else
    while(1) {
        event_dispatch(get_default_waitset());
    }
#endif
    //cleanup();
    return 0;
}

#pragma GCC pop_options