# 测试报告

## 1 多线程调度

### 1.1 抢占式优先级调度

在`test-case/multi_thread_prio`中实现。

创建了三个线程
- thread1: prio = 99, time_capacity = 1
- thread2: prio = 50, time_capacity = 2
- thread3: prio = 20, time_capacity = 3

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
