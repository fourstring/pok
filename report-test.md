# 测试报告

## 1 多线程调度

### 1.1 抢占式优先级调度

在`test-case/multi_thread_prio`中实现。

创建了三个线程
- thread1: prio = 99, time_capacity = 100
- thread2: prio = 50, time_capacity = 200
- thread3: prio = 20, time_capacity = 300

运行结果如下：
```
Thread 0.1 scheduled at 60
...
Thread 0.1 running at 160
Thread 0.1 finished at 160, next activation: 1000
Thread 0.2 scheduled at 160
Thread 0.2 running at 180
...
Thread 0.2 running at 360
Thread 0.2 finished at 360, next activation: 1000
Thread 0.3 scheduled at 360
Thread 0.3 running at 380
...
Thread 0.3 running at 660
Thread 0.3 finished at 660, next activation: 1000
Thread 0.4 scheduled at 660
Thread 0.1 scheduled at 1000
...
```
可以看到优先级最高的`thread 0.1`先运行，它的时间片运行完后开始调度优先级第二的`thread 0.2`，接着调度`thread 0.3`。注意到，tick=1000的时候`thread 0.1`重新开始调度。

### 1.2 抢占式 EDF 调度

在`test-case/multi_thread_prio`中实现。

创建了三个线程
- thread1: deadline = 1000, time_capacity = 100
- thread2: deadline = 600, time_capacity = 200
- thread3: deadline = 500, time_capacity = 300

运行结果如下：

```
Thread 0.3 scheduled at 80
Thread 0.3 running at 100
...
Thread 0.3 running at 380
Thread 0.3 finished at 380, deadline met, next activation: 1000
Thread 0.2 scheduled at 380
Thread 0.2 running at 400
...
Thread 0.2 running at 580
Thread 0.2 finished at 580, deadline met, next activation: 1000
Thread 0.1 scheduled at 580
Thread 0.1 running at 600
...
Thread 0.1 running at 680
Thread 0.1 finished at 680, deadline met, next activation: 1000
Thread 0.4 scheduled at 680
```

可以看到，系统优先调度了优先级最高的`thread 0.3`线程，接着依次调度了`thread 0.2`和`thread 0.1`线程，所有线程都在deadline结束前完成。

### 1.3 Round-Robin 调度

在`test-case/multi_thread_rr`中实现。完整示例输出在该目录下保存为`output.log.txt`。

创建了三个线程，分别配置为：
- thread1: period = 1000, time_capacity = 500
- thread2: period = 1000, time_capacity = 200
- thread3: period = 1000, time_capacity = 100

