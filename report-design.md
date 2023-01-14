# 设计报告

## 多分区调度

### 设计

POK原有的分区调度模型为同一时刻整个系统内只有一个分区处于运行状态，各分区之间通过WRR调度（**为POK原有功能，因此无需再重复实现**），所有分区及它们的权重均在编译期静态声明，例如通过`config.yaml`:

```yaml
kernel:
    features: [debug]
    scheduler:
        major_frame: 53s
        slots:
            - partition: pr1
              duration: 2s
            - partition: pr2
              duration: 40s
            - partition: pr1
              duration: 1s
            - partition: pr2
              duration: 10s
```

这样的设计可以保证不同分区之间的时间隔离，但与此同时，也让实现多分区之间的抢占式优先级或EDF调度没有意义，因为各分区的优先级并不存在动态的变化，也不存在任务的动态产生。为了实现多分区抢占式优先级和EDF调度，我们修改了分区的调度模型以及配置文件格式，让每个分区也可以指定优先级、周期、周期内完成时间及deadline，如下所示：

```yaml
kernel:
    features: [debug, debug_part]
    partition_sched: fp
    scheduler:
        slots:
            - partition: pr1
              period: 10s
              complete_time: 4s
              deadline: 10s
              priority: 1
            - partition: pr2
              period: 2s
              complete_time: 1s
              deadline: 2s
              priority: 2
```

通过设置`partition_sched`属性为`fp`或`edf`，即可选择多分区抢占式优先级或EDF调度，同时使用我们修改后的分区调度模型。若不设置该属性，则分区调度仍为POK原有模型。

### 实现

目前POK在x86-qemu平台上只使用1个CPU核心，此外，从其中断处理函数prologue可以看到：

```c
#define INTERRUPT_HANDLER(name)                                                \
  void name(void);                                                             \
  void name##_handler(interrupt_frame *frame);                                 \
  asm(".global " #name "			\n"                                                 \
      "\t.type " #name ",@function	\n" #name                              \
      ":				\n"                                                                \
      "cli			\n"                                                               \
      "subl $4, %esp			\n"                                                     \
      "pusha				\n"                                                            \
      "push %ds				\n"                                                         \
      "push %es				\n"                                                         \
      "push %esp			\n"                                                         \
      "mov $0x10, %ax			\n"                                                    \
      "mov %ax, %ds			\n"                                                      \
      "mov %ax, %es			\n"                                                      \
      "call " #name "_handler		\n"                                             \
      "call update_tss			\n"                                                   \
      "addl $4, %esp			\n"                                                     \
      "pop %es				\n"                                                          \
      "pop %ds				\n"                                                          \
      "popa				\n"                                                             \
      "addl $4, %esp			\n"                                                     \
      "sti			\n"                                                               \
      "iret				\n");                                  \
  void name##_handler(interrupt_frame *frame)
```

在跳转到中断处理函数前，通过`cli`指令关闭了中断，此外，从POK IDT定义中可以看到：

```c
pok_ret_t pok_exception_init() {
  int i;

  for (i = 0; exception_list[i].handler != NULL; ++i) {
    pok_idt_set_gate(exception_list[i].vector, GDT_CORE_CODE_SEGMENT << 3,
                     (uint32_t)exception_list[i].handler, IDTE_INTERRUPT, 3);
  }

  return (POK_ERRNO_OK);
}
```

即所有同步异常的IDT均被设为`IDTE_INTERRUPT`，对于此类型IDT，CPU在调用前默认关闭中断。因此，在x86-qemu平台上，POK在内核态运行时不存在并发，所有内核态代码可以认为是原子执行。故只需要在每次触发调度逻辑时，选择一个新的分区来执行，即可实现分区的抢占。具体地，POK原有的调度逻辑如下：

```c
void pok_global_sched() {
  uint8_t elected_partition = POK_SCHED_CURRENT_PARTITION;
  elected_partition = pok_elect_partition();
  new_partition = elected_partition != POK_SCHED_CURRENT_PARTITION;
  POK_SCHED_CURRENT_PARTITION = elected_partition;

  if (multiprocessing_system) {
    start_rendezvous(&fence);
    pok_send_global_schedule_thread();
  }
  pok_global_sched_thread(TRUE);
}
```

我们所实现的分区调度逻辑只需要在需要抢占时，通过`pok_elect_partition`返回新的分区id，即可完成分区的抢占。

为了与POK原有分区调度逻辑兼容，我们将配置文件中的新增配置转换为多个`deployment.h`中的配置宏，在POK原有分区调度函数中，通过条件编译加入我们的hook：

