#include "webapp_keygen.h"

namespace webapp
{

KeyGen::KeyGen()
{
    qsrand(QDateTime::currentDateTime().toTime_t());
    candidates.push_back(QString("ABCDEFGHIJKLMNOPQRSTUVWXYZ"));
    candidates.push_back(QString("0123456789"));
    candidates.push_back(QString("abcdefghijklmnopqrstuvwxyz"));
    candidates.push_back(QString("~!@#$%^&*()-=_+"));
}

KeyGen::~KeyGen()
{
}

QString KeyGen::generateKey(int length, Complexity cpl)
{
    QString key;
    for (int i = 0; i < length; ++i)
    {
        QChar c = generateChar(cpl);
        key.push_back(c);
    }
    return key;
}

QChar KeyGen::generateChar(Complexity cpl)
{
    int row = qrand() % cpl;
    int char_idx = qrand() % candidates[row].size();
    return candidates[row].at(char_idx);
}

}