运行结果如下：
```
POK kernel initialized
Thread 0.0 scheduled at 20
    SCHED_INFO: sched_time: 0 - 1, thread_id: 0, time_cap: -1, remain_cap: -1, state: 0
Thread 0.1 scheduled at 46
    SCHED_INFO: sched_time: 1 - 2, thread_id: 1, time_cap: 500, remain_cap: 500, state: 1
Thread 0.1 running at 60
Thread 0.2 scheduled at 60
    SCHED_INFO: sched_time: 2 - 3, thread_id: 2, time_cap: 200, remain_cap: 200, state: 1
Thread 0.2 running at 80
Thread 0.3 scheduled at 80
    SCHED_INFO: sched_time: 3 - 4, thread_id: 3, time_cap: 100, remain_cap: 100, state: 1
Thread 0.3 running at 100
Thread 0.1 scheduled at 100
    SCHED_INFO: sched_time: 4 - 5, thread_id: 1, time_cap: 500, remain_cap: 480, state: 1
Thread 0.1 running at 120
Thread 0.2 scheduled at 120
    SCHED_INFO: sched_time: 5 - 6, thread_id: 2, time_cap: 200, remain_cap: 180, state: 1
Thread 0.2 running at 140
Thread 0.3 scheduled at 140
    SCHED_INFO: sched_time: 6 - 7, thread_id: 3, time_cap: 100, remain_cap: 80, state: 1
Thread 0.3 running at 160
Thread 0.1 scheduled at 160
    SCHED_INFO: sched_time: 7 - 8, thread_id: 1, time_cap: 500, remain_cap: 460, state: 1
Thread 0.1 running at 180
Thread 0.2 scheduled at 180
    SCHED_INFO: sched_time: 8 - 9, thread_id: 2, time_cap: 200, remain_cap: 160, state: 1
Thread 0.2 running at 200
...


Thread 0.3 scheduled at 320
    SCHED_INFO: sched_time: 15 - 16, thread_id: 3, time_cap: 100, remain_cap: 20, state: 1
Thread 0.3 running at 340
Thread 0.3 finished at 340, next activation: 1000
Thread 0.1 scheduled at 340
    SCHED_INFO: sched_time: 16 - 17, thread_id: 1, time_cap: 500, remain_cap: 400, state: 1
Thread 0.1 running at 360
Thread 0.2 scheduled at 360
    SCHED_INFO: sched_time: 17 - 18, thread_id: 2, time_cap: 200, remain_cap: 100, state: 1
Thread 0.2 running at 380
Thread 0.1 scheduled at 380
    SCHED_INFO: sched_time: 18 - 19, thread_id: 1, time_cap: 500, remain_cap: 380, state: 1
Thread 0.1 running at 400
Thread 0.2 scheduled at 400
    SCHED_INFO: sched_time: 19 - 20, thread_id: 2, time_cap: 200, remain_cap: 80, state: 1
Thread 0.2 running at 420
Thread 0.1 scheduled at 420
    SCHED_INFO: sched_time: 20 - 21, thread_id: 1, time_cap: 500, remain_cap: 360, state: 1
Thread 0.1 running at 440
Thread 0.2 scheduled at 440
    SCHED_INFO: sched_time: 21 - 22, thread_id: 2, time_cap: 200, remain_cap: 60, state: 1
Thread 0.2 running at 460
Thread 0.1 scheduled at 460
    SCHED_INFO: sched_time: 22 - 23, thread_id: 1, time_cap: 500, remain_cap: 340, state: 1
Thread 0.1 running at 480
Thread 0.2 scheduled at 480
    SCHED_INFO: sched_time: 23 - 24, thread_id: 2, time_cap: 200, remain_cap: 40, state: 1
Thread 0.2 running at 500
Thread 0.1 scheduled at 500
    SCHED_INFO: sched_time: 24 - 25, thread_id: 1, time_cap: 500, remain_cap: 320, state: 1
Thread 0.1 running at 520
Thread 0.2 scheduled at 520
    SCHED_INFO: sched_time: 25 - 26, thread_id: 2, time_cap: 200, remain_cap: 20, state: 1
Thread 0.2 running at 540
Thread 0.2 finished at 540, next activation: 1000
Thread 0.1 scheduled at 540
    SCHED_INFO: sched_time: 26 - 27, thread_id: 1, time_cap: 500, remain_cap: 300, state: 1
Thread 0.1 running at 560
    SCHED_INFO: sched_time: 27 - 28, thread_id: 1, time_cap: 500, remain_cap: 280, state: 1
Thread 0.1 running at 580
    SCHED_INFO: sched_time: 28 - 29, thread_id: 1, time_cap: 500, remain_cap: 260, state: 1
Thread 0.1 running at 600
...


```

可以看到`thread 0.1`，`thread 0.2`，`thread 0.3`交替运行，直到`thread 0.3`最先运行完`time-capacity`后，`thread 0.1`，`thread 0.2`交替运行，直到`thread 0.2`也运行完`time-capacity`。注意到，tick=1000的时候`thread 0.1`，`thread 0.2`，`thread 0.3`重新开始调度。

### 1.4 Weighted-Round-Robin 调度

在`test-case/multi_thread_wrr`中实现。完整示例输出在该目录下保存为`output.log.txt`。

创建了三个线程，分别配置为：
- thread1: period = 1000, time_capacity = 500, weight = 2
- thread2: period = 1000, time_capacity = 200, weight = 1
- thread3: period = 1000, time_capacity = 100, weight = 1

