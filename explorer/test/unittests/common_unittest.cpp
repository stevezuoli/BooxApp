// Copyright 2007 Company Name. All Rights Reserved.
// Author: John

#include <boost/filesystem.hpp>
#include <fstream>
#include "onyx/base/base.h"
#include "common_unittest.h"

namespace fs = boost::filesystem;

namespace common_unittest
{

void CreateTempFile(const base::string & path_name,
                    const unsigned int size)
{
    std::ofstream file(path_name.c_str());
    char * data = new char[size];
    file.write(data, size);
    file.flush();
    delete [] data;
}

void RemoveFile(const base::string & path_name)
{
    boost::filesystem::remove(path_name);
}

void CreateTempDirectory(const boost::filesystem::path & path_name,
                         const FileInfo * tmp_files,
                         const int size)
{
    // Create temp directory.
    fs::remove_all(path_name);
    fs::create_directory(path_name);

    for(int i = 0; i < size; ++i)
    {
        CreateTempFile((path_name / tmp_files[i].path).string().c_str(),
                        tmp_files[i].size);
    }
}

void RemoveDirectory(const base::string &path)
{
    boost::filesystem::remove_all(path);
}



}
