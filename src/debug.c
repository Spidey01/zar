/*
 * Copyright 2016-current Terry Mathew Poulin <BigBoss1964@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

enum {
	DEBUG_error,
	DEBUG_warn,
	DEBUG_info,
	DEBUG_debug,
	DEBUG_xtrace
};

int debug_level = DEBUG_warn;


static void debug_printf(int level, const char* fmt, ...)
{
	if (debug_level < level)
		return;

	const char* p;
	switch (level) {
		case DEBUG_error:
			p = "error:";
			break;
		case DEBUG_warn:
			p = "warn:";
			break;
		case DEBUG_info:
			p = "info:";
			break;
		case DEBUG_debug:
			p = "debug:";
			break;
		case DEBUG_xtrace:
			p = "xtrace:";
			break;
		default:
			p = "wtf:";
			break;
	}
	fprintf(stderr, "%s", p);

	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	fputc('\n', stderr);
	va_end(args);
}

#define FUN(name) \
	void name (const char* fmt, ...) \
	{ \
		va_list args; \
		va_start(args, fmt); \
		debug_printf(DEBUG_##name, fmt, args); \
		va_end(args); \
	}

FUN(xtrace)
FUN(debug)
FUN(info)
FUN(warn)

void error(int status, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	debug_printf(DEBUG_error, fmt, args);
	va_end(args);
	exit(status);
}
