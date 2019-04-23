#include <barrelfish/barrelfish.h>
#include <barrelfish/spawn_client.h>

static char *task_id[] = {"21", "-2", "34", "-1", "-3"};
static char *tstart[] = {"0", "5000", "8000", "13000", "-20000"};

int main(int argc, char *argv[])
{
    coreid_t core_id = disp_get_core_id();
    errval_t err;
    char name[64];
    strcpy(name, argv[0]);

    if (argc == 1)
    {
        //spawn
        for (int i = 0; i < sizeof(task_id) / sizeof(char*); i++)
        {
            char *xargv[] = {"tt_task", task_id[i], tstart[i], NULL};
            printf("spawning...\n");
            err = spawn_program(core_id, name, xargv, NULL,
                                SPAWN_FLAGS_DEFAULT, NULL);
            printf("spawned...\n");
            assert(err_is_ok(err));
        }
    } else {
        while(true) {
            printf("test tt sched taskid:%s\n", argv[1]);
        }
    }
}
