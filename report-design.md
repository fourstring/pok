# 设计报告

## 1 多线程调度

为了实现新的线程调度算法，我们添加了新的`pok_sched_t`类型。根据程序的设置，调度函数会将partition的`sched_func`设置为对应的调度函数。

### 1.1 抢占式优先级调度

使用`pok_sched_part_prio`函数执行抢占式优先级调度。

```c
uint32_t pok_sched_part_prio(const uint32_t index_low, const uint32_t index_high, const uint32_t prev_thread,
                                 const uint32_t current_thread) {
    return select_thread_by_property(priority_cmp, index_low, index_high, prev_thread, current_thread);
}

static uint32_t select_thread_by_property(thread_comparator_fn property_cmp, const uint32_t index_low,
                                          const uint32_t index_high, const uint32_t prev_thread,
                                          const uint32_t current_thread) {
    uint32_t start, iter, thread;
    uint32_t max_property_thread = IDLE_THREAD;

    if (current_thread == IDLE_THREAD) {
      start = prev_thread;
    } else {
      start = current_thread;
    }

    for (iter = 0; iter < index_high - index_low; iter++) {
      thread = (start + iter) % (index_high - index_low);
      if (pok_threads[thread].state == POK_STATE_RUNNABLE &&
          property_cmp(thread, max_property_thread) > 0) {
        max_property_thread = thread;
      }
    }

    return max_property_thread;
}
```

`select_thread_by_property`函数会循环遍历从`index_low`到`index_high`的线程，找出其中优先级最高的线程。其中`priority_cmp`函数选出`priority`更大的线程，具体实现如下：

```c
static int priority_cmp(uint32_t t1, uint32_t t2) {
    return pok_threads[t1].priority - pok_threads[t2].priority;
}
```

### 1.2 抢占式 EDF 调度

使用`pok_sched_part_edf`函数抢占式 EDF 调度，与1.1的框架类似，

```c
uint32_t pok_sched_part_edf(const uint32_t index_low, const uint32_t index_high, const uint32_t prev_thread,
                                const uint32_t current_thread) {
    return select_thread_by_property(deadline_cmp, index_low, index_high, prev_thread, current_thread);
}
```

`deadline_cmp`函数挑选deadline更大的线程，具体实现如下：

```c
static int deadline_cmp(uint32_t t1, uint32_t t2) {
    /* Handle threads that don't have deadlines */
    if (pok_threads[t1].deadline == 0) return -1;
    if (pok_threads[t2].deadline == 0) return 1;
    /* Select the thread with earliest deadline */
    return pok_threads[t2].current_deadline - pok_threads[t1].current_deadline;
}
```
