
#include "onyx/ui/paginator.h"

int main(int argc, char *argv[])
{
    Paginator paginator;
    paginator.reset(0, 4, 10);


    paginator.isNextEnable();
    paginator.isPrevEnable();

    paginator.pages();

    paginator.next();
    paginator.currentPage();

    paginator.prev();
    paginator.currentPage();

    return 0;
}

