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

#include "io.h"

#include "debug.h"

#include "errno.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


/* ZAR0 stored in little-endian. This shows as "ZAR^@" if opened in vim. */
static const int32_t zar_start_mark = 0x0052415A;
/* The inverse but still in little-endian. */
static const int32_t end_mark = 0x5A415200;


static void foo(ZarHandle* archive, const char* file)
{
	int c;
	FILE* infile;
	ZarFileRecord record;

	if (*file == '/' || *file == '\\') {
		debug("stripping leading / from %s", file);
		++file;
	}
	strncpy(record.path, file, sizeof(record.path));

	if (archive->handle == NULL)
		error(70 /* EX_SOFTWARE */, "%s: handle not open!", __FILE__);

	infile = fopen(file, "r");
	if (infile == NULL) {
		warn("failed opening %s (%s)", file, strerror(errno));
		return;
	}
#if 0
	if (infile == NULL)
		error(74 /* EX_IOERR */, "failed opening %s (%s)", file, strerror(errno));
#endif

/* Simple tests that work like cat inputs > archive. */
#if 1
	debug("buffering %s", record.path);
	errno = 0;
	while ((c = fgetc(infile)) != EOF) {
		// Always returns EOF??
		debug("read: %c", c);
		if (fputc(c, archive->handle) == EOF)
			error(74 /* EX_IOERR */, "failed writing byte %c from %s to archive %s",
			      c, record.path, archive->path);
		debug("wrote: %c", c);
	}

#else /////////////////////////
	char buffer[128];
	memset(buffer, 0, sizeof(buffer));
	debug("blah:%s", fgets(buffer, sizeof(buffer), archive->handle));
	while (fgets(buffer, sizeof(buffer), infile) != NULL) {
		debug("fgets over %s read \"%s\"", record.path, buffer);
		int nobjs = fwrite(buffer, 1, sizeof(buffer), archive->handle);
		if (nobjs != sizeof(buffer))
			warn("only wrote %d bytes of %s to archive %s", nobjs, record.path, archive->path);
	}
#endif
	if (feof(infile))
		debug("end of %s", record.path);

	fclose(infile);
}

void zar_create(const char* archive, char* files[], size_t count)
{
	int i;

	info("archive name:%s", archive);
	debug("archive members:%d", count);

	ZarHandle* zar = zar_open(archive);
	if (zar == NULL)
		return;

	ZarVolumeRecord* volume = zar_create_volume_header();
	volume->nrecords = count;
	volume->checksum = 0;
	volume->offset = 0;
	/* archive->volumes[archive->nvolumes] = volume; */
	/* archive->nvolumes++; */

	zar_write_volume_record(volume, zar);
	exit(0);

	for (i=0; i < volume->nrecords; ++i) {
		const char* file = files[i];
		info("adding %s to archive %s", file, zar->path);
		foo(zar, file);
	}

	zar_close(zar);
}


void zar_list(const char* archive)
{
	int i;
	ZarHandle* zar;
	ZarVolumeRecord* volume;

	zar = zar_open(archive);
	if (zar == NULL)
		return;

	/* Uhh, handle multi-volume archives? */
	volume = zar_create_volume_header();
	zar_read_volume_record(volume, zar);
	for (i=0; i < volume->nrecords; ++i) {
		debug("dumping file record %d", i);
	}
	free(volume);
	zar_close(zar);
}


ZarHandle* zar_open(const char* archive)
{
	debug("%s:%s():%s", __FILE__, __FUNCTION__, archive);

	ZarHandle* r = malloc(sizeof(ZarHandle));
	if (r == NULL) {
		int e = 71 /* EX_OSERR */;
		error(e, errno == ENOMEM ? "No memory." : "Memory allocator failed");
	}
	memset(r->path, 0, sizeof(r->path));
	r->handle = NULL;
	r->nvolumes = 0;
	r->volumes = NULL;

	strncpy(r->path, archive, sizeof(r->path));
	debug("path:%s", r->path);
	r->handle = fopen(r->path, "r+");
	if (errno == ENOENT && r->handle == NULL) {
		debug("Archive doesn't exist: creating it.");
		r->handle = fopen(r->path, "w+");
	}

	if (r->handle == NULL) {
		free(r);
		error(74 /* EX_IOERR */, "Failed opening archive %s (%s)", archive, strerror(errno));
	}

	return r;
}


void zar_close(ZarHandle* archive)
{
	debug("Closing archive %s", archive->path);
	fclose(archive->handle);
	memset(archive->path, 0, sizeof(archive->path));
	free(archive);
}


ZarVolumeRecord* zar_create_volume_header()
{
	ZarVolumeRecord* header = malloc(sizeof(ZarVolumeRecord));
	header->nrecords = 0;
	header->records = NULL;
	header->checksum = 0;
	header->offset = 0;
	return header;
}


void zar_write_volume_record(ZarVolumeRecord* volume, ZarHandle* archive)
{
	debug("zar_start_mark:0x%08x (%d) sizeof %ld", zar_start_mark, zar_start_mark, sizeof(int32_t));
	debug("end_mark:0x%08x (%d) sizeof %ld", end_mark, end_mark, sizeof(int32_t));

	fwrite(&zar_start_mark, 1, 4, archive->handle);

	/* TODO: encode file map */

	fwrite(&volume->checksum, 1, 4, archive->handle);
	fwrite(&volume->offset, 1, 8, archive->handle);

	fwrite(&end_mark, 1, 4, archive->handle);
	debug("offset_t:%ld", sizeof(long));
}
#if 0
ZAR00000
// VOLUME HEADER
File map 
    Name + offset
    .......
Checksum of all file records in volume // e.g. read them all into a block and check sum them to verify archive volume integrity
Offset to footer
Archiver name // TPZAR
Archiver version // 1.0
00000000
#endif


void zar_read_volume_record(ZarVolumeRecord* volume, ZarHandle* archive)
{
	int32_t start, end;

	debug("Reading volume record from %s", archive);

	debug("zar_start_mark:0x%08x (%d) sizeof %ld", zar_start_mark, zar_start_mark, sizeof(int32_t));
	debug("end_mark:0x%08x (%d) sizeof %ld", end_mark, end_mark, sizeof(int32_t));

	fread(&start, 1, 4, archive->handle);
	if (start != zar_start_mark)
		error(65 /* EX_DATAERR */, "%s: bad volume header.", archive->path);

	/* TODO: decode file map */

	fread(&volume->checksum, 1, 4, archive->handle);
	debug("%s: volume checksum: %ld", archive->path, volume->checksum); /* TODO: to string! */

	fread(&volume->offset, 1, 8, archive->handle);
	debug("%s: offset to backup volume record %ld", archive->path, volume->offset);

	/* TODO: we might want to verify footer. */

}

