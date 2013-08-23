#ifndef ZTXT_STREAM_H_
#define ZTXT_STREAM_H_

#include "pdb_stream.h"

namespace pdb
{

#pragma pack(push)
#pragma pack(1)

struct ZTxtRecord0
{
    UINT16        version;
    UINT16        num_records;
    UINT32        size;
    UINT16        record_size;
    UINT16        num_bookmarks;
    UINT16        bm_record;
    UINT16        num_annotations;
    UINT16        an_record;
    unsigned char flags;
    unsigned char reserved;
    UINT32        crc32;
    unsigned char padding[8];
};

#pragma pack(pop)

class ZtxtStream : public PdbStream
{
public:
    ZtxtStream(const std::string& file_path);
    ~ZtxtStream();

protected:
    virtual bool FillBuffer();

private:
    static void ToHostByteOrder(ZTxtRecord0* rec0);

    /// @brief Decompress using zlib.
    /// @param in_buf The compressed data (to be decompressed).
    /// @param in_size The compressed data size.
    /// @param out_buf The output buffer, will be filled with decompressed data.
    /// @param out_size The output buffer size.
    /// @return True if successfully decompressed, otherwise false is returned.
    bool ZDecompress(unsigned char* in_buf,
                     unsigned int   in_size,
                     unsigned char* out_buf,
                     unsigned int   out_size);

private:
    ZTxtRecord0 rec0;
};

};

#endif // ZTXT_STREAM_H_
