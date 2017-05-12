#include <stdio.h>
#include <string.h>
#include "barrelrpc.h"

const char *service_name = "barrelrpc_rpc_service";
int finished = 0; // for event_dispatch
int initialized = 0; // create channel
struct barrelrpc_binding *gb;
errval_t gerr;

/* --------------------- Client ------------------------------ */


struct myrpc_params_state params;

static void rx_myrpc_response(struct barrelrpc_binding *b, String s)
{
	strcpy (params.p1, s);

    finished = 1; //rx_response
}


static void send_myrpc_call_cb(void *a)
{
    debug_printf("client: myrpc_call sent successfully\n");
}

static void send_myrpc_call(void *a)
{
    errval_t err;

    debug_printf("client: sending mycall\n");

    struct barrelrpc_binding *b = (struct barrelrpc_binding *)a;

    struct event_closure txcont = MKCONT(send_myrpc_call_cb, b);

    err = barrelrpc_myrpc_call__tx(b, txcont , params.p0);

    if (err_is_fail(err)) {
        if (err_no(err) == FLOUNDER_ERR_TX_BUSY) {
            debug_printf("client: re-sending mycall\n");
            struct waitset *ws = get_default_waitset();
	        txcont = MKCONT(send_myrpc_call, b);
            err = b->register_send(b, ws, txcont);
            if (err_is_fail(err)) {
                // note that only one continuation may be registered at a time
                DEBUG_ERR(err, "register_send on binding failed!");
            }
        } else {
            DEBUG_ERR(err, "error sending mycall message\n");
        }
    }

    gerr = err;
}

errval_t myrpc(int32 qqq, String *s){
    errval_t err;
#ifdef _RPCCOTL_SOME_TIMES_
    int try_times;
#endif
	params.p0 = qqq;
	params.p1 = *s;


    if (!initialized)
        USER_PANIC("channel have not been created!");
    //barrelfish require send_myrpc_call have no return!
    //so can't write 'err = send_myrpc_call(gb);'
    //store errval in global val gerr!
    gerr = SYS_ERR_OK;
    send_myrpc_call(gb);

#ifdef _RPCCOTL_ONLY_ONCE_
    if (err_is_fail(gerr))
        return gerr;
#else
    for (try_times = 0; try_times < MAX_TRY_TIMES; try_times++){
        gerr = SYS_ERR_OK;
        send_myrpc_call(gb);
        if (err_is_ok(gerr))
            break;
    }
    if (try_times == MAX_TRY_TIMES)
        return gerr;
#endif

    //事件循环
    finished = 0;
    struct waitset *ws = get_default_waitset();
    while (1) {
        if (finished == 1) //rx_response
			break;
        err = event_dispatch(ws);
        if (err_is_fail(err)) {
            DEBUG_ERR(err, "in event_dispatch");
            break;
        }
    }

    return SYS_ERR_OK;
} 

struct barrelrpc_rx_vtbl c_rx_vtbl = {
	.myrpc_response = rx_myrpc_response,

};

static void bind_cb(void *st, errval_t err, struct barrelrpc_binding *b)
{
    if (err_is_fail(err)) {
        USER_PANIC_ERR(err, "bind failed");
    }
    
    b->rx_vtbl = c_rx_vtbl;
    gb = b;
    finished = 2; //create_channel
    initialized = 1;
}


errval_t create_channel(void)
{
    errval_t err;
    iref_t iref;
    int try_times;

    err = nameservice_blocking_lookup(service_name, &iref);
    if (err_is_fail(err)) {
        DEBUG_ERR(err, "nameservice_blocking_lookup failed");
        return err;
    }

    err = barrelrpc_bind(iref, 
                     bind_cb, 
                     NULL /* state for bind_cb */,
                     get_default_waitset(),
                     IDC_BIND_FLAGS_DEFAULT);
    
#ifdef RPCCOTL_ONLY_ONCE
    if (err_is_fail(err)){
        DEBUG_ERR(err, "bind failed");
        return err;
    }
#else
    for (try_times = 0; try_times < MAX_TRY_TIMES; try_times++){
        err = barrelrpc_bind(iref, 
                        bind_cb, 
                        NULL /* state for bind_cb */,
                        get_default_waitset(),
                        IDC_BIND_FLAGS_DEFAULT);
        if (err_is_ok(err))
            break;
    }
    if (try_times == MAX_TRY_TIMES)
        return err;
#endif

    //事件循环,等待链接建立
    finished = 0;
    struct waitset *ws = get_default_waitset();
    while (1) {
        if (finished == 2) //create_channel
			break;
        err = event_dispatch(ws);
        if (err_is_fail(err)) {
            DEBUG_ERR(err, "in event_dispatch");
            break;
        }
    }
    
    return SYS_ERR_OK;
}
