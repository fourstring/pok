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

#include <core/semaphore.h>
#include <core/thread.h>
#include <core/time.h>
#include <libc/stdio.h>
#include <stdio.h>
#include <types.h>
#include <assert.h>
#include <errno.h>

#define SECOND_NS 1000000000LL

void *timing_job() {
  pok_ret_t ret;
  pok_time_t ns;
  while (1) {
    ret = pok_time_get(&ns);
    assert(ret == POK_ERRNO_OK);
    printf("P1T1: now %llu ns, %llu s\n", ns, ns / SECOND_NS);
    pok_thread_sleep(SECOND_NS / 1000 / 10);
  }
}
