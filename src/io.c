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
static const int32_t zar_end_mark = 0x5A415200;


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


static inline void put_string(char* str, FILE* file)
{
	fwrite(str, 1, strlen(str), file);
	fputc('\0', file);
}


/** Return an fpos_t marking current position in archive.
 *
 * This allows one to later seek there using fsetpos(). If it fails, error() is
 * called to exit the program.
 */
static fpos_t mark_position(ZarHandle* archive)
{
	fpos_t m;
	if (fgetpos(archive->handle, &m) != 0)
		error(EX_IOERR, "%s: unable to get file positon: %s", archive->path, strerror(errno));
	return m;
}


/** Store raw file data in archive.
 *
 * Call this to copy the file into the archive without any mutations.
 * Updates the record's checksum field with the inputs CRC-32.
 * Returns the length of the data in bytes.
 */
static ZarOffset_t record_raw_file(ZarFileRecord* record, ZarHandle* archive)
{
	ZarOffset_t length = 0;

	xtrace("%s: recording raw file to archive %s.", record->path, archive->path);

	FILE* infile = fopen(record->path, "rb");
	if (infile == NULL) {
		warn("failed opening %s (%s)", record->path, strerror(errno));
		return -1;
	}

	record->checksum = crc32(0L, Z_NULL, 0);
	int c;
	while ((c = fgetc(infile)) != EOF) {
		length += 1;
		record->checksum = crc32(record->checksum, (Bytef*)&c, 1);
		if (fputc(c, archive->handle) == EOF)
			error(EX_IOERR, "failed writing byte %d from %s to archive %s",
			      length, record->path, archive->path);
	}
	debug("%s: file checksum: %lu", record->path, record->checksum);
	debug("%s: file length: %d", record->path, length);

	if (feof(infile))
		debug("end of %s", record->path);
	fclose(infile);

	return length;
}


