#ifndef JHWR_ASSOCIATED_CHAR_H_
#define JHWR_ASSOCIATED_CHAR_H_

#include <QtGui/QtGui>

namespace handwriting
{

class jHWRAssociatedCharMgr
{
public:
    jHWRAssociatedCharMgr();
    virtual ~jHWRAssociatedCharMgr();

public:
    bool initialize(const QLocale & locale);
    bool deinitialize();
    bool setCandidateNum(const int number);

    bool getAssociatedChar(const QString & current_text, QStringList & result);
    bool adjustAssociatedChar(const QString & dst_text, int index);

protected:
    int                   candidates_num_;
    int                   lan_model_file_len_;
    unsigned char         *lan_model_data_;
};

};
#endif
