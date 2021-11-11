#ifndef __OCARINA_GENERATED_DEPLOYMENT_H_
#define __OCARINA_GENERATED_DEPLOYMENT_H_
#include <core/schedvalues.h>
/*****************************************************/

/*  This file was automatically generated by Ocarina */

/*  Do NOT hand-modify this file, as your            */

/*  changes will be lost when you re-run Ocarina     */

/*****************************************************/

#define POK_NEEDS_CONSOLE 1

#define POK_NEEDS_CONSOLE 1

#define POK_CONFIG_LOCAL_NODE 0

#define POK_GENERATED_CODE 1

/*  The POK_LOCAL_NODE macro corresponds to the identifier for this node in */
/*  the distributed system. This identifier is unique for each nodeIn this */
/*  case, this identifier was deduced from thecpu processor component.*/

#define POK_NEEDS_GETTICK 1

#define POK_NEEDS_DEBUG 1

/*  If you want to activate DEBUG mode, uncomment previous line*/

#define POK_CONFIG_NB_PARTITIONS 2

/*  The maccro POK_CONFIG_NB_PARTITIONS indicates the amount of partitions in*/
/*   the current system.It corresponds to the amount of process componentsin */
/*  the system.*/

#define POK_CONFIG_NB_THREADS 8

/*  The maccro POK_CONFIG_NB_THREADS indicates the amount of threads used in */
/*  the kernel.It comprises the tasks for the partition and the main task of */
/*  each partition that initialize all ressources.*/

#define POK_CONFIG_PARTITIONS_NTHREADS                                         \
  { 3, 3 }

/*  The maccro POK_CONFIG_NB_PARTITIONS_NTHREADS indicates the amount of */
/*  threads in each partition we also add an additional process that */
/*  initialize all partition's entities (communication, threads, ...)*/

#define POK_NEEDS_SCHED_RR 1

#define POK_CONFIG_PARTITIONS_SIZE                                             \
  { 80000, 80000 }

#define POK_CONFIG_PARTITIONS_SCHEDULER                                        \
  { POK_SCHED_RR, POK_SCHED_RR }

/*  The maccro POK_CONFIG_PARTTITIONS_SIZE indicates the required amount of */
/*  memory for each partition.This value was deduced from the property */
/*  POK::Needed_Memory_Size of each process*/

#define POK_CONFIG_SCHEDULING_SLOTS                                            \
  { 500000000, 500000000 }

#define POK_CONFIG_SCHEDULING_SLOTS_ALLOCATION                                 \
  { 0, 1 }

#define POK_CONFIG_SCHEDULING_NBSLOTS 2

#define POK_CONFIG_SCHEDULING_MAJOR_FRAME 1000000000

#define POK_NEEDS_PORTS_SAMPLING 1

/*  The maccro POK_NEEDS_PORTS_SAMPLING indicates that we need sampling ports*/
/*   facilities in POK.It also means that we have intra-partition */
/*  communication between several processes through event data port.*/

#define POK_CONFIG_STACKS_SIZE 16384

#define POK_CONFIG_PARTITIONS_PORTS                                            \
  { 0, 1 }

#define POK_CONFIG_NB_PORTS 2

#define POK_CONFIG_NB_GLOBAL_PORTS 2

/*  The maccro POK_CONFIG_NB_PORTS represent the amount of inter-partition */
/*  communication in the system.Sampling and Queueing ports are counted.*/

typedef enum {
  pr2_pdatain = 0,
  pr1_pdataout = 1,
  invalid_local_port = 2
} pok_port_local_identifier_t;

typedef enum { cpu = 0 } pok_node_identifier_t;

typedef enum {
  pr2_pdatain_global = 0,
  pr1_pdataout_global = 1
} pok_port_identifier_t;

/*  This enumeration describes all ports on the current nodes. It is used by */
/*  the configuration arrays in the generated file deployment.c.*/

#define POK_CONFIG_NB_BUSES 0

typedef enum { invalid_bus = 0 } pok_bus_identifier_t;

#define POK_CONFIG_NB_NODES 1

#define POK_USE_GENERATED_PARTITION_ERROR_HANDLER 1

#define POK_USE_GENERATED_KERNEL_ERROR_HANDLER 1

#define POK_USE_GENERATED_KERNEL_ERROR_CALLBACK 1

#endif
