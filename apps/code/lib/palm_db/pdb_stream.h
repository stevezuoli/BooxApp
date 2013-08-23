#ifndef PDB_STREAM_H_
#define PDB_STREAM_H_

#include <string>
#include <vector>
#include <fstream>

namespace pdb
{

static const int DB_NAME_LENGTH = 32;
typedef unsigned int   UINT32;
typedef unsigned short UINT16;

enum DBType
{
    DB_UNKNOWN,
    DB_PDF,
    DB_TEXT,
    DB_MOBI,
    DB_ZTXT
};

#pragma pack(push)
#pragma pack(1)

struct PdbHeader
{
    char   name[DB_NAME_LENGTH];
    UINT16 attr;
    UINT16 version;
    UINT32 create_time;
    UINT32 modify_time;
    UINT32 last_backup_time;
    UINT32 modification_num;
    UINT32 app_info;
    UINT32 sort_info;
    char   db_type[4];
    char   creator[4];
    UINT32 id_seed;
    UINT32 next_rec_list;
    UINT16 num_records;
};

struct RecEntry
{
    UINT32        offset;
    unsigned char attr;
    unsigned char unique_id[3];
};

#pragma pack(pop)

class PdbStream
{
public:
    PdbStream(const std::string& file_path);
    virtual ~PdbStream();

public:
    static DBType GetDBType(const std::string& file_path);

public:
    bool Open();
    void Close();
    size_t Read(char* buf, size_t bytes_to_read);
    bool End() const
    {
        return pos == size;
    }

    DBType GetDBType() const;

    size_t GetSize() const
    {
        return size;
    }

    /// @brief Dump content to disk file, for test only.
    void Dump();

protected:
    virtual bool FillBuffer();
    static void ToHostByteOrder(UINT16* val);
    static void ToHostByteOrder(UINT32* val);

protected:
    std::ifstream stream;
    std::vector<RecEntry> records;
    size_t  raw_size;
    size_t  size;
    char*   buf;

private:
    static DBType ExtractType(const PdbHeader& header);
    static void ToHostByteOrder(PdbHeader* header);
    static void ToHostByteOrder(RecEntry* rec_entry);

private:
    std::string file_path;
    PdbHeader pdb_header;
    size_t  pos;
    bool is_open;
};

};

#endif // PDB_STREAM_H_
