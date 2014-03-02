/* 
 * File:   misc.h
 * Author: jts
 *
 * Created on 3 March 2014, 9:05 AM
 */

#ifndef MISC_H
#define	MISC_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef int bool;
#define true 1
#define false 0

extern unsigned long get_time_us();
extern int min(int a, int b);
extern int max(int a, int b);

#endif	/* MISC_H */

