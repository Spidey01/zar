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

#define FUN(name) \
	void name (const char* fmt, ...) \
	{ \
		va_list args; \
		va_start(args, fmt); \
		fprintf(stderr, #name ":"); \
		vfprintf(stderr, fmt, args); \
		fputc('\n', stderr); \
		va_end(args); \
	}

FUN(debug)
FUN(info)
FUN(warn)

void error(int status, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	fprintf(stderr, "error:");
	vfprintf(stderr, fmt, args);
	fputc('\n', stderr);
	va_end(args);
	exit(status);
}
