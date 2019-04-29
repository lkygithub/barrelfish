#include <barrelfish/barrelfish.h>
#include <barrelfish/spawn_client.h>

static char *task_id[] = {"1", "2", "3", "4", "5"};
static char *tstart[] = {"0", "5000", "11000", "18000", "-30000"};

int main(int argc, char *argv[])
{
    coreid_t core_id = disp_get_core_id();
    errval_t err;
    int i;
    if (argc == 1)
    {
        //spawn
        for (i = 0; i < sizeof(task_id) / sizeof(char *); i++)
        {
            char *xargv[] = {"tt_task", task_id[i], tstart[i], NULL};
            printf("spawning... coreid:%d\n", core_id);
            err = spawn_program(core_id, argv[0], xargv, NULL,
                                SPAWN_FLAGS_DEFAULT, NULL);
            printf("spawned.\n");
            assert(err_is_ok(err));
        }
    }
    else
    {
        i = 1234567;
        while (true)
        {
            if (i % 2)
                i /= 2;
            else
            {
                i = i * 3 + 1;
            }
            if (i == 22222222) {
                printf("i = 2222\n");
            }
            //printf("test tt sched taskid:%s\n", argv[1]);
        }
    }
}
