# 设计报告

## 1 多线程调度

为了实现新的线程调度算法，我们添加了新的`pok_sched_t`类型。根据程序的设置，调度函数会将partition的`sched_func`设置为对应的调度函数。

线程调度实验环境预设。POK的实现中对于线程的time-capacity的处理有问题，在当前线程remaining_time_capacity用完后，应进入 POK_STATE_WAIT_NEXT_ACTIVATION 状态，等待周期结束重新激活（即填充remaining_time_capacity），但函数 pok_elect_thread()原有逻辑有误，导致永远无法进入 POK_STATE_WAIT_NEXT_ACTIVATION 状态。原实现如下：

```c
if ((POK_SCHED_CURRENT_THREAD != IDLE_THREAD) &&
    (POK_SCHED_CURRENT_THREAD != POK_CURRENT_PARTITION.thread_main) &&
    (POK_SCHED_CURRENT_THREAD != POK_CURRENT_PARTITION.thread_error)) {
    if (POK_CURRENT_THREAD.remaining_time_capacity > 0) {
    POK_CURRENT_THREAD.remaining_time_capacity =
        POK_CURRENT_THREAD.remaining_time_capacity - 1;
    } else if (POK_CURRENT_THREAD.time_capacity >
                0) // Wait next activation only for thread
                // with non-infinite capacity (could be
                // infinite with value -1 <--> INFINITE_TIME_CAPACITY)
    {
    POK_CURRENT_THREAD.state = POK_STATE_WAIT_NEXT_ACTIVATION;
    }
}
```

我们尝试在调度策略中维护，但在上层函数逻辑错误的情况下无法解决本质问题，除非破坏time-capacity准确语义。于是修改后的代码如下：

```c
if ((POK_SCHED_CURRENT_THREAD != IDLE_THREAD) &&
    (POK_SCHED_CURRENT_THREAD != POK_CURRENT_PARTITION.thread_main) &&
    (POK_SCHED_CURRENT_THREAD != POK_CURRENT_PARTITION.thread_error)) {
    if (POK_CURRENT_THREAD.remaining_time_capacity > 0) {
    POK_CURRENT_THREAD.remaining_time_capacity =
        POK_CURRENT_THREAD.remaining_time_capacity - 1;
    }
    if (POK_CURRENT_THREAD.remaining_time_capacity == 0 &&
    POK_CURRENT_THREAD.time_capacity >
                0) // Wait next activation only for thread
                // with non-infinite capacity (could be
                // infinite with value -1 <--> INFINITE_TIME_CAPACITY)
    {
    POK_CURRENT_THREAD.state = POK_STATE_WAIT_NEXT_ACTIVATION;
    }
}
```

下面将依次介绍线程调度策略的设计实现。

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

### 1.3 Round-Robin 调度

POK原来的RR调度实现实际上不是 Round-Robin，只能说是当一个的`time-capacity`在一个周期内用完后调度下一个。`time-capacity`表示在周期内最多占用的时间量，可以用来模拟计算时间确定的周期性任务，当`time-capacity`用完时表示该任务完成。因此原来的调度与基于任务创建时间先后的优先级调度类似。

我们实现了真正的 Round-Robin 调度，使用`pok_sched_part_real_rr()`函数进行调度，对应于kernel配置中的`POK_SCHED_REAL_RR`调度设置。为实现RR调度，我们在线程结构体中加入了剩余时间片数（`rr_budget`，时间片即连续两次调度的时间间隔，我们定义为 20 `tick`），如下：

```c
typedef struct {
  uint8_t priority;
  int64_t period;
  uint64_t deadline;
  uint64_t current_deadline;
  int64_t time_capacity;
  int64_t remaining_time_capacity;
  uint64_t rr_budget;   // remain time slice number
  ...
} pok_thread_t;
```

`pok_sched_part_real_rr()`函数与POK原有的调度函数形式类似，但额外在该函数中维护了每个线程（即任务）的剩余时间片。首先结算先前被调度的线程的剩余时间片，再从该线程开始，在可调度线程中选取第一个剩余时间片不为0的线程。若遇到剩余时间片为0的线程，则恢复它的剩余时间片数，并跳过。

