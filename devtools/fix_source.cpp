// Author: Hong Jiang <hjiang@18scorpii.com>

#include <iostream>
#include <string>

namespace devtools
{

bool fix_source(std::istream* input, std::ostream* output)
{
    int num_spaces = 0; // number of spaces seen
    char cur; // current character
    while(input->get(cur))
    {
        switch (cur)
        {
            case ' ':
                num_spaces++;
                break;
            case '\r':
                // '\r' only occurs in DOS files, so it's sent to
                // output
                break;
            case '\t':
                // replace with four spaces
                num_spaces +=4;
                break;
            case '\n':
                // reset space counter
                num_spaces = 0;
                output->put('\n');
                break;
            default:
                // output accumulated spaces
                for (int i = 0; i< num_spaces; i++)
                {
                    output->put(' ');
                }
                num_spaces = 0;
                output->put(cur);
        }
    }
    if (input->bad() || !input->eof())
    {
        return false;
    }
    else
    {
        return true;
    }
}

}
