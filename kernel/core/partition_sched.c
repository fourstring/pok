#include <types.h>
#include <core/sched.h>
#include <libc.h>

#if defined (POK_CONFIG_PRIO_PART_SCHED)

#define PART_SCHED_FP 1
#define PART_SCHED_EDF 2

#if defined (POK_NEEDS_DEBUG_PART_PRIO)
#define debug_log(format, ...) printf("[DEBUG] %s,%d: " format, __func__, __LINE__, ##__VA_ARGS__)
#else
#define debug_log(...)
#endif

uint64_t pok_part_next_arrival_time = 0;
uint64_t pok_part_next_complete_time = 0;

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

static uint8_t pok_partition_prio_elect() {
#if POK_CONFIG_PRIO_PART_SCHED == PART_SCHED_FP
    return pok_partition_fp_elect();
#elif POK_CONFIG_PRIO_PART_SCHED == PART_SCHED_EDF
    return pok_partition_edf_elect();
#else
#ifdef POK_NEEDS_DEBUG
    printf("No priority-based partition scheduling algorithm is specified.\n");
#endif
    return POK_SCHED_CURRENT_PARTITION;
#endif
}

static void pok_partition_post_elect(uint8_t elected_partition, uint64_t now) {
    uint8_t old_partition = POK_SCHED_CURRENT_PARTITION;
    pok_partition_instance_t *old_instance, *elected_instance;

    debug_log("now: %llu, old_partition=%u, elected_partition=%u\n", now, old_partition, elected_partition);

    old_instance = &pok_part_instances[old_partition];
    elected_instance = &pok_part_instances[elected_partition];

    if (old_partition != elected_partition) {
        old_instance->run_time += (now - old_instance->last_sched_time);
        debug_log("old instance run_time=%llu\n", old_instance->run_time);
        /**
        * @pok_part_next_complete_time is the completion time of currently running instance.
        * So when a new instance is elected, @pok_part_next_complete_time should be updated
        * to the time when the new instance keeps running without any preemption.
        * 
        */
        pok_part_next_complete_time = now + (pok_part_prio_attrs[elected_partition].complete_time - elected_instance->run_time);

        debug_log("pok_part_next_complete_time=%llu\n", pok_part_next_complete_time);
    }

    elected_instance->last_sched_time = now;
}

static void pok_arrived_partition_reset(uint8_t partition) {
    pok_partition_instance_t *new_instance = &pok_part_instances[partition];
    new_instance->next_deadline = new_instance->next_arrival_time + pok_part_prio_attrs[partition].deadline;
    new_instance->next_arrival_time += pok_part_prio_attrs[partition].period;
    new_instance->run_time = 0;
    debug_log("next_deadline=%llu, next_arrival_time=%llu\n", new_instance->next_deadline, new_instance->next_arrival_time);
}

static uint8_t pok_partition_prio_sched_core(uint64_t now) {
    uint8_t elected_partition = POK_SCHED_CURRENT_PARTITION;
    uint8_t old_partition = POK_SCHED_CURRENT_PARTITION;
    uint8_t cur_partition;
    uint64_t cur_arrival_time, next_arrival_time;
    pok_partition_instance_t *cur_instance;
    pok_partition_instance_t *old_instance = &pok_part_instances[old_partition];

    if (pok_part_next_arrival_time <= now) {
        /**
         * Some partitions issued new instances, consider partition-level preemption here.
         */
        for (cur_partition = 0; cur_partition < POK_CONFIG_NB_PARTITIONS; cur_partition++) {
            cur_instance = &pok_part_instances[cur_partition];
            cur_arrival_time = cur_instance->next_arrival_time;
            /**
             * Every partition whose arrival time is between @pok_part_next_arrival_time and
             * @now should be considered as it have issued a new instance.
             */
            debug_log("now: %llu, cur_partition=%u, cur_arrival_time=%llu\n", now, cur_partition, cur_arrival_time);
            if (pok_part_next_arrival_time <= cur_arrival_time && cur_arrival_time <= now) {
                pok_arrived_partition_reset(cur_partition);
                /**
                 * If @now <= next deadline of current new instance, this instance should
                 * be runnable in its current period.
                 * Generally, this if statement should always take the true branch, because
                 * we assume that the POK_GETTIME would return continuous ticks or there is
                 * no glitches.
                 */
                if (now <= cur_instance->next_deadline) {
                    debug_log("now: %llu, partition=%u has been runnanle\n", now, cur_partition);
                    cur_instance->runnable = TRUE;
                } else {
                    cur_instance->runnable = FALSE;
                }
            }
        }

        elected_partition = pok_partition_prio_elect();

        pok_partition_post_elect(elected_partition, now);

        /**
         * Update @pok_part_next_arrival_time here. @pok_part_next_arrival_time is the
         * most recent arrival time of a new instance. So this value is MIN(instances.
         * next_arrival_time). And it should only be updated after @next_arrival_time of
         * some instance is changed. The latter one would only happen due to some partition
         * issues new instances which replaced the old ones, or just in this branch.
         * 
         */
        next_arrival_time = pok_part_instances[0].next_arrival_time;
        for (cur_partition = 1; cur_partition < POK_CONFIG_NB_PARTITIONS; cur_partition++) {
            cur_instance = &pok_part_instances[cur_partition];
            if (cur_instance->next_arrival_time < next_arrival_time) {
                next_arrival_time = cur_instance->next_arrival_time;
            }
        }
        pok_part_next_arrival_time = next_arrival_time;
        debug_log("pok_part_next_arrival_time=%llu\n", pok_part_next_arrival_time);
    } else if (pok_part_next_complete_time <= now ||
                (now != 0 && old_instance->next_deadline <= now)) {
        /**
         * An instance was run successfully or passed its deadline, so 
         * we have to choose another instance.
         */
        
        /**
         * Obviously, the old instance should not be scheduled in this period.
         */
        old_instance->runnable = FALSE;

        debug_log("now: %llu, partition=%u has been non-runnanle\n", now, old_partition);

        elected_partition = pok_partition_prio_elect();

        pok_partition_post_elect(elected_partition, now);
    }

    debug_log("now: %llu, [DONE] elected_partition=%u\n\n\n", now, elected_partition);
    return elected_partition;
}

uint8_t pok_partition_prio_sched(uint64_t now) {
#if defined (POK_CONFIG_PRIO_PART_SCHED)
    return pok_partition_prio_sched_core(now);
#else
#ifdef POK_NEEDS_DEBUG
    printf("No priority-based partition scheduling algorithm is specified.\n");
#endif
    (void) now;
    return POK_SCHED_CURRENT_PARTITION;
#endif
}

#endif