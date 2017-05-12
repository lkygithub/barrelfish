#include "barrelrpc.h"

errval_t myrpc_impl(int32 val, String *str)
{
	printf ("\n###server: myrpc_impl recieved int: %d\n\n", val);
	
	//do what you want to do!
	snprintf(*str, 20,  "val is %d\n", val);
	
	return SYS_ERR_OK;
}

int main(void)
{
	event_loop();

	return 1;
}