static void foo(ZarHandle* archive, const char* file)
{
	FILE* infile;
	ZarFileRecord record;

	/* Where to move this? */
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

	// snip

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
		volume->records[i] = zar_create_file_record(files[i]);
		debug("adding %s to file map for archive %s",
		      volume->records[i]->path, zar->path);
	}
	/* archive->volumes[archive->nvolumes] = volume; */
	/* archive->nvolumes++; */

	zar_write_volume_record(volume, zar);

	for (size_t i=0; i < volume->nrecords; ++i) {
		const char* file = files[i];
		info("adding %s to archive %s", file, zar->path);
		/* foo(zar, file); */
		zar_write_file_record(volume->records[i], zar);
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
		ZarFileRecord* record = volume->records[i];
		debug("dumping file record %s", record->path);

		/* We don't need to do this as we already know the path.
		 * But hey, I wanna test this function >_<.
		 */
		zar_read_file_record(record, zar);
	}

	for (size_t i=0; i < volume->nrecords; ++i) {
		free(volume->records[i]);
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
	fpos_t mark = mark_position(archive);

	ZarOffset_t diff = 0;
	fwrite(&diff, 1, sizeof(diff), archive->handle);

	diff = ftell(archive->handle) - sizeof(diff);
	xtrace("diff marked at %d", diff);
	/* tpzar only supports UTF-8, and only in as much as the C library does if even that. */
	static const char* encoding = "utf-8";
	put_string(encoding, archive->handle);

	/* File map is a simple offset -> path. */
	for (size_t i=0; i < volume->nrecords; ++i) {
		debug("write 0'd offset %d bytes long", sizeof(ZarOffset_t));
		ZarOffset_t offset = 0;
		fwrite(&offset, 1, sizeof(ZarOffset_t), archive->handle);

		debug("write NUL terminated string '%s', %d bytes long",
		      volume->records[i]->path, strlen(volume->records[i]->path)+1);
		put_string(volume->records[i]->path, archive->handle);
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
	debug("zar_end_mark:0x%08x (%d) sizeof %ld", zar_end_mark, zar_end_mark, sizeof(int32_t));

	/* How do we know if we should write start or end mark? */
	fwrite(&zar_start_mark, 1, 4, archive->handle);

	zar_write_filemap(volume, archive);

	fwrite(&volume->checksum, 1, 4, archive->handle);
	fwrite(&volume->offset, 1, 8, archive->handle);

	debug("offset_t:%ld", sizeof(long));

	const char* myname = "TPZAR";
	put_string(myname, archive->handle);

	const char* myver = "0.1";
	put_string(myver, archive->handle);

	info("Wrote volume created by %s/%s", myname);
}

void zar_read_volume_record(ZarVolumeRecord* volume, ZarHandle* archive)
{
	int32_t start, end;

	debug("Reading volume record from %s", archive);

	debug("zar_start_mark:0x%08x (%d) sizeof %ld", zar_start_mark, zar_start_mark, sizeof(int32_t));
	debug("zar_end_mark:0x%08x (%d) sizeof %ld", zar_end_mark, zar_end_mark, sizeof(int32_t));

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
	volume->records = NULL;
	do {
		ZarOffset_t offset;
		char path[ZAR_MAX_PATH];
		fread(&offset, 1, sizeof(offset), archive->handle);
		get_string(path, sizeof(path), archive->handle);
		debug("%s: next offset in file map: %d", archive->path, offset);
		debug("%s: next path in file map: %s", archive->path, path);
		pos = ftell(archive->handle);

		/*
		 * There's no indication of how many files are in the map. Assume 1 and
		 * realloc() as necessary. That way we can stub a record.
		 */
		if (volume->records == NULL) {
			volume->records = malloc(sizeof(ZarFileRecord) * 1);
		} else {
			volume->records = realloc(volume->records, sizeof(ZarFileRecord) * (volume->nrecords + 1));
		}
		volume->records[volume->nrecords] = zar_create_file_record(path);
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


ZarFileRecord* zar_create_file_record(const char* path)
{
	ZarFileRecord* r = malloc(sizeof(ZarFileRecord));
	if (r == NULL)
		error(EX_OSERR, errno == ENOMEM ? "No memory." : "Memory allocator failed");
	strncpy(r->path, path, sizeof(r->path));
	debug("created file record for path %s", r->path);

	/* Make sure these fields are initialized rather than left in an undefined state. */
	r->offset = 0;
	r->checksum = 0;
	r->length = 0;
	r->format[0] = 0xDE;
	r->format[1] = 0xAD;

	return r;
}


/** Read record from current archive position.
 *
 * Current position into the archive must be aligned to the start of a record.
 */
void zar_read_file_record(ZarFileRecord* record, ZarHandle* archive)
{
	warn("pos at read offset: %ld", ftell(archive->handle));
	fread(&record->offset, 1, sizeof(ZarOffset_t), archive->handle);
	debug("offset to end of record: %ld bytes", record->offset);

	warn("pos at read format: %ld", ftell(archive->handle));
	record->format[0] = (char)fgetc(archive->handle);
	record->format[1] = (char)fgetc(archive->handle);
	debug("file data format is %c%c", record->format[0], record->format[0]);

	warn("pos at read path: %ld", ftell(archive->handle));
	/* We're limiting paths to ZAR_MAX_PATH but the format uses NUL termination. */
	get_string(record->path, sizeof(record->path), archive->handle);
	debug("read file record path: %s", record->path);

	warn("pos at read length: %ld", ftell(archive->handle));
	fread(&record->length, 1, sizeof(ZarOffset_t), archive->handle);
	debug("file data is %d bytes long", record->length);
	fseek(archive->handle, record->length, SEEK_CUR);

	fread(&record->checksum, 1, sizeof(CRC32_t), archive->handle);
	debug("file record checksum: %lu", record->checksum); /* TODO: to string! */
}


void zar_write_file_record(ZarFileRecord* record, ZarHandle* archive)
{
	puts("------------------------------------------------------------------------------------------");
	fpos_t offset_mark = mark_position(archive);

	xtrace("start of record at %ld bytes", ftell(archive->handle));
	fwrite("OOOOOOOO", 1, sizeof(ZarOffset_t), archive->handle);
	xtrace("after offset at start of record at %ld bytes", ftell(archive->handle));
	record->offset = ftell(archive->handle);

	/* TODO: How do we want to decide format?
	 *
	 * Best plan is probably to make a guess if the file matches a known
	 * file extension or magic number for a type we know won't compress
	 * well. And then either store it or apply a minimalist RLE.
	 *
	 * If it's just random data other than that: deflate the sucker.
	 *
	 * Reserve future XZ support for certain data sets or a "Try harder" option.
	 */
	/* For now we just store the data. */
	record->format[1] = record->format[0] = 0x00;

	warn("pos at write format: %ld", ftell(archive->handle));
	fputc(record->format[0], archive->handle);
	fputc(record->format[1], archive->handle);

	warn("pos at write path: %ld", ftell(archive->handle));
	put_string(record->path, archive->handle);

	fpos_t length_mark = mark_position(archive);
	warn("pos at write length: %ld", ftell(archive->handle));
	fwrite("LLLLLLLL", 1, sizeof(ZarOffset_t), archive->handle);

	record->length = record_raw_file(record, archive);

	fwrite(&record->checksum, 1, sizeof(CRC32_t), archive->handle);

	xtrace("end of record at %ld bytes", ftell(archive->handle));

	/* Leap back to update the offset to end of record. */
	record->offset = ftell(archive->handle) - record->offset;
	fpos_t end_mark = mark_position(archive);
	if (fsetpos(archive->handle, &offset_mark) != 0)
		error(EX_IOERR, "%s: failed seeking back to file record offset", archive->path);
	debug("offset to next record: %ld", record->offset);
	fwrite(&record->offset, 1, sizeof(ZarOffset_t), archive->handle);

	/* And forward to update the length of the compressed data. */
	if (fsetpos(archive->handle, &length_mark))
		error(EX_IOERR, "%s: failed seeking back to file data length", archive->path);
	debug("length of recorded file: %ld", record->length);
	fwrite(&record->length, 1, sizeof(ZarOffset_t), archive->handle);


	if (fsetpos(archive->handle, &end_mark) != 0)
		error(EX_IOERR, "%s: failed seeking back to end of record", archive->path);
}