```c
uint8_t pok_elect_partition() {
  uint8_t next_partition = POK_SCHED_CURRENT_PARTITION;
#if POK_CONFIG_NB_PARTITIONS > 1
  uint64_t now = POK_GETTICK();
  pok_sched_flushports(now);
#if !defined (POK_NEEDS_PRIO_PART_SCHED)
  if (pok_sched_next_deadline <= now) {
    /* Here, we change the partition */

    pok_sched_current_slot =
        (pok_sched_current_slot + 1) % POK_CONFIG_SCHEDULING_NBSLOTS;
    pok_sched_next_deadline =
        pok_sched_next_deadline + pok_sched_slots[pok_sched_current_slot];
    /*
        *  FIXME : current debug session about exceptions-handled
          printf ("Switch from partition %d to partition %d\n",
       pok_current_partition, pok_sched_current_slot); printf ("old current
       thread = %d\n", POK_SCHED_CURRENT_THREAD);

          printf ("new current thread = %d\n",
       CURRENT_THREAD(pok_partitions[pok_sched_current_slot])); printf ("new
       prev current thread = %d\n",
       pok_partitions[pok_sched_current_slot].prev_thread);
          */
    next_partition = pok_sched_slots_allocation[pok_sched_current_slot];
  }
#else
  next_partition = pok_partition_prio_sched(now);
#endif /* defined (POK_NEEDS_PRIO_PART_SCHED) */
#endif /* POK_CONFIG_NB_PARTITIONS > 1 */

  return next_partition;
}
```

如果在`config.yaml`中设置了`kernel.partition_sched`属性，则会生成`POK_NEEDS_PRIO_PART_SCHED`宏，将控制流跳转到我们新增的`pok_partition_prio_sched`函数。由于FP与EDF均属于基于优先级的调度，因此我们把它们统一到一个hook入口中。`pok_partition_prio_sched`及我们实现的其他分区调度逻辑在`kernel/core/partition_sched.c`中。

由于POK原有分区数据结构并不具备我们修改后的分区调度模型所需的成员，因此我们采用与POK中其他数据结构类似的静态声明和初始化方法，在`partition_sched.c`中声明分区调度所需的数据结构：

```c
/**
 * @brief represents attributes of partitions which have been declared in 
 * configurations(config.yaml or deployment.h).
 * 
 */
typedef struct {
    uint64_t period;
    uint64_t complete_time;
    uint64_t deadline;
#if POK_CONFIG_PRIO_PART_SCHED == PART_SCHED_FP
    uint64_t prio;
#endif
} pok_partition_prio_attr_t;

/**
 * @brief represents a running instance of a partition. A partition has only
 one corresponding instance at any time. 
 * 
 * This instance may be runnable or non-runnable. Because we employ a periodic
 * model for partitions too, a partition would issue a "new instance" at the
 * beginning of each period. During a period, if this instance has not run out
 * of its time(complete_time in pok_partition_prio_attr_t), it would be consider-
 * ed as runnable. Otherwise, it's non-runnable. There are two possible scenarios
 * where an instance would be non-runnable. The first one is that an instance 
 * has completed its job in a period. The other one is that an instance is not done,
 * but it has passed its deadline when period > deadline.
 * 
 */
typedef struct {
    bool_t runnable;
    uint64_t next_arrival_time;
    uint64_t next_deadline;
    uint64_t last_sched_time;
    /* run time in a period, get cleared in a new period */
    uint64_t run_time;
} pok_partition_instance_t;

pok_partition_prio_attr_t pok_part_prio_attrs[POK_CONFIG_NB_PARTITIONS] = POK_CONFIG_PARTITIONS_PRIO_ATTRS;

pok_partition_instance_t pok_part_instances[POK_CONFIG_NB_PARTITIONS] = {0};
```

其中，`pok_partition_prio_attr_t`用于存储一个分区在配置文件中声明的分区调度相关属性，而`pok_partition_instance_t`则表示一个分区当前的运行实例，包括该分区的下一个周期的开始时间、下一个deadline、上一次被调度的时间，及当前周期内的运行时间。我们修改了`misc/gen_deployment`脚本，使其根据`config.yaml`中的内容生成`pok_part_prio_attrs`的初始化定义`POK_CONFIG_PARTITIONS_PRIO_ATTRS`。

分区调度的核心逻辑由`pok_partition_prio_sched_core`函数实现，该函数每次被调用时，首先检查当前时刻是否有某个分区进入了新的运行周期，只有此时，所有可运行分区间的优先级排序可能发生改变，抢占可能发生；否则，检查当前运行的分区是否已经完成运行或超过了它在本次周期内的deadline。若上述任意条件满足，调用`pok_partition_prio_elect`函数选择新的分区供运行，并调用`pok_partition_post_elect`更新新的分区所对应的实例状态。

