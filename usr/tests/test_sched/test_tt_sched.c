#include <barrelfish/barrelfish.h>
#include <barrelfish/spawn_client.h>
#include <stdlib.h>

uint64_t now(void);
uint64_t now(void)
{

    uint64_t cntpct;
    __asm volatile(
        "mrs %[cntpct], cntpct_el0 \n\t"
        : [cntpct] "=r"(cntpct));
    return cntpct;
}

//static char *node_0_task_id[] = {NULL};
//static char *node_1_task_id[] = {"1", NULL};
//static char *node_2_task_id[] = {"1", NULL};

int main(int argc, char *argv[])
{

    errval_t err;
    char time_str[20];
    coreid_t core_id;
    printf("spawning...\n");

    /* core 0 */
    printf("spawned on core %d.\n", 0);
    core_id = 0;
    /* core 1 */
    printf("spawned on core %d.\n", 1);
    core_id = 1;
    char *xargv_1_1[] = {"tt_task", "1", "0", /* for tt-sched, second one is my_task_id */
        "1",    /* my_task_id */
        "2",    /* dst_core_id */
        "2",    /* dst_task_id */
        "100000", /* deadline */
        "1000000", /* peroid */
        NULL};
    err = spawn_program(core_id, "/armv8/sbin/tt/producer", xargv_1_1, NULL, SPAWN_FLAGS_DEFAULT, NULL);
    assert(err_is_ok(err));
    char *xargv_1_2[] = {"tt_task", "2", "0", /* for tt-sched */
        "2",    /* my_task_id */
        "500000", /* deadline */
        "1000000", /* peroid */
        NULL};
    err = spawn_program(core_id, "/armv8/sbin/tt/free", xargv_1_2, NULL, SPAWN_FLAGS_DEFAULT, NULL);
    assert(err_is_ok(err));
    char *xargv_1_3[] = {"tt_task", "3", "0", /* for tt-sched */
        "3",        /* my_task_id */
        "2",        /* src_core_id */
        "2",        /* src_task_id */
        "766666",   /* deadline */
        "1000000",   /* peroid */
        NULL};
    err = spawn_program(core_id, "/armv8/sbin/tt/consumer", xargv_1_3, NULL, SPAWN_FLAGS_DEFAULT, NULL);
    assert(err_is_ok(err));

    /* core 2 */
    printf("spawned on core %d.\n", 2);
    core_id = 2;
    char *xargv_2_1[] = {"tt_task", "1", "0", /* for tt-sched */
        "1",    /* my_task_id */
        "650000", /* deadline */
        "1000000", /* peroid */
        NULL};
    err = spawn_program(core_id, "/armv8/sbin/tt/free", xargv_2_1, NULL, SPAWN_FLAGS_DEFAULT, NULL);
    assert(err_is_ok(err));
    char *xargv_2_2[] = {"tt_task", "2", "0", /* for tt-sched */
        "2",        /* my_task_id */
        "1",        /* src_core_id */
        "1",        /* src_task_id */
        "1",        /* dst_core_id */
        "3",        /* dst_task_id */
        "500000",   /* deadline */
        "1000000",   /* peroid */
        NULL};
    err = spawn_program(core_id, "/armv8/sbin/tt/shaper", xargv_2_2, NULL, SPAWN_FLAGS_DEFAULT, NULL);
    assert(err_is_ok(err));
    char *xargv_2_3[] = {"tt_task", "3", "0", /* for tt-sched */
        "3",    /* my_task_id */
        "250000", /* deadline */
        "1000000", /* peroid */
        NULL};
    err = spawn_program(core_id, "/armv8/sbin/tt/free", xargv_2_3, NULL, SPAWN_FLAGS_DEFAULT, NULL);
    assert(err_is_ok(err));

    uint64_t t = now();
    t += 100 * 1000000 * 4; //3s
    sprintf(time_str, "%llu", t);

    /* not notice core 0 */
    for(int i = 1; i < 3; i++){
        char *xargv[] = {time_str, "1000000", NULL};
        err = spawn_program(i, "/armv8/sbin/tt/tt_starter", xargv, NULL, SPAWN_FLAGS_DEFAULT, NULL);
        assert(err_is_ok(err));
    }

    t = now() + 100 * 1000000;
    while(now() < t);
    printf("tt schedule start after serial time\n");
}
