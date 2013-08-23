#include <zlib.h>
#include "ztxt_stream.h"

namespace pdb
{

// Allow largest WBIT size for data
#define MAXWBITS 15

ZtxtStream::ZtxtStream(const std::string& file_path)
: PdbStream(file_path)
{
}

ZtxtStream::~ZtxtStream()
{
}

bool ZtxtStream::FillBuffer()
{
    stream.seekg(records[0].offset, std::ios_base::beg);
    stream.read(reinterpret_cast<char *>(&rec0), sizeof(ZTxtRecord0));
    if (stream.eof())
    {
        return false;
    }

    // To host byte order.
    ToHostByteOrder(&rec0);

    size = rec0.size;
    buf = new char[size];

    int in_size = raw_size - records[1].offset;
    char* in_buf = new char[in_size];
    stream.read(in_buf, in_size);

    // Start to decompress.
    if (!ZDecompress(reinterpret_cast<unsigned char *>(in_buf),
                     in_size,
                     reinterpret_cast<unsigned char *>(buf),
                     static_cast<uInt>(size)))
    {
        delete[] in_buf;
        delete[] buf;
        buf = 0;
        return false;
    }

    delete[] in_buf;
    return true;
}

void ZtxtStream::ToHostByteOrder(ZTxtRecord0* rec0)
{
    PdbStream::ToHostByteOrder(&rec0->version);
    PdbStream::ToHostByteOrder(&rec0->num_records);
    PdbStream::ToHostByteOrder(&rec0->size);
    PdbStream::ToHostByteOrder(&rec0->record_size);
    PdbStream::ToHostByteOrder(&rec0->num_bookmarks);
    PdbStream::ToHostByteOrder(&rec0->bm_record);
    PdbStream::ToHostByteOrder(&rec0->num_annotations);
    PdbStream::ToHostByteOrder(&rec0->an_record);
    PdbStream::ToHostByteOrder(&rec0->crc32);
}

bool ZtxtStream::ZDecompress(unsigned char* in_buf,
                             unsigned int   in_size,
                             unsigned char* out_buf,
                             unsigned int   out_size)
{
    z_stream zstream;
    zstream.zalloc    = Z_NULL;
    zstream.zfree     = Z_NULL;
    zstream.opaque    = Z_NULL;
    zstream.next_in   = in_buf;
    zstream.next_out  = out_buf;
    zstream.avail_in  = in_size;
    zstream.avail_out = out_size;

    if (inflateInit2(&zstream, MAXWBITS) != Z_OK)
    {
        return false;
    }

    int ret = inflate(&zstream, Z_SYNC_FLUSH);
    if ((ret != Z_STREAM_END) && (ret != Z_OK))
    {
        return false;
    }

    return true;
}

};
