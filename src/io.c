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

#include "sysexits.h"

#include "zlib.h"

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


/* ZAR0 stored in little-endian. This shows as "ZAR^@" if opened in vim. */
static const int32_t zar_start_mark = 0x0052415A;
/* The inverse but still in little-endian. */
static const int32_t end_mark = 0x5A415200;


/** Like fgets() but looks for NUL terminator instead of newline. */
static inline void get_string(char* dest, size_t length, FILE *file)
{
	size_t i = 0;
	memset(dest, 0, length);
	while (i < length) {
		/* What about EOF? */
		dest[i] = (char)fgetc(file);
		if (dest[i] == '\0')
			break;
		i++;
	}
}


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
		error(EX_SOFTWARE, "%s: handle not open!", __FILE__);

	infile = fopen(file, "rb");
	if (infile == NULL) {
		warn("failed opening %s (%s)", file, strerror(errno));
		return;
	}
#if 0
	if (infile == NULL)
		error(EX_IOERR, "failed opening %s (%s)", file, strerror(errno));
#endif

/* Simple tests that work like cat inputs > archive. */
#if 1
	debug("buffering %s", record.path);
	errno = 0;
	record.checksum = crc32(0L, Z_NULL, 0);
	while ((c = fgetc(infile)) != EOF) {
		debug("read: %c", c);
		record.checksum = crc32(record.checksum, &c, 1);
		if (fputc(c, archive->handle) == EOF)
			error(EX_IOERR, "failed writing byte %c from %s to archive %s",
			      c, record.path, archive->path);
		debug("wrote: %c", c);
	}
	debug("%s CRC-32: %lu", record.path, record.checksum);
	fwrite(&record.checksum, 1, sizeof(CRC32_t), archive->handle);

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
	info("archive name:%s", archive);
	debug("archive members:%d", count);

	ZarHandle* zar = zar_open(archive);
	if (zar == NULL)
		return;

	ZarVolumeRecord* volume = zar_create_volume_header();
	volume->nrecords = count;
	volume->records = malloc(volume->nrecords * sizeof(ZarFileRecord));
	debug("volume->records: %p", volume->records);
	volume->checksum = 0;
	volume->offset = 0;
	for (size_t i=0; i < volume->nrecords; ++i) {
		ZarFileRecord* r = volume->records[i] = malloc(sizeof(ZarFileRecord));
		strncpy(r->path, files[i], sizeof(r->path));
		debug("adding %s to file map for archive %s", r->path, zar->path);
	}
	/* archive->volumes[archive->nvolumes] = volume; */
	/* archive->nvolumes++; */

	zar_write_volume_record(volume, zar);

	for (size_t i=0; i < volume->nrecords; ++i) {
		const char* file = files[i];
		info("adding %s to archive %s", file, zar->path);
		foo(zar, file);
	}

	zar_close(zar);
}


void zar_list(const char* archive)
{
	ZarHandle* zar;
	ZarVolumeRecord* volume;

	zar = zar_open(archive);
	if (zar == NULL)
		return;

	/* Uhh, handle multi-volume archives? */
	volume = zar_create_volume_header();
	zar_read_volume_record(volume, zar);
	for (size_t i=0; i < volume->nrecords; ++i) {
		debug("dumping file record %d", i);
		/* We don't have any records yet after a read. */
		/* info("%s", volume->records[i]->path); */
	}
	free(volume);
	zar_close(zar);
}


