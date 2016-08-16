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

#include "system.h"
#include "io.h"

#ifdef _WIN32
#include <direct.h>
#define chdir(s) _chdir(s)
#define getcwd(buffer, length) _getcwd(buffer, length)
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

#include <errno.h>
#include <stdlib.h>
#include <string.h>

char* system_getcwd(char* out, size_t size)
{
	return getcwd(out, size);
}


int system_chdir(const char* path)
{
	return chdir(path);
}


static int wrapped_mkdir(const char* path)
{
	int status = -1;
#if _WIN32
	status =  _mkdir(path);
#else
	status =  mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO);
#endif
	if (status != 0 && errno == EEXIST)
		return 0;
	else
		return status;
}


int system_mkdir(const char* path)
{
	const char* seg = NULL;
	const char* cur = path;
	size_t offset = 0;
	while ((seg = strchr(cur, '/')) != NULL) {

		/* printf("seg: %s\tcur: %s\n", seg, cur); */

		/* The node we just skipped over. */
		char buffer[ZAR_MAX_PATH];
		memset(buffer, '\0', sizeof(buffer));
		size_t step = strlen(cur) - strlen(seg);

		cur += step + 1;
		offset += step + 1;

		strncpy(buffer, path, offset - 1);
		/* printf("buffer: %s\n", buffer); */
		if (wrapped_mkdir(buffer) != 0)
			return -1;
	}
	return wrapped_mkdir(path);
}


char* system_dirname(const char* path)
{
	static const char* def = ".";
	char* out = NULL;

	if (path == NULL || *path == '\0')
		goto FAIL;

	out = strrchr(path, '/');
	if (out == NULL)
		goto FAIL;

	size_t end = strlen(path) - strlen(out);
	out = malloc(end);
	if (out == NULL)
		return NULL;

	for (size_t i=0; i < end; ++i)
		out[i] = path[i];
	out[end] = '\0';

	return out;

FAIL:
	out = malloc(2);
	out[0] = '.';
	out[1] = '\0';
	return out;
}


const char* system_basename(const char* path)
{
	static const char* def = ".";
	char* out = NULL;

	if (path == NULL || *path == '\0')
		goto FAIL;

	const char* p = strrchr(path, '/');
	if (p == NULL)
		goto FAIL;
	++p;

	size_t length = strlen(p);
	out = malloc(length + 1);
	memcpy(out, p, length);
	out[length] = '\0';
	return out;
FAIL:
	out = malloc(2);
	out[0] = '.';
	out[1] = '\0';
	return out;
}


char* system_fix_pathseps(char* path)
{
	char* slash = NULL;
	do {
		/*
		 * Ensure paths will be UNIX style, not DOS style.
		 * VMS/RISOS users and cie are on their own.
		 */
		slash = strstr(path, "\\");
		if (slash != NULL)
			*slash = '/';
	} while(slash != NULL);
	return path;
}

