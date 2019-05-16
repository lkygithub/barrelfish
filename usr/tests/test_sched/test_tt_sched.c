#include <barrelfish/barrelfish.h>
#include <barrelfish/spawn_client.h>

uint64_t now(void);

static char *task_id[] = {"1", "3", "2", "3", "-1", "1000"};
static char *tstart[] = {"0", "15000", "16883", "21383", "34500", "50000"};

uint64_t now(void)
{

    uint64_t cntpct;
    __asm volatile(
        "mrs %[cntpct], cntpct_el0 \n\t"
        : [cntpct] "=r"(cntpct));
    return cntpct;
}

int main(int argc, char *argv[])
{
    if (!strcmp(argv[0], "tt_task"))
    {
        int i = 1234567;
        while (true)
        {
            if (i % 2)
                i /= 2;
            else
            {
                i = i * 3 + 1;
            }
            if (i == 22222222)
            {
                printf("i = 2222\n");
            }
        }
    }
    else if (!strcmp(argv[0], "tt_starter"))
    {
        sys_setoff_tt();
    }
    else
    {
        errval_t err;
        int i, j;
        //spawn
        assert(argc > 1);
        printf("spawning...\n");
        for (i = 1; i < argc; i++)
        {
            coreid_t coreid = strtol(argv[i], NULL, 10);
            for (j = 0; j < sizeof(task_id) / sizeof(char *); j++)
            {
                char *xargv[] = {"tt_task", task_id[j], tstart[j], NULL};
                err = spawn_program(coreid, argv[0], xargv, NULL,
                                    SPAWN_FLAGS_DEFAULT, NULL);
                printf("spawned %d on core %d.\n", j, coreid);
                assert(err_is_ok(err));
            }
        }

        uint64_t base = now();
        uint64_t twait = 0x50000000; //some seconds.
        printf("waiting.\n");
        while (now() <= base + twait)
            ;
        printf("tt starting.\n");
        for (i = 1; i < argc; i++)
        {
            coreid_t coreid = strtol(argv[i], NULL, 10);
            char *xargv[] = {"tt_starter", NULL};
            err = spawn_program(coreid, argv[0], xargv, NULL,
                                    SPAWN_FLAGS_DEFAULT, NULL);
            printf("spawned tt starter on core %d\n", coreid);
            assert(err_is_ok(err));
        }
    }
}
