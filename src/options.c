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
	puts("usage: zar {zarfile} [input files]");
	putchar('\n');
	puts("\t-h,     \tshort help.");
	puts("\t--help, \tlong help.");
	exit(64);
}


bool inline is_option(const char* option, const char* argument)
{
	/* Yes. I. Am. Lazy. */
	return strcmp(option, argument) == 0;
}


struct ZarOptions parse_options(int argc, char* argv[])
{
	int i;
	bool done = false;
	struct ZarOptions opts;

	opts.zarfile = "-";
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
		else {
			printf("unrecongized option: %s\n", arg);
			usage_short();
		}
	}
	argv += i;
	argc -= i;
	for (i=0; i < argc; ++i)
		debug("remaining args:%s", argv[i]);

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
	for (i=0; i < opts.ninputs; ++i) {
		const char* path = argv[i];
		debug("Adding %s to inputs[%d]", path, i);
		opts.inputs[i] = argv[i];
	}

	debug("ZarOptions::zarfile:%s", opts.zarfile);
	debug("ZarOptions::mode: %c", opts.mode);
	for (i=0; i < opts.ninputs; ++i)
		debug("ZarOptions::inputs[%d]:%s", i, opts.inputs[i]);

	return opts;
}
