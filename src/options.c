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
#include "system.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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


/* Cheap hack for right now. */
#ifdef _WIN32
#include <direct.h>
#define getcwd(buffer, length) _getcwd(buffer, length)
#else
#include <unistd.h>
#endif
struct ZarOptions parse_options(int argc, char* argv[])
{
	int i;
	bool done = false;
	struct ZarOptions opts;

	opts.zarfile = "-";
	opts.dir = getcwd(NULL, 0);
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

	opts.ninputs = argc;
	debug("number of inputs:%d", opts.ninputs);
	/*
	 * Populate the input list.
	 *
	 * We should be allocating and copying memory here more properly.
	 * Technically we don't need to give a crap: input is always from
	 * suitable static data (main's argv). So we don't.
	 */
	opts.inputs = malloc(opts.ninputs);
	if (opts.inputs == NULL) {
		error(71 /* EX_OSERR */, "Unable to allocate memory for %d input files.", argc);
	}
	for (size_t j=0; j < opts.ninputs; ++j) {
		const char* path = system_fix_pathseps(argv[j]);
		debug("Adding %s to inputs[%d]", path, j);
		opts.inputs[j] = argv[j];
	}

	debug("ZarOptions::zarfile:%s", opts.zarfile);
	debug("ZarOptions::mode: %c", opts.mode);
	for (size_t j=0; j < opts.ninputs; ++j)
		debug("ZarOptions::inputs[%d]:%s", j, opts.inputs[j]);

	return opts;
}
