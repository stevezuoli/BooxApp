
#include <stdio.h>
#include <stdlib.h>
#include "screen_manager.h"

using namespace screen;

int main(int argc, char **argv)
{
    if (argc == 2)
    {
        ScreenManager::instance().snapshot(argv[1]);
    }
    else
    {
        ScreenManager::instance().snapshot("snapshot.png");
    }
    return 0;
}
