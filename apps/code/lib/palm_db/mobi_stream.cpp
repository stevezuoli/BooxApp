#include <string.h>
#include "mobi_stream.h"

namespace pdb
{

static unsigned char TOKEN_CODE[256] =
{
    0, 1, 1, 1,     1, 1, 1, 1,     1, 0, 0, 0,     0, 0, 0, 0,
    0, 0, 0, 0,     0, 0, 0, 0,     0, 0, 0, 0,     0, 0, 0, 0,
    0, 0, 0, 0,     0, 0, 0, 0,     0, 0, 0, 0,     0, 0, 0, 0,
    0, 0, 0, 0,     0, 0, 0, 0,     0, 0, 0, 0,     0, 0, 0, 0,
    0, 0, 0, 0,     0, 0, 0, 0,     0, 0, 0, 0,     0, 0, 0, 0,
    0, 0, 0, 0,     0, 0, 0, 0,     0, 0, 0, 0,     0, 0, 0, 0,
    0, 0, 0, 0,     0, 0, 0, 0,     0, 0, 0, 0,     0, 0, 0, 0,
    0, 0, 0, 0,     0, 0, 0, 0,     0, 0, 0, 0,     0, 0, 0, 0,
    3, 3, 3, 3,     3, 3, 3, 3,     3, 3, 3, 3,     3, 3, 3, 3,
    3, 3, 3, 3,     3, 3, 3, 3,     3, 3, 3, 3,     3, 3, 3, 3,
    3, 3, 3, 3,     3, 3, 3, 3,     3, 3, 3, 3,     3, 3, 3, 3,
    3, 3, 3, 3,     3, 3, 3, 3,     3, 3, 3, 3,     3, 3, 3, 3,
    2, 2, 2, 2,     2, 2, 2, 2,     2, 2, 2, 2,     2, 2, 2, 2,
    2, 2, 2, 2,     2, 2, 2, 2,     2, 2, 2, 2,     2, 2, 2, 2,
    2, 2, 2, 2,     2, 2, 2, 2,     2, 2, 2, 2,     2, 2, 2, 2,
    2, 2, 2, 2,     2, 2, 2, 2,     2, 2, 2, 2,     2, 2, 2, 2,
};

MobiStream::MobiStream(const std::string& file_path)
: PdbStream(file_path)
{
}

MobiStream::~MobiStream()
{
    for (unsigned int i=0; i<images_.size(); i++)
    {
        delete[] images_[i].data;
    }
}

bool MobiStream::FillBuffer()
{
    stream.seekg(records[0].offset, std::ios_base::beg);
    stream.read(reinterpret_cast<char *>(&rec0), sizeof(MobiRecord0));
    if (stream.eof())
    {
        return false;
    }

    // To host byte order.
    ToHostByteOrder(&rec0);

    // Check compression type.
    if (rec0.compress == 17480)
    {
        // TODO: DRMed book, not supported for now.
        return false;
    }

    if (rec0.compress == 1)
    {
        // Uncompressed.
        return PdbStream::FillBuffer();
    }

    // PalmDOC compressed.
    buf = new char[rec0.size + 64];

    for (unsigned int i=1; i<rec0.first_non_book_index; i++)
    {
        stream.seekg(records[i].offset, std::ios_base::beg);
        if (stream.eof())
        {
            goto Error;
        }

        char compressed[4096];
        size_t bytes_to_read = records[i+1].offset - records[i].offset;
        stream.read(compressed, static_cast<std::streamsize>(bytes_to_read));

        size += LZ77Decompress(reinterpret_cast<unsigned char *>(compressed),
                               bytes_to_read,
                               reinterpret_cast<unsigned char *>(buf + size),
                               rec0.size - size);
        while (buf[size - 1] == '\0')
        {
            size--;
        }
    }

    // Read images.
    ReadImages();
    return true;

Error:
    delete[] buf;
    buf = 0;
    return false;
}

void MobiStream::ReadImages()
{
    for (unsigned int index = rec0.first_image_index; index < records.size(); index++)
    {
        stream.seekg(records[index].offset, std::ios_base::beg);
        if (stream.eof())
        {
            break;
        }

        char bu[4];
        stream.read(bu, 4);
        static const char jpegStart[2] = { (char)0xFF, (char)0xd8 };
        if ((strncmp(bu, "BM", 2) == 0) ||
            (strncmp(bu, "GIF8", 4) == 0) ||
            (strncmp(bu, jpegStart, 2) == 0))
        {
            // Found an image.
            Image img;
            img.size = records[index+1].offset - records[index].offset;
            img.data = new char[img.size];

            stream.seekg(records[index].offset, std::ios_base::beg);
            stream.read(img.data, static_cast<std::streamsize>(img.size));

            // Add image to image list.
            images_.push_back(img);
        }
    }
}

void MobiStream::ToHostByteOrder(MobiRecord0* rec0)
{
    PdbStream::ToHostByteOrder(&rec0->compress);
    PdbStream::ToHostByteOrder(&rec0->size);
    PdbStream::ToHostByteOrder(&rec0->num_records);
    PdbStream::ToHostByteOrder(&rec0->max_record_size);
    PdbStream::ToHostByteOrder(&rec0->current_pos);

    PdbStream::ToHostByteOrder(&rec0->header_len);
    PdbStream::ToHostByteOrder(&rec0->mobi_type);
    PdbStream::ToHostByteOrder(&rec0->text_encoding);
    PdbStream::ToHostByteOrder(&rec0->unique_id);
    PdbStream::ToHostByteOrder(&rec0->ver1);
    PdbStream::ToHostByteOrder(&rec0->first_non_book_index);
    PdbStream::ToHostByteOrder(&rec0->full_name_offset);
    PdbStream::ToHostByteOrder(&rec0->full_name_len);
    PdbStream::ToHostByteOrder(&rec0->language);
    PdbStream::ToHostByteOrder(&rec0->ver2);
    PdbStream::ToHostByteOrder(&rec0->first_image_index);
    PdbStream::ToHostByteOrder(&rec0->exth_flag);
}

size_t MobiStream::LZ77Decompress(unsigned char* in_buf,
                                 unsigned int   in_size,
                                 unsigned char* out_buf,
                                 unsigned int   out_size)
{
    const unsigned char *sourceBufferEnd = in_buf + in_size;
    const unsigned char *sourcePtr = in_buf;
    unsigned char *targetPtr = out_buf;
    unsigned char *targetBufferEnd = out_buf + out_size;
    unsigned char token;
    unsigned short copyLength, N, shift;
    unsigned char *shifted;

    while ((sourcePtr < sourceBufferEnd) && (targetPtr < targetBufferEnd))
    {
        token = *(sourcePtr++);
        switch (TOKEN_CODE[token])
        {
        case 0:
            *(targetPtr++) = token;
            break;
        case 1:
            if ((sourcePtr + token > sourceBufferEnd) ||
                (targetPtr + token > targetBufferEnd))
            {
                goto endOfLoop;
            }
            memcpy(targetPtr, sourcePtr, token);
            sourcePtr += token;
            targetPtr += token;
            break;
        case 2:
            if (targetPtr + 2 > targetBufferEnd)
            {
                goto endOfLoop;
            }
            *(targetPtr++) = ' ';
            *(targetPtr++) = token ^ 0x80;
            break;
        case 3:
            if (sourcePtr + 1 > sourceBufferEnd)
            {
                goto endOfLoop;
            }
            N = 256 * token + *(sourcePtr++);
            copyLength = (N & 7) + 3;
            if (targetPtr + copyLength > targetBufferEnd)
            {
                goto endOfLoop;
            }
            shift = (N & 0x3fff) / 8;
            shifted = targetPtr - shift;
            if (reinterpret_cast<unsigned char *>(shifted) >= out_buf)
            {
                for (short i = 0; i < copyLength; i++)
                {
                    *(targetPtr++) = *(shifted++);
                }
            }
            break;
        }
    }

endOfLoop:
    return targetPtr - out_buf;
}

};