如前所述，EDF和FP本质上都属于基于优先级的调度策略，因此我们通过`pok_partition_prio_sched_core`函数封装了大部分公共的调度逻辑，仅在选择下一个分区时，二者存在差异。在`pok_partition_prio_elect`函数中，我们根据`POK_CONFIG_PRIO_PART_SCHED`配置宏将控制流转发到EDF或FP调度函数，它们分别根据每个实例的`next_deadline`或每个分区的`prio`选择需要运行的分区。

```c
#if POK_CONFIG_PRIO_PART_SCHED == PART_SCHED_FP
static uint8_t pok_partition_fp_elect(void) {
    uint8_t elected_partition = POK_SCHED_CURRENT_PARTITION, cur_partition;
    pok_partition_instance_t *cur_instance, *elected_instance = &pok_part_instances[elected_partition];

    for (cur_partition = 0; cur_partition < POK_CONFIG_NB_PARTITIONS; cur_partition++) {
        cur_instance = &pok_part_instances[cur_partition];

        if (cur_instance->runnable) {
            if (!elected_instance->runnable) {
                elected_partition = cur_partition;
                elected_instance = cur_instance;
                continue;
            }

            if (pok_part_prio_attrs[cur_partition].prio > pok_part_prio_attrs[elected_partition].prio) {
                elected_partition = cur_partition;
                elected_instance = cur_instance;
            }
        }
    }

    return elected_partition;
}
#endif

#if POK_CONFIG_PRIO_PART_SCHED == PART_SCHED_EDF
static uint8_t pok_partition_edf_elect(void) {
    uint8_t elected_partition = POK_SCHED_CURRENT_PARTITION, cur_partition;
    pok_partition_instance_t *elected_instance = &pok_part_instances[elected_partition], *cur_instance;

    for (cur_partition = 0; cur_partition < POK_CONFIG_NB_PARTITIONS; cur_partition++) {
        cur_instance = &pok_part_instances[cur_partition];

        if (cur_instance->runnable) {
            if (!elected_instance->runnable) {
                elected_partition = cur_partition;
                elected_instance = cur_instance;
                continue;
            }

            if (cur_instance->next_deadline < elected_instance->next_deadline) {
                elected_partition = cur_partition;
                elected_instance = cur_instance;
            }
        }
    }

    return elected_partition;
}
#endif
```

## 多线程调度

为了实现新的线程调度算法，我们添加了新的`pok_sched_t`类型。根据程序的设置，调度函数会将partition的`sched_func`设置为对应的调度函数。

### 抢占式优先级调度

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

### 抢占式 EDF 调度

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

## 新场景调度

### 场景描述
我们寻找了一个工厂流水线次品分拣系统的场景，该场景中，系统需要通过实时监控视频对流水线上的商品进行识别，发现次品并通知机械臂对其进行分拣，还需要为工厂的工作人员提供实时的反馈。

具体而言，系统中包括三项任务：
1. 分拣任务：高优先级，否则次品会超出抓取的范围，导致错过截至时间。分拣失败。
2. 识别任务：中优先级，次品的分拣需要依赖首先对物体进行识别，优先级较高；物体从出现到离开抓取范围有一段的时间，任务可以被打断。
3. 管理任务：低优先级，将系统的实时状态反馈给管理员。

### 算法设计

由于该场景中存在高、中、低三类优先级线程，且高优先级与低优先级线程间存在依赖，因此可以归约为一个典型的存在优先级反转问题的场景。优先级反转指的是高优先级线程与低优先级线程间存在对某个资源的共享关系，且低优先级线程在高优先级线程之前获得了资源的独占访问权的情况。在这一场景下，由于操作系统不理解应用的语义，为了保证应用的正确性，操作系统只能让低优先级线程继续执行，而将高优先级进程休眠，从外界来看，就表现为低优先级线程反而成为了高优先级线程，二者的优先级“反转”了。

上述两类优先级线程间的优先级反转问题从操作系统的角度是无法完全避免的，这是因为操作系统不能改变应用程序的语义。但是，在这样的场景中，高优先级线程被阻塞的时间是可以预测的，即低优先级线程的关键区域长度。然而，一旦还存在优先级位于二者之间的中优先级线程，那么情况则会进一步恶化。由于中优先级线程的优先级高于低优先级线程，所以在前者完成执行前，后者都无法执行；然而，后者却拥有高优先级线程所依赖的资源的独占访问权，所以在存在三类优先级的情况下，一旦发生优先级反转，高优先级线程的阻塞时间将不再是低优先级线程的关键区域时长，而还要加上中优先级线程的运行时间。甚至在特定的场景下，中优先级线程会反复不断运行（因为此时高优先级线程已经休眠，而低优先级线程拥有更低的优先级），则高优先级线程被阻塞的时间将会达到任意长。

