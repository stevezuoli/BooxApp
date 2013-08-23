#include "onyx/base/ui.h"

void DumpForLocale()
{
    QFontDatabase db;

    // Try to get locale at first.
    // Try to get the language and then get the writing system.
    QStringList list;
    switch (QLocale::system().language())
    {
    case QLocale::Chinese:
        list = db.families(QFontDatabase::SimplifiedChinese);
        break;
    case QLocale::English:
        list = db.families(QFontDatabase::Latin);
        break;
    default:
        break;
    }
    qDebug() << list;
}

int main(int argc, char * argv[])
{
    DumpForLocale();

    return 0;
}

