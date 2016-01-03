/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2016] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#ifndef LOADER_H_
#define LOADER_H_


#include "process/process_list.h"

void load_elf(proc_list_t * const list, const void * const data);

#endif /* LOADER_H_ */
