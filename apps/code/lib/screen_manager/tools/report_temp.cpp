
#include <stdio.h>
#include <stdlib.h>
#include "screen_manager/screen_manager.h"


int main(int argc, char **argv)
{
    printf("Current temperature %d", screen::instance().temperature());
    return 0;
}
