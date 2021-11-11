/*
 *                               POK header
 *
 * The following file is a part of the POK project. Any modification should
 * be made according to the POK licence. You CANNOT use this file or a part
 * of a file for your own project.
 *
 * For more information on the POK licence, please see our LICENCE FILE
 *
 * Please follow the coding guidelines described in doc/CODING_GUIDELINES
 *
 *                                      Copyright (c) 2007-2021 POK team
 */

#include <assert.h>
#include <core/partition.h>
#include <core/semaphore.h>
#include <core/thread.h>
#include <libc/stdio.h>
#include <types.h>

#ifndef AFFINITY_0
#define AFFINITY_0 0
#endif

#ifndef AFFINITY_1
#define AFFINITY_1 0
#endif

static uint8_t sid;
static uint8_t fail, success, signal = 0;
static int j;

static void *pinger_job() {
  for (j = 1; j <= 5; j++) {
    pok_thread_sleep(15000);
    printf("P2T1: semaphore signal (round %d)\n", j);
    signal++;
    pok_sem_signal(sid);
    if (j % 2) {
      printf("P2T1: second semaphore signal (round %d)\n", j);
      signal++;
      pok_sem_signal(sid);
    }
  }
  printf("P2T1: sleeping forever\n");
  for (;;)
    pok_thread_sleep(1000000);
  return NULL;
}

static void *pinger_job2() {
  pok_ret_t ret;
  for (int i = 1; i <= 15; i++) {
    printf("P2T2: I will wait for the semaphore (round %d)\n", i);
    ret = pok_sem_wait(sid, 10000);
    if (!ret) {
      printf("P2T2: got the semaphore (round %d)\n", i);
      success++;
      assert(success == signal + 1);
    } else {
      printf("P2T2: failed to get the semaphore (round %d)\n", i);
      fail++;
      assert(fail == j);
    }
  }
  printf("P2T2: sleeping forever\n");
  printf("SUCCESS!!!");
  for (;;)
    pok_thread_sleep(1000000);
  return NULL;
}

int main() {
  uint32_t tid;
  pok_ret_t ret;
  pok_thread_attr_t tattr;

  printf("[P2] Semaphore start value is 1\n");
  ret = pok_sem_create(&sid, 1, 50, POK_QUEUEING_DISCIPLINE_DEFAULT);
  printf("[P2] pok_sem_create return=%d, mid=%d\n", ret, sid);

  tattr.entry = pinger_job;
  tattr.processor_affinity = AFFINITY_0;

  ret = pok_thread_create(&tid, &tattr);
  printf("[P2] pok_thread_create (1) return=%d\n", ret);

  tattr.entry = pinger_job2;
  tattr.processor_affinity = AFFINITY_1;

  ret = pok_thread_create(&tid, &tattr);
  printf("[P2] pok_thread_create (2) return=%d\n", ret);

  pok_partition_set_mode(POK_PARTITION_MODE_NORMAL);

  return 0;
}
