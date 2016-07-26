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

#ifndef ZAR_SRC_OPTIONS__H
#define ZAR_SRC_OPTIONS__H

#include <stddef.h>
#include <stdbool.h>

struct ZarOptions {
	const char* zarfile;
	const char* dir;
	size_t ninputs;
	char** inputs;
	/*
	 * c == create new archive from inputs.
	 * x == extract specified archive.
	 * t == list contents of archive.
	 * i == info about archive.
	 */
	char mode;
	bool verbose;
};

struct ZarOptions parse_options(int argc, char* argv[]);

#endif
