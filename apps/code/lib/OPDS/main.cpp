#include "feedmanager.h"

#include <QCoreApplication>
#include <QStringList>
#include <stdio.h>


int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QStringList arguments = app.arguments();
    arguments.takeFirst();      // remove the first argument, which is the program's name

    if (arguments.isEmpty()) 
    {
        return 0;
    }

    FeedManager::instance().requestFeed(arguments.front());

    return app.exec();
}
