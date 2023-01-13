# 测试报告

## 多分区调度

### 抢占式优先级调度

在`test-case/partitions_fp`中实现。

创建两个分区：

```yaml
kernel:
    features: [debug]
    partition_sched: fp
    scheduler:
        slots:
            - partition: pr1
              period: 3s
              complete_time: 2s
              deadline: 3s
              priority: 1
            - partition: pr2
              period: 3s
              complete_time: 1s
              deadline: 3s
              priority: 2
```

运行结果如下：

```
[P2] thread create returns=0
P2T1: now 1382850 ns, 0 s
P2T1: now 102026673 ns, 0 s
P2T1: now 203011599 ns, 0 s
P2T1: now 304005744 ns, 0 s
P2T1: now 405018327 ns, 0 s
P2T1: now 506003253 ns, 0 s
P2T1: now 607015836 ns, 0 s
P2T1: now 708000762 ns, 0 s
P2T1: now 809004126 ns, 0 s
P2T1: now 910007490 ns, 0 s
[P1] pok_thread_create (1) return=0
P1T1: now 1305908226 ns, 1 s
P1T1: now 1407022218 ns, 1 s
P1T1: now 1508007144 ns, 1 s
P1T1: now 1609001289 ns, 1 s
P1T1: now 1710004653 ns, 1 s
P1T1: now 1811017236 ns, 1 s
P1T1: now 1912002162 ns, 1 s
P1T1: now 2013005526 ns, 2 s
P1T1: now 2114008890 ns, 2 s
P1T1: now 2215003035 ns, 2 s
P1T1: now 2316006399 ns, 2 s
P1T1: now 2417000544 ns, 2 s
P1T1: now 2518022346 ns, 2 s
P1T1: now 2619016491 ns, 2 s
P1T1: now 2720001417 ns, 2 s
P1T1: now 2821014000 ns, 2 s
P1T1: now 2922008145 ns, 2 s
P2T1: now 3000010104 ns, 3 s
P2T1: now 3101004249 ns, 3 s
P2T1: now 3202007613 ns, 3 s
P2T1: now 3303001758 ns, 3 s
P2T1: now 3404005122 ns, 3 s
P2T1: now 3505008486 ns, 3 s
P2T1: now 3606002631 ns, 3 s
P2T1: now 3707005995 ns, 3 s
P2T1: now 3808000140 ns, 3 s
P2T1: now 3909012723 ns, 3 s
P1T1: now 4000013472 ns, 4 s
P1T1: now 4101016836 ns, 4 s
P1T1: now 4202001762 ns, 4 s
P1T1: now 4303005126 ns, 4 s
P1T1: now 4404008490 ns, 4 s
P1T1: now 4505011854 ns, 4 s
P1T1: now 4606005999 ns, 4 s
P1T1: now 4707000144 ns, 4 s
P1T1: now 4808003508 ns, 4 s
P1T1: now 4909006872 ns, 4 s
P1T1: now 5010001017 ns, 5 s
P1T1: now 5111004381 ns, 5 s
P1T1: now 5212007745 ns, 5 s
P1T1: now 5313001890 ns, 5 s
P1T1: now 5414005254 ns, 5 s
P1T1: now 5515008618 ns, 5 s
P1T1: now 5616002763 ns, 5 s
P1T1: now 5717006127 ns, 5 s
P1T1: now 5818018710 ns, 5 s
P1T1: now 5919012855 ns, 5 s
P2T1: now 6000010989 ns, 6 s
P2T1: now 6101014353 ns, 6 s
P2T1: now 6202008498 ns, 6 s
P2T1: now 6303002643 ns, 6 s
P2T1: now 6404006007 ns, 6 s
P2T1: now 6505000152 ns, 6 s
P2T1: now 6606003516 ns, 6 s
P2T1: now 6707006880 ns, 6 s
P2T1: now 6808001025 ns, 6 s
P2T1: now 6909004389 ns, 6 s
```

可以看到，在每个3秒周期内，优先级较高的分区2均首先运行1秒到完成当周期运行后，优先级较低的分区1才能运行剩余的2秒。

### 抢占式EDF调度

在`test-case/partitions_edf`中实现。

定义三个分区：

```yaml
kernel:
    features: [debug]
    partition_sched: edf
    scheduler:
        slots:
            - partition: pr1
              period: 6s
              complete_time: 1s
              deadline: 6s
            - partition: pr2
              period: 4s
              complete_time: 2s
              deadline: 4s
            - partition: pr2
              period: 3s
              complete_time: 1s
              deadline: 3s
```

运行结果如下：

