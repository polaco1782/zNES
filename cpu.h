/* 
 * File:   cpu.h
 * Author: cassiano
 *
 * Created on January 23, 2016, 10:12 PM
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef CPU_H
#define	CPU_H

#ifdef	__cplusplus
extern "C" {
#endif

void cpu_reset();
void cpu_nmi_notify();
void cpu_stall_notify(uint32_t value);
uint32_t cpu_execop();


#ifdef	__cplusplus
}
#endif

#endif	/* CPU_H */

