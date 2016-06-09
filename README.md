Tool for Zippy Archive Revolutionary files.
===========================

I created the ZAR file format for fun. Don't take it too seriously.

The name is based on a few scripts I've toyed with over the years for making a tar like interface to InfoZip + unix tar. It's expansion is just me having a brain fart^H^H^H^H^H^H^H^H^H^H, stubbing this README file.

Feel free to call the program tpzar. I probably will.

Tool Usage
----------

usage: zar [options] {zarfile} [input files]

        -h,                             short help.
        --help,                         long help.
        -c, --create,                   create an archive.
        -t, --list,                     list archive members.
        -x, --extract,                  list archive members.
        -f FILE, --file FILE,           specify ZAR archive file.
        -v, --verbose,                  chitty, chatty two shoes.

ZAR File Format
---------------

The format consists of a series of volume records and file records.

A typical ZAR archive looks like this:

### Volume Records ###

Every volume is preceeded by a header called a volume record. At the end of a volume, a ...

Fields are stored in little endian unless otherwise stated.


    Offset  Bytes    Value          Comment
    0       4        0x0052415A     Magic number. ZAR0.
    TODO: format version
    TODO: tool name
    TODO: tool version
    TODO: file map
    ????    4        CRC-32         Checksum over all file records.
    ????    8        int64_t        Offset to next volume record.
    ????    4        0x5A415200     Inversed magic number. 0RAZ.

