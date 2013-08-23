
#include "met.h"



int main(int argc, char * argv[])
{
    if (argc < 2)
    {
        return -1;
    }

    QApplication app(argc, argv);

    met::Manager extractor;
    if (extractor.extract(app.arguments().at(1)))
    {
        return 0;
    }
    return -1;
}

