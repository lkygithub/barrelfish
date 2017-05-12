#include "barrelrpc.h"
#include <stdio.h>
#include <string.h>

int main(void)
{
    errval_t err;
    int in;
    String str = (String) malloc(2048);

    //create channel first!
    err = create_channel();
    if (err_is_fail(err)){
        debug_printf("client: create channel failed!\n");
        return EXIT_SUCCESS;
    }
    //first send: 42
    in = 42;    
    printf("\n###client: call myrpc sent int: %d\n\n",in);
    err = myrpc(in, &str);

    if (err_is_ok(err)) {
        printf("\n###client: myrpc recieve String:\n\t%s\n", str);
    } else {
        DEBUG_ERR(err, "barrelrpc myrpc");
    }
    
    //second send: 24
    printf("########Test Again########\n");
    in = 24;
    printf("\n###client: call myrpc sent int: %d\n\n",in);
    err = myrpc(in, &str);

    if (err_is_ok(err)) {
        printf("\n###client: myrpc recieve String:\n\t%s\n", str);
    } else {
        DEBUG_ERR(err, "barrelrpc myrpc");
    }
    
    //FIXME: free channel then we can exit!
	while(1);

	return EXIT_SUCCESS;
}
