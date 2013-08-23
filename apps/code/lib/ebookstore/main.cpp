#include "content.h"

int main(int argc, char *argv[])
{
    OContent c;
    int s = sizeof(c);
    QVector<OContent> children = c.children();
    foreach(OContent i, children)
    {
        i.children().push_back(OContent());
    }
    return 0;
}
