#include <barrelfish/barrelfish.h>
#include <barrelfish/nameservice_client.h>

#include <if/barrelrpc_defs.h>
#include <if/barrelrpc_rpcclient_defs.h>

#define _RPCCOTL_ONLY_ONCE_
#define MAX_TRY_TIMES 3

typedef int8_t int8;
typedef uint8_t uint8;
typedef int16_t int16;
typedef uint16_t uint16;
typedef int32_t int32;
typedef uint32_t uint32;
typedef char* String;


struct myrpc_params_state
{
	struct barrelrpc_binding* b;
	int32 p0;
	String p1;

};

//client
errval_t myrpc(int32 qqq, String *s);

errval_t create_channel(void);

//server
errval_t myrpc_impl(int32 qqq, String *s);

void event_loop(void);