```
[P2] thread create returns=0
P3T1: now 1401288 ns, 0 s
P3T1: now 102045111 ns, 0 s
P3T1: now 203011599 ns, 0 s
P3T1: now 304014963 ns, 0 s
P3T1: now 405027546 ns, 0 s
P3T1: now 506012472 ns, 0 s
P3T1: now 607015836 ns, 0 s
P3T1: now 708019200 ns, 0 s
P3T1: now 809013345 ns, 0 s
P3T1: now 910016709 ns, 0 s
[P2] thread create returns=0
P2T1: now 1001579817 ns, 1 s
P2T1: now 1103034912 ns, 1 s
P2T1: now 1204001400 ns, 1 s
P2T1: now 1305004764 ns, 1 s
P2T1: now 1406008128 ns, 1 s
P2T1: now 1507002273 ns, 1 s
P2T1: now 1608005637 ns, 1 s
P2T1: now 1709009001 ns, 1 s
P2T1: now 1810003146 ns, 1 s
P2T1: now 1911006510 ns, 1 s
P2T1: now 2012000655 ns, 2 s
P2T1: now 2113013238 ns, 2 s
P2T1: now 2214016602 ns, 2 s
P2T1: now 2315010747 ns, 2 s
P2T1: now 2416004892 ns, 2 s
P2T1: now 2517017475 ns, 2 s
P2T1: now 2618011620 ns, 2 s
P2T1: now 2719005765 ns, 2 s
P2T1: now 2820009129 ns, 2 s
P2T1: now 2921003274 ns, 2 s
[P1] pok_thread_create (1) return=0
P1T1: now 3441397386 ns, 3 s
P1T1: now 3542031990 ns, 3 s
P1T1: now 3643007697 ns, 3 s
P1T1: now 3744011061 ns, 3 s
P1T1: now 3845005206 ns, 3 s
P1T1: now 3946008570 ns, 3 s
P3T1: now 4001027562 ns, 4 s
P3T1: now 4102012488 ns, 4 s
P3T1: now 4203006633 ns, 4 s
P3T1: now 4304000778 ns, 4 s
P3T1: now 4405004142 ns, 4 s
P3T1: now 4506016725 ns, 4 s
P3T1: now 4607010870 ns, 4 s
P3T1: now 4708014234 ns, 4 s
P3T1: now 4809008379 ns, 4 s
P3T1: now 4910002524 ns, 4 s
P2T1: now 5002017363 ns, 5 s
P2T1: now 5103011508 ns, 5 s
P2T1: now 5204005653 ns, 5 s
P2T1: now 5305009017 ns, 5 s
P2T1: now 5406003162 ns, 5 s
P2T1: now 5507015745 ns, 5 s
P2T1: now 5608009890 ns, 5 s
P2T1: now 5709004035 ns, 5 s
P2T1: now 5810007399 ns, 5 s
P2T1: now 5911001544 ns, 5 s
P2T1: now 6012004908 ns, 6 s
P2T1: now 6113008272 ns, 6 s
P2T1: now 6214002417 ns, 6 s
P2T1: now 6315015000 ns, 6 s
P2T1: now 6416009145 ns, 6 s
P2T1: now 6517003290 ns, 6 s
P2T1: now 6618006654 ns, 6 s
P2T1: now 6719010018 ns, 6 s
P2T1: now 6820004163 ns, 6 s
P2T1: now 6921007527 ns, 6 s
P3T1: now 7003010532 ns, 7 s
P3T1: now 7104004677 ns, 7 s
P3T1: now 7205008041 ns, 7 s
P3T1: now 7306002186 ns, 7 s
P3T1: now 7407005550 ns, 7 s
P3T1: now 7508008914 ns, 7 s
P3T1: now 7609012278 ns, 7 s
P3T1: now 7710006423 ns, 7 s
P3T1: now 7811000568 ns, 7 s
P3T1: now 7912003932 ns, 7 s
P1T1: now 8003013900 ns, 8 s
P1T1: now 8104026483 ns, 8 s
P1T1: now 8205020628 ns, 8 s
P1T1: now 8306014773 ns, 8 s
P1T1: now 8407008918 ns, 8 s
P1T1: now 8508003063 ns, 8 s
P1T1: now 8609006427 ns, 8 s
P1T1: now 8710009791 ns, 8 s
P1T1: now 8811022374 ns, 8 s
P1T1: now 8912016519 ns, 8 s
```

可以看到，分区3由于周期最短，所以在0秒时，其deadline也最短，因此第一个运行，随后是分区2和分区3.另外可以注意到，在第7秒，分区2完成第2次运行后，分区3的第3次运行的deadline（第9秒）小于分区2第3次运行（第12秒）和分区1第2次运行（第12秒），因此分区3抢占分区1、2执行，符合EDF调度策略。

