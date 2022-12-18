#include <libc/stdio.h>
#include <libc/string.h>
#include <core/thread.h>
#include <core/partition.h>
#include <types.h>

static void task();
static void create_task();

/*
    Thread period's unit is ns, but time_capacity's unit is time slice(the period of schedule).
    Time slice is defined in core/time.h, the POK_TIMER_QUANTUM is the frequency of schedule.
    So in my config, time slice is 0.5s.
 */
#define NS_ONE_SECOND 1000000000LL

int main() {
    uint32_t tid;
    pok_thread_attr_t tattr;
    memset(&tattr, 0, sizeof(pok_thread_attr_t));

    tattr.period = 5 * NS_ONE_SECOND;
    tattr.time_capacity = 2;
    tattr.entry = task;
    pok_thread_create(&tid, &tattr);

    tattr.period = 5 * NS_ONE_SECOND;
    tattr.time_capacity = 1;
    tattr.entry = create_task;
    pok_thread_create(&tid, &tattr);

    pok_partition_set_mode(POK_PARTITION_MODE_NORMAL);
    pok_thread_wait_infinite();
    return 0;
}

static void task() {
    for (;;) {
    }
}

static void create_task() {
    uint32_t tid;
    pok_thread_attr_t tattr;
    memset(&tattr, 0, sizeof(pok_thread_attr_t));

    pok_thread_sleep(5000000);
    printf("SQY@%s trace create_task\n", __func__);

    tattr.period = 5 * NS_ONE_SECOND;
    tattr.time_capacity = 5;
    tattr.entry = task;
    tattr.dynamic_created = TRUE;
    pok_thread_create(&tid, &tattr);

    for (;;) {
    }
}
