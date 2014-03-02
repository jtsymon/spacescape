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
#include <stdarg.h>

typedef int bool;
#define true 1
#define false 0

#define wait_for_fucking_retarded_netbeans_terminal

extern void end(int status);

extern void vprint_err(const char* format, va_list args);
extern void print_err(const char* format, ...);
extern void fail(const char* format, ...);

extern unsigned long long get_time_us();
extern int min(int a, int b);
extern int max(int a, int b);

#endif	/* MISC_H */