运行结果如下：
```
POK kernel initialized
Thread 0.0 scheduled at 20
    SCHED_INFO: sched_time: 0 - 1, thread_id: 0, time_cap: -1, remain_cap: -1, state: 0
SQY@pok_sched_part_wrr trace: 825
Thread 0.1 scheduled at 48
    SCHED_INFO: sched_time: 1 - 2, thread_id: 1, time_cap: 500, remain_cap: 500, state: 1
Thread 0.1 running at 60
SQY@pok_sched_part_wrr trace: 825
    SCHED_INFO: sched_time: 2 - 3, thread_id: 1, time_cap: 500, remain_cap: 480, state: 1
Thread 0.1 running at 80
SQY@pok_sched_part_wrr trace: 825
Thread 0.2 scheduled at 80
    SCHED_INFO: sched_time: 3 - 4, thread_id: 2, time_cap: 200, remain_cap: 200, state: 1
Thread 0.2 running at 100
SQY@pok_sched_part_wrr trace: 825
Thread 0.3 scheduled at 100
    SCHED_INFO: sched_time: 4 - 5, thread_id: 3, time_cap: 100, remain_cap: 100, state: 1
Thread 0.3 running at 120
SQY@pok_sched_part_wrr trace: 825
Thread 0.1 scheduled at 120
    SCHED_INFO: sched_time: 5 - 6, thread_id: 1, time_cap: 500, remain_cap: 460, state: 1
Thread 0.1 running at 140
SQY@pok_sched_part_wrr trace: 825
    SCHED_INFO: sched_time: 6 - 7, thread_id: 1, time_cap: 500, remain_cap: 440, state: 1
Thread 0.1 running at 160
SQY@pok_sched_part_wrr trace: 825
Thread 0.2 scheduled at 160
    SCHED_INFO: sched_time: 7 - 8, thread_id: 2, time_cap: 200, remain_cap: 180, state: 1
Thread 0.2 running at 180
SQY@pok_sched_part_wrr trace: 825
Thread 0.3 scheduled at 180
    SCHED_INFO: sched_time: 8 - 9, thread_id: 3, time_cap: 100, remain_cap: 80, state: 1
Thread 0.3 running at 200
SQY@pok_sched_part_wrr trace: 825
Thread 0.1 scheduled at 200
    SCHED_INFO: sched_time: 9 - 10, thread_id: 1, time_cap: 500, remain_cap: 420, state: 1
Thread 0.1 running at 220
SQY@pok_sched_part_wrr trace: 825
    SCHED_INFO: sched_time: 10 - 11, thread_id: 1, time_cap: 500, remain_cap: 400, state: 1
Thread 0.1 running at 240
SQY@pok_sched_part_wrr trace: 825
Thread 0.2 scheduled at 240
    SCHED_INFO: sched_time: 11 - 12, thread_id: 2, time_cap: 200, remain_cap: 160, state: 1
Thread 0.2 running at 260
SQY@pok_sched_part_wrr trace: 825
...
```

可以看到`thread 0.1`，`thread 0.2`，`thread 0.3`交替运行，但`thread 0.1`每次连续执行两个调度最小单位（`time-slice`），而`thread 0.2`，`thread 0.3`只执行一个，这体现了权重的不同设置。其他特征均与 Round-Robin调度相同，详情请看`output.log.txt`。

## 2 动态线程创建

在`test-case/dynamic_thread_create`中实现。完整示例输出在该目录下保存为`output.log.txt`。

示例测试程序使用前文介绍的 Round-Robin 调度策略。在分区启动阶段，只创建两个线程，分别配置为：
- thread1: period = 1000, time_capacity = 200
- thread2: period = 1000, time_capacity = 100

线程2的payload如下代码所示，首先睡眠5s，之后创建一个新线程，紧接着进入循环。
```c
static void create_task() {
    uint32_t tid;
    pok_thread_attr_t tattr;
    memset(&tattr, 0, sizeof(pok_thread_attr_t));

    pok_thread_sleep(5);

    tattr.period = 1000;
    tattr.time_capacity = 500;
    tattr.entry = task;
    tattr.dynamic_created = TRUE;
    pok_thread_create(&tid, &tattr);

    for (;;) {
    }
}
```
被动态创建的线程的配置如下：
- thread3: period = 1000, time_capacity = 500


