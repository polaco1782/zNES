/* 
 * File:   joystick.h
 * Author: cassiano
 *
 * Created on January 31, 2016, 7:23 PM
 */

#include <stdio.h>
#include <stdint.h>

#ifndef JOYSTICK_H
#define	JOYSTICK_H

#ifdef	__cplusplus
extern "C" {
#endif

void joy_writereg(uint8_t joy, uint8_t value);
uint8_t joy_readreg(uint8_t joy);


#ifdef	__cplusplus
}
#endif

#endif	/* JOYSTICK_H */

