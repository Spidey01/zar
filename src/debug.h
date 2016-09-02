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
#ifndef ZAR_SRC_DEBUG__H
#define ZAR_SRC_DEBUG__H

#include <stdarg.h>

extern int debug_level;

void debug_printf(int level, const char* fmt, ...);
void debug_vprintf(int level, const char* fmt, va_list vargs);
void error(int status, const char* fmt, ...);


enum {
	DEBUG_error,
	DEBUG_warn,
	DEBUG_info,
	DEBUG_debug,
	DEBUG_xtrace
};


#define xtrace(fmt, ...) debug_printf(DEBUG_xtrace, fmt, ##__VA_ARGS__);
#define debug(fmt, ...) debug_printf(DEBUG_debug, fmt, ##__VA_ARGS__);
#define info(fmt, ...) debug_printf(DEBUG_info, fmt, ##__VA_ARGS__);
#define warn(fmt, ...) debug_printf(DEBUG_warn, fmt, ##__VA_ARGS__);

#endif