从操作系统的角度来看，缓解优先级反转问题，必须解决前述的三种优先级线程下的优先级反转问题。解决这一问题的思路也是非常直观的，既然高优先级线程必须在低优先级线程离开关键区域后才能继续执行，那么就应当让低优先级线程尽可能快地完成其关键区域；而低优先级线程又会被中优先级线程阻塞，所以我们可以临时提高低优先级线程的优先级至高于中优先级线程，让前者可以尽快完成关键区域，从而减少高优先级线程的等待时间。这种临时提高线程优先级的方案被称为优先级置顶（Priority Ceiling）。

我们实现了立即优先级置顶协议（Immediate Priority Ceiling Protocol，IPCP）。具体而言，每个共享资源（在实现中表现为mutex）都需要定义一个置顶值，其值应当设为所有可能竞争该资源的线程中的优先级最高者。当一个线程获得对该资源的独占访问时，**立即**将其优先级提升到该资源的置顶值，并在该线程停止访问该资源时，将其优先级恢复为它原先声明的优先级。

### 算法实现

我们在多线程优先级调度的基础上，实现了防止优先级无限制反转的IPCP算法。在`config.yaml`中，可以选择开启优先级置顶功能，并且需要正确定义每个mutex的置顶值，如下所示：

```yaml
partitions:
    - name: pr1
      features: [timer, console, libc, assert]
      prio_ceiling: true
      mutexes:
          - ceiling: 100
      scheduler: prio
```

如果将`prio_ceiling`属性设为true，则我们修改后的脚本会定义`POK_NEEDS_PRIO_CEILING`宏，从而开启条件编译。在`lockobj.c`中，我们修改mutex的上锁与放锁操作，根据条件调用优先级置顶与复原hook：

```c
#if defined (POK_NEEDS_PRIO_CEILING)
static void pok_lockobj_ceil_current_thread(pok_lockobj_t *obj) {
  pok_threads[POK_SCHED_CURRENT_THREAD].priority = obj->ceiling_value;
  printf("[DEBUG] ceil priority of thread %u to %u, original=%u\n", POK_SCHED_CURRENT_THREAD, obj->ceiling_value, pok_threads[POK_SCHED_CURRENT_THREAD].base_priority);
}

static void pok_lockobj_unceil_current_thread() {
  pok_threads[POK_SCHED_CURRENT_THREAD].priority = pok_threads[POK_SCHED_CURRENT_THREAD].base_priority;
  printf("[DEBUG] unceil priority of thread %u\n", POK_SCHED_CURRENT_THREAD);
}
#endif

pok_ret_t pok_lockobj_lock(pok_lockobj_t *obj,
                           const pok_lockobj_lockattr_t *attr) {
  if (obj->initialized == FALSE) {
    return POK_ERRNO_LOCKOBJ_NOTREADY;
  }
  SPIN_LOCK(obj->spin);

  if (obj->current_value > 0) {
    // Short path: object is available right now
    assert(pok_lockobj_fifo_is_empty(&obj->fifo));
    obj->current_value--;
#if defined (POK_NEEDS_PRIO_CEILING)
    pok_lockobj_ceil_current_thread(obj);
#endif
    SPIN_UNLOCK(obj->spin);
    return POK_ERRNO_OK;
  } else {
    uint64_t deadline =
        attr != NULL && attr->timeout > 0 ? attr->timeout + POK_GETTICK() : 0;
    pok_lockobj_enqueue(&obj->fifo, POK_SCHED_CURRENT_THREAD,
                        obj->queueing_policy);
    if (deadline > 0)
      pok_sched_lock_current_thread_timed(deadline);
    else
      pok_sched_lock_current_thread();

    SPIN_UNLOCK(obj->spin);
    pok_sched_thread(TRUE);
    SPIN_LOCK(obj->spin);

    if ((deadline != 0) && (POK_GETTICK() >= deadline)) {
      pok_lockobj_remove_thread(&obj->fifo, POK_SCHED_CURRENT_THREAD);
      SPIN_UNLOCK(obj->spin);
      return POK_ERRNO_TIMEOUT;
    } else {
#if defined (POK_NEEDS_PRIO_CEILING)
      pok_lockobj_ceil_current_thread(obj);
#endif
      SPIN_UNLOCK(obj->spin);
      return POK_ERRNO_OK;
    }
  }
}

pok_ret_t pok_lockobj_unlock(pok_lockobj_t *obj,
                             const pok_lockobj_lockattr_t *attr) {

  (void)attr; /* unused at this time */

  if (obj->initialized == FALSE) {
    return POK_ERRNO_LOCKOBJ_NOTREADY;
  }
  // Take the lock object internal lock
  SPIN_LOCK(obj->spin);

#if defined (POK_NEEDS_PRIO_CEILING)
    pok_lockobj_unceil_current_thread();
#endif

  /* ... */
}
```

需要注意，一个mutex在上锁时可能有两条得锁路径，即立即得锁或休眠后唤醒得锁，对这两条路径，我们均添加了对`pok_lockobj_ceil_current_thread` hook的调用。