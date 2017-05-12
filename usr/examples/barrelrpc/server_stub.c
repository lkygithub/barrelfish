#include <stdio.h>
#include <string.h>
#include "barrelrpc.h"

static const char *service_name = "barrelrpc_rpc_service";
static errval_t connect_cb(void *st, struct barrelrpc_binding *b);
static void export_cb(void *st, errval_t err, iref_t iref);

static void send_myrpc_response_cb(void *a)
{
    struct myrpc_params_state *st = (struct myrpc_params_state*)a;

    debug_printf("server: myresponse sent succesfully\n");

	if(st->p1 != NULL) free(st->p1);

    free(st);
}

static void send_myrpc_response(void *a)
{
    errval_t err;
    struct myrpc_params_state *st = (struct myrpc_params_state*)a;

    struct event_closure txcont = MKCONT(send_myrpc_response_cb, st);

	myrpc_impl(st->p0, &(st->p1));

    debug_printf("server: sending myresponse\n");

    err = barrelrpc_myrpc_response__tx(st->b, txcont , (st->p1));

    if (err_is_fail(err)) {
        if (err_no(err) == FLOUNDER_ERR_TX_BUSY) {
            debug_printf("server: re-sending myresponse\n");
            struct waitset *ws = get_default_waitset();
            txcont = MKCONT(send_myrpc_response, st);
            err = st->b->register_send(st->b, ws, txcont);
            if (err_is_fail(err)) {
                // note that only one continuation may be registered at a time
                DEBUG_ERR(err, "register_send on binding failed!");
                free(st);
            }
        } else {
            DEBUG_ERR(err, "error sending mycall message\n");
            free(st);
        }
    }
}

static void rx_myrpc_call(struct barrelrpc_binding *b , int32 p0)
{
    debug_printf("server: received myrpc_call\n");

    // prepare and send reply

    struct myrpc_params_state *st = malloc(sizeof(struct myrpc_params_state));
    if (st == NULL) {
        USER_PANIC("cannot reply, out of memory");
    }

    st->b = b;
	st->p0 = p0;
	st->p1 = (String) malloc(2048);


    send_myrpc_response(st);
}


static void start_server(void)
{
    errval_t err;

    err = barrelrpc_export(NULL /* state pointer for connect/export callbacks */,
                        export_cb, connect_cb,
                        get_default_waitset(),
                        IDC_EXPORT_FLAGS_DEFAULT);
    if (err_is_fail(err)) {
        USER_PANIC_ERR(err, "export failed");
    }
}

static struct barrelrpc_rx_vtbl s_rx_vtbl = {
	.myrpc_call = rx_myrpc_call,

};

static errval_t connect_cb(void *st, struct barrelrpc_binding *b) 
{    
    b->rx_vtbl = s_rx_vtbl;

    return SYS_ERR_OK;
}

static void export_cb(void *st, errval_t err, iref_t iref)
{
    if (err_is_fail(err)) {
        USER_PANIC_ERR(err, "export failed");
    }

    err = nameservice_register(service_name, iref);
    if (err_is_fail(err)) {
        USER_PANIC_ERR(err, "nameservice_register failed");
    }
}

void event_loop(void)
{
	int err;
	start_server();
    while (1) {
        err = event_dispatch(get_default_waitset());
        if (err_is_fail(err)) {
            DEBUG_ERR(err, "in event_dispatch");
            break;
        }
    }
}