运行结果如下：
```
POK kernel initialized
Thread 0.0 scheduled at 20
    SCHED_INFO: sched_time: 0 - 1, thread_id: 0, time_cap: -1, remain_cap: -1, state: 0
Thread 0.1 scheduled at 45
    SCHED_INFO: sched_time: 1 - 2, thread_id: 1, time_cap: 200, remain_cap: 200, state: 1
Thread 0.1 running at 60
Thread 0.2 scheduled at 60
    SCHED_INFO: sched_time: 2 - 3, thread_id: 2, time_cap: 100, remain_cap: 100, state: 2
Thread 0.2 running at 66
    SCHED_INFO: sched_time: 3 - 4, thread_id: 1, time_cap: 200, remain_cap: 180, state: 1
Thread 0.1 running at 80
    SCHED_INFO: sched_time: 4 - 5, thread_id: 1, time_cap: 200, remain_cap: 160, state: 1
Thread 0.1 running at 100
    SCHED_INFO: sched_time: 5 - 6, thread_id: 1, time_cap: 200, remain_cap: 140, state: 1
Thread 0.1 running at 120
...


...
    SCHED_INFO: sched_time: 225 - 226, thread_id: 4, time_cap: -1, remain_cap: -1, state: 1
    SCHED_INFO: sched_time: 226 - 227, thread_id: 4, time_cap: -1, remain_cap: -1, state: 1
    SCHED_INFO: sched_time: 227 - 228, thread_id: 4, time_cap: -1, remain_cap: -1, state: 1
    SCHED_INFO: sched_time: 228 - 229, thread_id: 4, time_cap: -1, remain_cap: -1, state: 1
...


...
    SCHED_INFO: sched_time: 253 - 254, thread_id: 1, time_cap: 200, remain_cap: 140, state: 1
Thread 0.1 running at 5080
    SCHED_INFO: sched_time: 254 - 255, thread_id: 1, time_cap: 200, remain_cap: 120, state: 1
Thread 0.1 running at 5100
Thread 0.2 scheduled at 5100
    SCHED_INFO: sched_time: 255 - 256, thread_id: 2, time_cap: 100, remain_cap: 80, state: 1
Thread 0.2 running at 5120
Thread 0.3 scheduled at 5120
    SCHED_INFO: sched_time: 256 - 257, thread_id: 3, time_cap: 500, remain_cap: 500, state: 1
Thread 0.3 running at 5140
Thread 0.1 scheduled at 5140
    SCHED_INFO: sched_time: 257 - 258, thread_id: 1, time_cap: 200, remain_cap: 100, state: 1
Thread 0.1 running at 5160
Thread 0.2 scheduled at 5160
    SCHED_INFO: sched_time: 258 - 259, thread_id: 2, time_cap: 100, remain_cap: 60, state: 1
Thread 0.2 running at 5180
Thread 0.3 scheduled at 5180
    SCHED_INFO: sched_time: 259 - 260, thread_id: 3, time_cap: 500, remain_cap: 480, state: 1
Thread 0.3 running at 5200
Thread 0.1 scheduled at 5200
    SCHED_INFO: sched_time: 260 - 261, thread_id: 1, time_cap: 200, remain_cap: 80, state: 1
Thread 0.1 running at 5220
Thread 0.2 scheduled at 5220
    SCHED_INFO: sched_time: 261 - 262, thread_id: 2, time_cap: 100, remain_cap: 40, state: 1
Thread 0.2 running at 5240
Thread 0.3 scheduled at 5240
    SCHED_INFO: sched_time: 262 - 263, thread_id: 3, time_cap: 500, remain_cap: 460, state: 1
Thread 0.3 running at 5260
Thread 0.1 scheduled at 5260
    SCHED_INFO: sched_time: 263 - 264, thread_id: 1, time_cap: 200, remain_cap: 60, state: 1
Thread 0.1 running at 5280
Thread 0.2 scheduled at 5280
    SCHED_INFO: sched_time: 264 - 265, thread_id: 2, time_cap: 100, remain_cap: 20, state: 1
Thread 0.2 running at 5300
Thread 0.2 finished at 5300, next activation: 1000
Thread 0.3 scheduled at 5300
    SCHED_INFO: sched_time: 265 - 266, thread_id: 3, time_cap: 500, remain_cap: 440, state: 1
Thread 0.3 running at 5320
```

可以看到`thread 0.2`在第一次被调度后立即进入睡眠，只剩`thread 0.1`独自运行，在`thread 0.1`的time-capacity用完后，idle线程（即`thread 0.4`）开始运行直到周期结束。

直到5s后（`5000 ticks`），`thread 0.2`醒来被调度，然后创建了`thread 0.3`开始运行、参与调度。之后的运行特征均与 Round-Robin调度相同，`thread 0.1`，`thread 0.2`，`thread 0.3`交替运行，详情请看`output.log.txt`。
