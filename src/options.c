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
#include "options.h"
#include "sysexits.h"
#include "system.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* TODO: move to shared header... */
#define ZAR_MAX_PATH 1024

extern int debug_level;

void usage_short()
{
	puts("usage: zar [options] {zarfile} [input files]");
	exit(64);
}


void usage_long()
{
	puts("usage: zar [options] {zarfile} [input files]");
	putchar('\n');
	puts("\t-h,                      \tshort help.");
	puts("\t--help,                  \tlong help.");
	puts("\t-c, --create,            \tcreate an archive.");
	puts("\t-t, --list,              \tlist archive members.");
	puts("\t-i, --info,              \tinfo about archive.");
	puts("\t-x, --extract,           \tlist archive members.");
	puts("\t-C DIR, --directory DIR  \twhere to extract archive.");
	puts("\t-f FILE, --file FILE,    \tspecify ZAR archive file.");
	puts("\t-v, --verbose,           \tchitty, chatty two shoes.");
	exit(64);
}


static inline bool is_option(const char* option, const char* argument)
{
	/* Yes. I. Am. Lazy. */
	return strcmp(option, argument) == 0;
}


static inline void append_to_inputs(struct ZarOptions* opts, const char* path, size_t* index)
{
	info("Adding %s to input list at index %d", path, *index);

	opts->ninputs += 1;

	opts->inputs = realloc(opts->inputs, ZAR_MAX_PATH * opts->ninputs);
	if (opts->inputs == NULL)
		error(EX_OSERR, "Unable to allocate memory");

	char* p = opts->inputs[*index] = malloc(ZAR_MAX_PATH);
	if (p == NULL)
		error(EX_OSERR, "Unable to allocate memory for %s", path);
	memset(p, '\0', ZAR_MAX_PATH);
	strncpy(p, path, strlen(path) + 1);

	*index += 1;
}


static inline void foo(struct ZarOptions* opts, const char* root, size_t* index)
{
	info("Adding %s to input list (recursively)", root);
	if (*index > 30)
		error(1, "FORCE EXIT");

	void* dir = system_opendir(root);
	char* entry;
	char buffer[ZAR_MAX_PATH];

	memset(buffer, '\0', ZAR_MAX_PATH);
	while ((entry = system_readdir(dir, buffer, sizeof(buffer))) != NULL) {
		debug("entry/buffer: %s", buffer);

		char path[ZAR_MAX_PATH];

		xtrace("Combing root and system_readdir() entry into one path.");
		size_t sep = strlen(root);
		strncpy(path, root, ZAR_MAX_PATH);
		path[sep] = '/';
		char* p = path + sep + 1;
		strncpy(p, entry, ZAR_MAX_PATH - strlen(path));
		xtrace("Combined path: %s", path);

		#if 0
		append_to_inputs(opts, path, index);
		#else
		void* child = system_opendir(path);
		if (child == NULL) {
			append_to_inputs(opts, path, index);
		} else {
			system_closedir(child);
			foo(opts, path, index);
		}
		#endif
	}

	system_closedir(dir);
}


struct ZarOptions parse_options(int argc, char* argv[])
{
	int i;
	bool done = false;
	struct ZarOptions opts;

	opts.zarfile = "-";
	opts.dir = system_getcwd(NULL, 0);
	opts.inputs = NULL;
	opts.mode = '\0';

	for (i=0; i < argc; ++i) {
		const char* arg = argv[i];

		debug("arg:%s", arg);

		if (arg[0] != '-' || is_option("--", arg)) {
			/* Treat '--' as end of argument processing. */
			done = true;
			break;
		}
		else if (is_option("-h", arg)) {
			usage_short();
		}
		else if (is_option("--help", arg)) {
			usage_long();
		}
		else if (is_option("-v", arg) || is_option("--verbose", arg)) {
			opts.verbose = true;
			debug_level += 1;
		}
		else if (is_option("-D", arg) || is_option("--debug-level", arg)) {
			i++;
			debug_level = atoi(argv[i]);
		}
		else if (is_option("-f", arg) || is_option("--file", arg)) {
			i++;
			opts.zarfile = argv[i];
		}
		else if (is_option("-c", arg) || is_option("--create", arg)) {
			opts.mode = 'c';
		}
		else if (is_option("-x", arg) || is_option("--extract", arg)) {
			opts.mode = 'x';
		}
		else if (is_option("-t", arg) || is_option("--list", arg)) {
			opts.mode = 't';
		}
		else if (is_option("-i", arg) || is_option("--info", arg)) {
			opts.mode = 'i';
		}
		else if (is_option("-C", arg) || is_option("--directory", arg)) {
			i++;
			opts.dir = argv[i];
		}
		else {
			printf("unrecognized option: %s\n", arg);
			usage_short();
		}
	}
	argv += i;
	argc -= i;
	for (i=0; i < argc; ++i)
		debug("remaining args:%s", argv[i]);
	i = -1;

	debug("number of inputs on command line:%d", argc);

	/* Just a stub. */
	opts.inputs = malloc(1);
	if (opts.inputs == NULL)
		error(EX_OSERR, "Unable to allocate any memory");

	/*
	 * Add the files first, 'cuz easy peasy.
	 */
	opts.ninputs = 0;
	size_t index = 0;
	for (int j=0; j < argc; ++j) {
		const char* path = system_fix_pathseps(argv[j]);
		if (system_isdir(path))
			continue;
		append_to_inputs(&opts, path, &index);
	}
	debug("index now: %d", index);

	/*
	 * Second pass for the directories.
	 */
	for (int j=0; j < argc; ++j) {
		const char* path = system_fix_pathseps(argv[j]);
		if (!system_isdir(path))
			continue;
		foo(&opts, path, &index);
	}
	debug("index now: %d", index);

	debug("ZarOptions::zarfile:%s", opts.zarfile);
	debug("ZarOptions::mode: %c", opts.mode);
	debug("ZarOptions::ninputs: %d", opts.ninputs);
	for (size_t j=0; j < opts.ninputs; ++j)
		debug("ZarOptions::inputs[%d]:%s", j, opts.inputs[j]);

	return opts;
}
