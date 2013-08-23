#include <string.h>
#include <sys/stat.h>
#include "pdb_stream.h"

namespace pdb
{

PdbStream::PdbStream(const std::string& file_path)
: stream(file_path.c_str(), std::ios_base::in | std::ios_base::binary)
, raw_size(0)
, size(0)
, buf(0)
, file_path(file_path)
, pos(0)
, is_open(false)
{
    // Get raw file size.
    struct stat stat_buf;
    stat(file_path.c_str(), &stat_buf);
    raw_size = stat_buf.st_size;
}

PdbStream::~PdbStream()
{
    if (is_open)
    {
        Close();
    }
}

// Get the database type without actually open the file.
DBType PdbStream::GetDBType(const std::string& file_path)
{
    DBType type = DB_UNKNOWN;
    std::ifstream ifs(file_path.c_str(), std::ios_base::in | std::ios_base::binary);

    if (ifs)
    {
        PdbHeader header;
        ifs.read(reinterpret_cast<char *>(&header), sizeof(PdbHeader));

        ToHostByteOrder(&header);
        type = ExtractType(header);
    }

    return type;
}

// Open pdb file.
bool PdbStream::Open()
{
    if (!stream)
    {
        return false;
    }

    // Read pdb header.
    stream.read(reinterpret_cast<char *>(&pdb_header), sizeof(PdbHeader));
    ToHostByteOrder(&pdb_header);
    if (stream.eof())
    {
        return false;
    }

    // Read record list.
    records.resize(pdb_header.num_records);
    stream.read(reinterpret_cast<char *>(&records[0]), sizeof(RecEntry) * pdb_header.num_records);
    if (stream.eof())
    {
        return false;
    }

    // To host byte order.
    for (unsigned int i=0; i<records.size(); i++)
    {
        ToHostByteOrder(&records[i]);
    }

    // Read content to internal buffer.
    if (!FillBuffer())
    {
        return false;
    }

    return is_open = true;
}

void PdbStream::Close()
{
    if (buf)
    {
        delete[] buf;
        buf = 0;
    }

    is_open = false;
}

size_t PdbStream::Read(char* dst, size_t bytes_to_read)
{
    // Make sure the file is already opened.
    if (!is_open)
    {
        return 0;
    }

    size_t bytes_read = std::min(size - pos, bytes_to_read);
    memcpy(dst, buf + pos, bytes_read);
    pos += bytes_read;
    return bytes_read;
}

DBType PdbStream::GetDBType() const
{
    if (!is_open)
    {
        return DB_UNKNOWN;
    }

    return ExtractType(pdb_header);
}

void PdbStream::Dump()
{
    std::string dump_file = file_path + ".txt";
    std::ofstream out(dump_file.c_str(), std::ios_base::out);

    if (!out)
    {
        return;
    }

    out.write(buf, size);
    out.close();
}

// Read content to internal buffer, update size and buf as well.
bool PdbStream::FillBuffer()
{
    size = raw_size - records[1].offset;
    buf = new char[size];

    stream.seekg(records[1].offset, std::ios_base::beg);
    stream.read(buf, size);
    return true;
}

void PdbStream::ToHostByteOrder(UINT16* val)
{
    unsigned char* p = reinterpret_cast<unsigned char*>(val);
    unsigned char tmp = p[0];
    p[0] = p[1];
    p[1] = tmp;
}

DBType PdbStream::ExtractType(const PdbHeader& header)
{
    DBType type = DB_UNKNOWN;
    if (strncmp(header.db_type, "TEXt", 4) == 0)
    {
        type = DB_TEXT;
    }
    else if (strncmp(header.db_type, "zTXT", 4) == 0)
    {
        type = DB_ZTXT;
    }
    else if (strncmp(header.db_type, ".pdf", 4) == 0)
    {
        type = DB_PDF;
    }
    else if (strncmp(header.db_type, "BOOK", 4) == 0)
    {
        type = DB_MOBI;
    }

    return type;
}

void PdbStream::ToHostByteOrder(UINT32* val)
{
    unsigned char* p = reinterpret_cast<unsigned char*>(val);
    unsigned char tmp = p[0];
    p[0] = p[3];
    p[3] = tmp;
    tmp = p[1];
    p[1] = p[2];
    p[2] = tmp;
}

void PdbStream::ToHostByteOrder(PdbHeader* header)
{
    ToHostByteOrder(&header->attr);
    ToHostByteOrder(&header->version);
    ToHostByteOrder(&header->create_time);
    ToHostByteOrder(&header->modify_time);
    ToHostByteOrder(&header->last_backup_time);
    ToHostByteOrder(&header->modification_num);
    ToHostByteOrder(&header->app_info);
    ToHostByteOrder(&header->sort_info);
    ToHostByteOrder(&header->id_seed);
    ToHostByteOrder(&header->next_rec_list);
    ToHostByteOrder(&header->num_records);
}

void PdbStream::ToHostByteOrder(RecEntry* rec_entry)
{
    ToHostByteOrder(&rec_entry->offset);
}

}