ZarHandle* zar_open(const char* archive)
{
	debug("%s:%s():%s", __FILE__, __FUNCTION__, archive);

	ZarHandle* r = malloc(sizeof(ZarHandle));
	if (r == NULL) {
		error(EX_OSERR, errno == ENOMEM ? "No memory." : "Memory allocator failed");
	}
	memset(r->path, 0, sizeof(r->path));
	r->handle = NULL;
	r->nvolumes = 0;
	r->volumes = NULL;

	strncpy(r->path, archive, sizeof(r->path));
	debug("path:%s", r->path);
	r->handle = fopen(r->path, "r+b");
	if (errno == ENOENT && r->handle == NULL) {
		debug("Archive doesn't exist: creating it.");
		r->handle = fopen(r->path, "w+b");
	}

	if (r->handle == NULL) {
		free(r);
		error(EX_IOERR, "Failed opening archive %s (%s)", archive, strerror(errno));
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

/** Writes the volume record's file map to archive.
 *
 * A file map consists of the following data:
 *
 *   - Length of this block, the file map.
 *   - Encoding of paths utf-what.
 *   - Offset to first file record.
 *   - Path of first file record.
 *   - ... more records ...
 *
 * Basically an offset to slurp or skip it, the encoding, and a sequence of offsets and strings.
 */
void zar_write_filemap(ZarVolumeRecord* volume, ZarHandle* archive)
{
	xtrace("pos at %s start: %d", __FUNCTION__, ftell(archive->handle));

	/* We don't know the length yet, so B/P to rewind to here and write it.
	 * And then reuse it to fast forward back again.
	 * Be mindful of the math when seeking!
	 */
	fpos_t mark;
	if (fgetpos(archive->handle, &mark) != 0)
		error(EX_IOERR, "%s: unable to get file positon: %s", archive->path, strerror(errno));

	ZarOffset_t diff = 0;
	fwrite(&diff, 1, sizeof(diff), archive->handle);

	diff = ftell(archive->handle) - sizeof(diff);
	xtrace("diff marked at %d", diff);
	/* tpzar only supports UTF-8, and only in as much as the C library does if even that. */
	static const char* encoding = "utf-8";
	fwrite(encoding, 1, strlen(encoding), archive->handle);
	fputc(0, archive->handle);

	/* File map is a simple offset -> path. */
	for (size_t i=0; i < volume->nrecords; ++i) {
		debug("write 0'd offset %d bytes long", sizeof(ZarOffset_t));
		ZarOffset_t offset = 0;
		fwrite(&offset, 1, sizeof(ZarOffset_t), archive->handle);

		debug("write NUL terminated string '%s', %d bytes long",
		      volume->records[i]->path, strlen(volume->records[i]->path)+1);
		fwrite(volume->records[i]->path, 1, strlen(volume->records[i]->path), archive->handle);
		fputc(0, archive->handle);
	}

	xtrace("pos after map written: %ld", ftell(archive->handle));
	/* Alright: rewind and write the updated file map length. */
	diff = ftell(archive->handle) - diff - sizeof(diff);
	xtrace("diff: %d", diff);
	xtrace("rewinding from current pos %ld", ftell(archive->handle));
	if (fsetpos(archive->handle, &mark) != 0)
		error(EX_IOERR, "%s: failed seeking back to file map offset", archive->path);
	xtrace("rewound to current pos %ld", ftell(archive->handle));
	/* Great for testing. */
	#if 0
	const char* xxxxxxxx = "XXXXXXXX"; 
	fwrite(xxxxxxxx , 1, sizeof(diff), archive->handle);
	#else
	fwrite(&diff, 1, sizeof(diff), archive->handle);
	#endif
	xtrace("wrote updated file map length of %d", diff);
	if (fseek(archive->handle, (long)diff, SEEK_CUR) != 0)
		error(EX_IOERR, "%s: failed seeking to end of file map", archive->path);
	xtrace("fast forwarded current pos %ld", ftell(archive->handle));
}

void zar_write_volume_record(ZarVolumeRecord* volume, ZarHandle* archive)
{
	debug("zar_start_mark:0x%08x (%d) sizeof %ld", zar_start_mark, zar_start_mark, sizeof(int32_t));
	debug("end_mark:0x%08x (%d) sizeof %ld", end_mark, end_mark, sizeof(int32_t));

	/* How do we know if we should write start or end mark? */
	fwrite(&zar_start_mark, 1, 4, archive->handle);

	zar_write_filemap(volume, archive);

	fwrite(&volume->checksum, 1, 4, archive->handle);
	fwrite(&volume->offset, 1, 8, archive->handle);

	debug("offset_t:%ld", sizeof(long));

	const char* myname = "TPZAR";
	fwrite(myname, 1, strlen(myname), archive->handle);
	fputc('\0', archive->handle);
	fwrite(myname, 1, strlen(myname), stderr); fputc('\n', stderr);

	const char* myver = "0.1";
	fwrite(myver, 1, strlen(myver), archive->handle);
	fputc('\0', archive->handle);
	fwrite(myver, 1, strlen(myver), stderr); fputc('\n', stderr);
}

void zar_read_volume_record(ZarVolumeRecord* volume, ZarHandle* archive)
{
	int32_t start, end;

	debug("Reading volume record from %s", archive);

	debug("zar_start_mark:0x%08x (%d) sizeof %ld", zar_start_mark, zar_start_mark, sizeof(int32_t));
	debug("end_mark:0x%08x (%d) sizeof %ld", end_mark, end_mark, sizeof(int32_t));

	fread(&start, 1, 4, archive->handle);
	if (start != zar_start_mark)
		error(EX_DATAERR, "%s: bad volume header.", archive->path);

	/* TODO: decode file map */
	ZarOffset_t maplength;
	fread(&maplength, 1, sizeof(ZarOffset_t), archive->handle);
	debug("%s: file map is %zd bytes long", archive->path, maplength);
	char encoding[8];
	get_string(encoding, sizeof(encoding), archive->handle);
	debug("%s: file map is & paths are encoded as %s", archive->path, encoding);

	long pos = ftell(archive->handle);
	xtrace("Started reading file map entries at %d", pos);
	do {
		ZarOffset_t offset;
		char path[ZAR_MAX_PATH];
		fread(&offset, 1, sizeof(offset), archive->handle);
		get_string(path, sizeof(path), archive->handle);
		debug("%s: next offset in file map: %d", archive->path, offset);
		debug("%s: next path in file map: %s", archive->path, path);
		pos = ftell(archive->handle);

		volume->nrecords += 1;
	} while(pos < maplength);
	xtrace("Finished reading file map entries at %d", ftell(archive->handle));

	fread(&volume->checksum, 1, 4, archive->handle);
	debug("%s: volume checksum: %ld", archive->path, volume->checksum); /* TODO: to string! */
	fread(&volume->offset, 1, 8, archive->handle);
	debug("%s: offset to backup volume record %ld", archive->path, volume->offset);

	/* 
	 * Parse the name and version of what created this volume.
	 */
	char app[16], ver[16];
	get_string(app, sizeof(app), archive->handle);
	get_string(ver, sizeof(ver), archive->handle);
	info("volume created by %s/%s", app, ver);

	/* TODO: we might want to verify footer. */

}

