// Author: Hong Jiang <18scorpii.com>

/// Just a simple driver for fix_source() (in fix_source.h)

#include <fstream>
#include <iostream>

#include <boost/filesystem.hpp>

#include "fix_source.h"

using devtools::fix_source;
using std::fstream;
using std::string;

namespace fs = boost::filesystem;

bool fix_file(const string& filename)
{
    const string temp_filename(filename + ".old");
    try
    {
        fs::rename(filename, temp_filename);
    }
    catch (const fs::filesystem_error& e)
    {
         std::cerr << "Error renaming " << filename << " to " << temp_filename
                   << ".\nCheck if " << temp_filename << " already exists."
                   << std::endl;
        return false;
    }
    fstream in(temp_filename.c_str(), fstream::in | fstream::binary);
    fstream out(filename.c_str(), fstream::out | fstream::binary);
    return fix_source(&in, &out);
}

int main(int argc, char** argv)
{
    int result = 0;
    if (argc == 1) {
        if (!fix_source(&std::cin, &std::cout))
        {
            result++;
        }
    }
    else
    {
        for (int i = 1; i < argc; i++)
        {
            if (!fix_file(argv[i]))
            {
                std::cerr << "An error occured when modifying file: "
                          << argv[i]
                          << std::endl;
                result++;
            }
            else
            {
                std::cout << argv[i]
                          << " has been modified. A backup is saved as "
                          << argv[i] << ".old" << std::endl;
            }
        }
    }
    return result;
}
