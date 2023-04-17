// Copyright (c) 2023, Marvin Borner <dev@marvinborner.de>
// SPDX-License-Identifier: MIT

#ifndef BLOC_LOG_H
#define BLOC_LOG_H

void debug(const char *format, ...);
void debug_enable(int enable);
void fatal(const char *format, ...) __attribute__((noreturn));

#endif
