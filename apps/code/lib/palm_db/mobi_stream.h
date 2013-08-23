#ifndef MOBI_STREAM_H_
#define MOBI_STREAM_H_

#include "pdb_stream.h"

namespace pdb
{

#pragma pack(push)
#pragma pack(1)

struct MobiRecord0
{
    // PalmDOC header part.
    UINT16        compress;
    UINT16        reserved1;
    UINT32        size;
    UINT16        num_records;
    UINT16        max_record_size;
    UINT32        current_pos;

    // Mobipocket header.
    unsigned char identifier[4];
    UINT32        header_len;
    UINT32        mobi_type;
    UINT32        text_encoding;
    UINT32        unique_id;
    UINT32        ver1;             // ? Not sure.
    unsigned char reserved2[40];
    UINT32        first_non_book_index;
    UINT32        full_name_offset;
    UINT32        full_name_len;
    UINT32        language;
    unsigned char unknown1[8];
    UINT32        ver2;
    UINT32        first_image_index;
    unsigned char unknown2[16];
    UINT32        exth_flag;
};

#pragma pack(pop)

struct Image
{
    char*  data;
    size_t size;
};

class MobiStream : public PdbStream
{
public:
    MobiStream(const std::string& file_path);
    ~MobiStream();

public:
    const std::vector<Image> & images() const
    {
        return images_;
    }

protected:
    virtual bool FillBuffer();

private:
    void ReadImages();
    static void ToHostByteOrder(MobiRecord0* rec0);

    /// @brief Decompress files compressed with LZ77 algorithm.
    /// @param in_buf The compressed data (to be decompressed).
    /// @param in_size The compressed data size.
    /// @param out_buf The output buffer, will be filled with decompressed data.
    /// @param out_size The output buffer size.
    /// @return True if successfully decompressed, otherwise false is returned.
    size_t LZ77Decompress(unsigned char* in_buf,
                          unsigned int   in_size,
                          unsigned char* out_buf,
                          unsigned int   out_size);

private:
    MobiRecord0 rec0;
    std::vector<Image> images_;
};

};

#endif // MOBI_STREAM_H_
