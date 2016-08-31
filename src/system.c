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

#include "debug.h"
#include "io.h"
#include "sysexits.h"
#include "system.h"

#ifdef _WIN32
#include <direct.h>
#define chdir(s) _chdir(s)
#define getcwd(buffer, length) _getcwd(buffer, length)
#include <sys/types.h>
#include <sys/stat.h>
#include <windows.h>
#include <winbase.h>
#include <FileAPI.h>
#define stat(path, buffer) _stat(path, buffer)
#define S_ISDIR(mode) (mode & _S_IFDIR)
#else
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
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


bool system_isdir(const char* path)
{
	struct stat s;
	if (stat(path, &s) != 0)
		error(EX_IOERR, "%s: stat() failed: %s", path, strerror(errno));
	return S_ISDIR(s.st_mode);
}


#if _WIN32
/* Used to emulate the POSIX interface on top of the local hacks. */
struct DirHandleWrapper {
	char* kludge;
	WIN32_FIND_DATA data;
	HANDLE handle;
};
#endif


void* system_opendir(const char* path)
{
#if _WIN32
	struct DirHandleWrapper* wrapper = malloc(sizeof(struct DirHandleWrapper));
	if (wrapper == NULL)
		return NULL;

	/* Windows is weird: we need to append \* or we don't get the contents. */
	size_t len = strlen(path);
	wrapper->kludge = malloc(len + 3);
	if (wrapper->kludge == NULL)
		error(EX_OSERR, "%s(): unable allocate memory.", __func__);
	memcpy(wrapper->kludge, path, len);
	wrapper->kludge[len] = '\\';
	wrapper->kludge[len+1] = '*';
	wrapper->kludge[len+2] = '\0';

	wrapper->handle = FindFirstFile(wrapper->kludge, &wrapper->data);
	if (wrapper->handle == INVALID_HANDLE_VALUE) {
		free(wrapper->kludge);
		return NULL;
	}
	return wrapper;
#else
	return opendir(path);
#endif
}


char* system_readdir(void* dirhandle, char* result, size_t max)
{
#if _WIN32
	struct DirHandleWrapper* wrapper = (struct DirHandleWrapper*)dirhandle;
SKIP_DOTS:
	/* XXX: for some reason we get a crash instead of a 0 at end of dir. */
	if (FindNextFile(wrapper->handle, &wrapper->data) == 0)
		return NULL;
	if (strcmp("..", wrapper->data.cFileName) == 0)
		goto SKIP_DOTS;
	return strncpy(result, wrapper->data.cFileName, max);
#else
	struct dirent* entry = readdir((DIR*)dirhandle);
	if (entry == NULL) {
		return NULL;
	}
	/* I forget if unix readdir() returns . or .. */
	return strncpy(result, entry->d_name, max);
#endif
}


void system_closedir(void* dirhandle)
{
#if _WIN32
	struct DirHandleWrapper* wrapper = (struct DirHandleWrapper*)dirhandle;
	FindClose(wrapper->handle);
	free(wrapper->kludge);
	free(wrapper);
#else
	closedir((DIR*)dirhandle);
#endif
}

