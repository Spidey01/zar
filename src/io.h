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
#ifndef ZAR_SRC_IO__H
#define ZAR_SRC_IO__H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define ZAR_MAX_PATH 1024
/* Place holder because we don't do CRC-32 yet. */
typedef uint32_t CRC32_t;
typedef int64_t ZarOffset_t;

struct ZarVolumeRecord_t;

typedef struct {
	char path[ZAR_MAX_PATH];
	FILE* handle;
	size_t nvolumes;
	struct ZarVolumeRecord_t* volumes;
} ZarHandle;

/** Records a file within a ZAR volume. */
typedef struct ZarFileRecord_t {
	/** Path relative to the root of the archive. */
	char path[ZAR_MAX_PATH];

	/** CRC32 Checksum of original file being recorded. */
	CRC32_t checksum;

	/** Character representation of how this file is stored.
	 *
	 * Standard format codes:
	 *
	 *     - "gz" => GZip compressed.
	 *     - "bz" => BZip2 compressed.
	 *     - "xz" => XZ compressed.
	 *     - "  " => No compression.
	 */
	char format[2];


	/* TODO: the actual file data. */

	/* TODO: offset to next record. */

	/* TODO: file permissions */

	/* TODO: whatever. */
} ZarFileRecord;

typedef struct ZarVolumeRecord_T {
	/* TODO: format version. */
	/* TODO: tool name. */
	/* TODO: tool version. */
	size_t nrecords;
	ZarFileRecord** records;
	/* Checksum of all records data. */
	CRC32_t checksum;
	/*
	 * Number of bytes between end of volume header and start of volume footer.
	 * I.e. the byte range containing file records.
	 */
	ZarOffset_t offset;
} ZarVolumeRecord;

/* Probably want to return ZarHandle*? */
void zar_create(const char* archive, char* files[], size_t count);

void zar_list(const char* archive);

ZarHandle* zar_open(const char* archive);
void zar_close(ZarHandle* archive);

ZarVolumeRecord* zar_create_volume_header();
void zar_read_volume_record(ZarVolumeRecord* volume, ZarHandle* archive);
void zar_write_volume_record(ZarVolumeRecord* volume, ZarHandle* archive);

#endif

