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
#ifndef ZAR_SRC_SYSTEM__H
#define ZAR_SRC_SYSTEM__H

#include <stdint.h>
#include <stdbool.h>

char* system_getcwd(char* out, size_t size);
int system_chdir(const char* path);
int system_mkdir(const char* path);
/** Similar to dirname(). Result must be free()'d. */
char* system_dirname(const char* path);
/** Similar to basename(). Result must be free()'d. */
const char* system_basename(const char* path);

/** Modifies path to ensure correct path separators.
 * Returns path.
 */
char* system_fix_pathseps(char* path);

bool system_isdir(const char* path);
void* system_opendir(const char* path);
/** Like strncpy() over the name of the next directory entry.
 * If end of directory: NULL is returned and result is untouched.
 */
char* system_readdir(void* dirhandle, char* result, size_t max);
void system_closedir(void* dirhandle);

#endif
