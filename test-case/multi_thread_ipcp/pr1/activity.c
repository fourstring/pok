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
 *                                      Copyright (c) 2007-2022 POK team
 */

#include <core/mutex.h>
#include <core/thread.h>
#include <core/time.h>
#include <libc/stdio.h>
#include <stdio.h>
#include <types.h>
#include <assert.h>

extern uint8_t mid;
uint8_t val;

void fake_job(char *name, pok_time_t time, pok_time_t report_interval) {
  pok_time_t start, now, last_report, runned = 0;
  pok_ret_t ret;

  ret = pok_time_get(&start);
  assert(ret == POK_ERRNO_OK);
  now = last_report = start;

  while (runned < time) {
    ret = pok_time_get(&now);
    assert(ret == POK_ERRNO_OK);
    if (now - last_report > report_interval) {
      printf("%s: I'm alive at %llu\n", name, now);
      last_report = now;
      runned += report_interval;
    }
  }
}

#define SECOND_US 1000000LL
#define SECOND_NS (1000LL * SECOND_US)

void *high_prio_job() {
  pok_ret_t ret;
  pok_time_t ns;
  // sleep deliberately
  pok_thread_sleep(5 * SECOND_US);

  ret = pok_mutex_lock(mid);
  printf("high_prio_job: mutex lock, ret=%d\n", ret);

  ret = pok_time_get(&ns);
  assert(ret == POK_ERRNO_OK);
  printf("high_prio_job: I'm alive at %llu ns\n", ns);

  ret = pok_mutex_unlock(mid);
  printf("high_prio_job: mutex unlock, ret=%d\n", ret);

  pok_thread_wait_infinite();
}

void *medium_prio_job() {
  pok_thread_sleep(3 * SECOND_US);

  fake_job("medium_prio_job", 5 * SECOND_NS, 1 * SECOND_NS / 2);

  pok_thread_wait_infinite();
}

void *low_prio_job() {
  pok_ret_t ret;

  ret = pok_mutex_lock(mid);
  printf("low_prio_job: mutex lock, ret=%d\n", ret);

  fake_job("low_prio_job", 10 * SECOND_NS, 1 * SECOND_NS / 2);

  ret = pok_mutex_unlock(mid);
  printf("low_prio_job: mutex unlock, ret=%d\n", ret);
  
  pok_thread_wait_infinite();
}
