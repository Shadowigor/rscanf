
#ifndef RSCANF_H_
#define RSCANF_H_

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

int rvfscanf(FILE *file, char *fmt, va_list va);
int rfscnaf(FILE *file, char *fmt, ...);
int rscanf(char *fmt, ...);

#endif /* RSCANF_H_ */
