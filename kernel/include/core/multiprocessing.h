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
 *                                      Copyright (c) 2007-2020 POK team
 */

/**
 * \file    kernel/include/core/multiprocessing.h
 * \author  Romain Guilloteau
 * \date    2020
 * \brief   Generic interface to handle multiprocessing
 */

#ifndef __POK_MULTIPROCESSING_H__
#define __POK_MULTIPROCESSING_H__
#include <types.h>

extern uint8_t multiprocessing_system;
uint8_t pok_get_proc_id(void);

#ifdef POK_ARCH_PPC
#endif

#ifdef POK_ARCH_X86
#include <arch/x86/multiprocessing.h>
#endif

#ifdef POK_ARCH_SPARC
#endif

#endif /* !__POK_MULTIPROCESSING_H__ */
