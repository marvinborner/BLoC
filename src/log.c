// Copyright (c) 2023, Marvin Borner <dev@marvinborner.de>
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include <log.h>

static int debug_enabled = 0;

void debug(const char *format, ...)
{
	if (!debug_enabled)
		return;

	fprintf(stderr, "[DEBUG] BLoC: ");

	va_list ap;
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
}

void debug_enable(int enable)
{
	debug_enabled = enable;
}

void fatal(const char *format, ...)
{
	fprintf(stderr, "[FATAL] BLoC: ");

	va_list ap;
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);

	abort();
}
