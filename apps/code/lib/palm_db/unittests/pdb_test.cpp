#include <stdio.h>

#include "mobi_stream.h"
using namespace pdb;

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <file_path>.\n", argv[0]);
        return -1;
    }

    MobiStream stream(argv[1]);
    if (!stream.Open())
    {
        fprintf(stderr, "Error opening %s.\n", argv[1]);
        return -1;
    }

    stream.Dump();
    stream.Close();
    return 0;
}