### RR和WRR调度

## 多线程调度

### 抢占式优先级调度

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

### 抢占式 EDF 调度

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

## 新场景调度

在`test-case/multi_thread_ipcp`中实现。

定义了三个线程，其中高优先级线程优先级为100，中优先级线程为50，低优先级线程为1.为了模拟优先级反转场景，我们让高优先级线程在第5秒开始运行，中优先级线程在第3秒开始运行，低优先级线程立刻开始运行。

在未启用优先级置顶协议时，低优先级线程将会立刻尝试获取与高优先级线程共享的资源访问权（锁），随后中优先级线程开始运行，高优先级线程将会被二者阻塞到第15秒才开始运行，如下所示：

```
pok_mutex_create return=0, mid=0
low_prio_job: mutex lock, ret=0
low_prio_job: I'm alive at 501559695
low_prio_job: I'm alive at 1001561379
low_prio_job: I'm alive at 1501563063
low_prio_job: I'm alive at 2001564747
low_prio_job: I'm alive at 2501566431
low_prio_job: I'm alive at 3001568115
medium_prio_job: I'm alive at 3502030749
medium_prio_job: I'm alive at 4002032433
medium_prio_job: I'm alive at 4502034117
medium_prio_job: I'm alive at 5002035801
medium_prio_job: I'm alive at 5502037485
medium_prio_job: I'm alive at 6002039169
medium_prio_job: I'm alive at 6502040853
medium_prio_job: I'm alive at 7002042537
medium_prio_job: I'm alive at 7502044221
medium_prio_job: I'm alive at 8002045905
low_prio_job: I'm alive at 8002101219
low_prio_job: I'm alive at 8502102903
low_prio_job: I'm alive at 9002104587
low_prio_job: I'm alive at 9502106271
low_prio_job: I'm alive at 10002107955
low_prio_job: I'm alive at 10502109639
low_prio_job: I'm alive at 11002111323
low_prio_job: I'm alive at 11502113007
low_prio_job: I'm alive at 12002114691
low_prio_job: I'm alive at 12502116375
low_prio_job: I'm alive at 13002118059
low_prio_job: I'm alive at 13502119743
low_prio_job: I'm alive at 14002121427
low_prio_job: I'm alive at 14502123111
high_prio_job: mutex lock, ret=0
high_prio_job: I'm alive at 14502215301 ns
high_prio_job: mutex unlock, ret=0
low_prio_job: mutex unlock, ret=0
```

启用优先级置顶协议后，低优先级线程在获取锁后优先级将会被立即提升到100，高优先级线程在第10秒即可开始运行，不会被中优先级线程阻塞，如下所示：

```
[DEBUG] set lock obj 0 ceiling_value to 100
pok_mutex_create return=0, mid=0
[DEBUG] ceil priority of thread 3 to 100, original=1
low_prio_job: mutex lock, ret=0
low_prio_job: I'm alive at 501587352
low_prio_job: I'm alive at 1001589036
low_prio_job: I'm alive at 1501590720
low_prio_job: I'm alive at 2001592404
low_prio_job: I'm alive at 2501594088
low_prio_job: I'm alive at 3001595772
low_prio_job: I'm alive at 3501597456
low_prio_job: I'm alive at 4001599140
low_prio_job: I'm alive at 4501600824
low_prio_job: I'm alive at 5001602508
low_prio_job: I'm alive at 5501604192
low_prio_job: I'm alive at 6001605876
low_prio_job: I'm alive at 6501607560
low_prio_job: I'm alive at 7001609244
low_prio_job: I'm alive at 7501610928
low_prio_job: I'm alive at 8001612612
low_prio_job: I'm alive at 8501614296
low_prio_job: I'm alive at 9001615980
low_prio_job: I'm alive at 9501617664
low_prio_job: I'm alive at 10001619348
[DEBUG] unceil priority of thread 3
low_prio_job: mutex unlock, ret=0
[DEBUG] ceil priority of thread 1 to 100, original=100
high_prio_job: mutex lock, ret=0
high_prio_job: I'm alive at 10001794509 ns
[DEBUG] unceil priority of thread 1
high_prio_job: mutex unlock, ret=0
medium_prio_job: I'm alive at 10501897602
medium_prio_job: I'm alive at 11001899286
medium_prio_job: I'm alive at 11501900970
medium_prio_job: I'm alive at 12001902654
medium_prio_job: I'm alive at 12501904338
medium_prio_job: I'm alive at 13001906022
medium_prio_job: I'm alive at 13501907706
medium_prio_job: I'm alive at 14001909390
medium_prio_job: I'm alive at 14501911074
medium_prio_job: I'm alive at 15001912758
```