```c
uint32_t pok_sched_part_real_rr(const uint32_t index_low, const uint32_t index_high,
                                const uint32_t prev_thread,
                                const uint32_t current_thread) {
    uint32_t elected;
    uint32_t from;
    bool_t exist_one = FALSE;
    uint8_t current_proc = pok_get_proc_id();

    if (current_thread == IDLE_THREAD) {
        elected = (prev_thread != IDLE_THREAD) ? prev_thread : index_low;
    } else {
        if (pok_threads[current_thread].rr_budget > 0) {
            pok_threads[current_thread].rr_budget--;
        }
        elected = current_thread;
    }

    from = elected;

    do {
        if ((pok_threads[elected].remaining_time_capacity > 0 ||
            pok_threads[elected].time_capacity == INFINITE_TIME_VALUE) &&
            pok_threads[elected].state == POK_STATE_RUNNABLE &&
            pok_threads[elected].processor_affinity == current_proc) {
            // At least one thread can be scheduled under other conditions
            exist_one = TRUE;

            if (pok_threads[elected].rr_budget > 0) {
                break;
            } else {
                pok_threads[elected].rr_budget = POK_LAB_SCHED_RR_BUDGET;
            }
        }
        elected++;
        if (elected >= index_high) {
            elected = index_low;
        }
    } while (elected != from || exist_one);

    if (!exist_one) {
        elected = IDLE_THREAD;
    }
    return elected;
}
```

### 1.3 Weighted-Round-Robin 调度

带权的Round-Robin基于Round-Robin很容易实现，只需在线程中维护`weight`成员，以此来初始化和重置`rr_budget`即可。线程结构如图所示：

```c
typedef struct {
  uint8_t priority;
  int64_t period;
  uint64_t deadline;
  uint64_t current_deadline;
  int64_t time_capacity;
  int64_t remaining_time_capacity;
  uint64_t rr_budget;
  uint64_t weight;
  ...
}
```

使用`pok_sched_part_wrr()` 函数进行调度，代码如下：

```c
uint32_t pok_sched_part_wrr(const uint32_t index_low, const uint32_t index_high,
                                const uint32_t prev_thread,
                                const uint32_t current_thread) {
    uint32_t elected;
    uint32_t from;
    bool_t exist_one = FALSE;
    uint8_t current_proc = pok_get_proc_id();

    if (current_thread == IDLE_THREAD) {
        elected = (prev_thread != IDLE_THREAD) ? prev_thread : index_low;
    } else {
        if (pok_threads[current_thread].rr_budget > 0) {
            pok_threads[current_thread].rr_budget--;
        }
        elected = current_thread;
    }

    from = elected;

    do {
        if ((pok_threads[elected].remaining_time_capacity > 0 ||
            pok_threads[elected].time_capacity == INFINITE_TIME_VALUE) &&
            pok_threads[elected].state == POK_STATE_RUNNABLE &&
            pok_threads[elected].processor_affinity == current_proc) {
            // At least one thread can be scheduled under other conditions
            exist_one = TRUE;

            if (pok_threads[elected].rr_budget > 0) {
                break;
            } else {
                pok_threads[elected].rr_budget = pok_threads[elected].weight * POK_LAB_SCHED_RR_BUDGET;
            }
        }
        elected++;
        if (elected >= index_high) {
            elected = index_low;
        }
    } while (elected != from || exist_one);

    if (!exist_one) {
        elected = IDLE_THREAD;
    }
    return elected;
}
```

## 2 动态线程创建

`POK_CONFIG_PARTITIONS_NTHREADS` 表示所有分区对应的总的线程控制块数量，POK的线程数量本身是确定的，不能更改的，这是因为线程控制块所在的数组是静态声明的，而不是动态分配的。换句话说，线程数量有上限。因此，为了避免线程创建系统调用的误用，在系统调用处理函数`pok_partition_thread_create()`中，当分区状态已为`NORMAL`时，不予创建并返回创建失败，如下所示：

```c
if ((pok_partitions[partition_id].mode != POK_PARTITION_MODE_INIT_COLD) &&
    (pok_partitions[partition_id].mode != POK_PARTITION_MODE_INIT_WARM)) {
    return POK_ERRNO_MODE;
}
```

实际上，未初始化的线程控制块本身状态为 `POK_STATE_STOPPED`，不参与调度，不影响正确性。因此只要我们使用`POK_CONFIG_PARTITIONS_NTHREADS` 预留足够的空间，并在线程创建系统调用中允许创建线程、正常初始化线程即可。
为了不侵入修改已有逻辑，我们专为动态创建的情况作判断，已完成特殊处理。在`pok_thread_attr_t` 引入了一个新的属性 `dynamic_created`，表示为动态创建的线程。

```c
typedef struct {
  uint8_t priority; /* Priority is from 0 to 255 */
  uint8_t processor_affinity;
  void *entry; /* entrypoint of the thread  */
  uint64_t period;
  uint64_t weight;
  uint64_t deadline;
  uint64_t time_capacity;
  uint32_t stack_size;
  pok_state_t state;
  bool_t dynamic_created;
} pok_thread_attr_t;
```

同时在 `pok_partition_thread_create()` 做判断：

```c
if (!attr->dynamic_created &&
    (pok_partitions[partition_id].mode != POK_PARTITION_MODE_INIT_COLD) &&
    (pok_partitions[partition_id].mode != POK_PARTITION_MODE_INIT_WARM)) {
return POK_ERRNO_MODE;
}
```

经测试无误，详见测试报告。
