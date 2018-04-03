#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <barrelfish/barrelfish.h>
#include <trace/trace.h>
#include <trace_definitions/trace_defs.h>

static void callback(void *arg)
{
    printf("%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
    return;
}

int main(void){
    // initialize the trace framework
    trace_reset_all();
    trace_prepare(MKCLOSURE(callback, NULL)); //do clock synchronization

    //tell the framework when to start recording
    trace_control(TRACE_EVENT(TRACE_SUBSYS_MTRACE,
                                    TRACE_EVENT_MTRACE_START, 0),
                        TRACE_EVENT(TRACE_SUBSYS_MTRACE,
                                    TRACE_EVENT_MTRACE_END, 0),
                                    0);
    
    // enable the subsystems that should be traced
    trace_set_subsys_enabled ( TRACE_SUBSYS_MTRACE , true );
    // or activate for all:
    trace_set_all_subsys_enabled ( true );
    // fire the event that triggers start of recording
    trace_event ( TRACE_SUBSYS_MTRACE , TRACE_EVENT_MTRACE_RUN , 5643 );
    int i, count = 0;
    for (i = 0; i < 1000; i++){
        count += count * i / (i + 1);
    }
    // fire the event that triggers end of recording
    trace_event ( TRACE_SUBSYS_MTRACE , TRACE_EVENT_MTRACE_RUN , count );
    // Flush the buffer
    trace_flush (MKCLOSURE(callback, NULL));
}