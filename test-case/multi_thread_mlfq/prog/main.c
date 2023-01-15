#include <libc/stdio.h>
#include <libc/string.h>
#include <core/thread.h>
#include <core/partition.h>
#include <types.h>

static void task();

#define NS_ONE_SECOND 1000000000LL

int main() {
    uint32_t tid;
    pok_thread_attr_t tattr;
    memset(&tattr, 0, sizeof(pok_thread_attr_t));

    tattr.period = 200;
    tattr.time_capacity = 40;
    tattr.entry = task;
    pok_thread_create(&tid, &tattr);

    tattr.period = 200;
    tattr.time_capacity = 80;
    tattr.entry = task;
    pok_thread_create(&tid, &tattr);

    tattr.period = 2000;
    tattr.time_capacity = -1;
    tattr.entry = task;
    pok_thread_create(&tid, &tattr);

    pok_partition_set_mode(POK_PARTITION_MODE_NORMAL);
    pok_thread_wait_infinite();
    return 0;
}

static void task() {
    for (;;) {
    }
}
