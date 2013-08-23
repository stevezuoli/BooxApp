#ifndef JHWR_EUROPEN_H_
#define JHWR_EUROPEN_H_

#include "jhwr_base.h"

namespace handwriting
{

class jHWREuropean : public jHWRBase
{
public:
    jHWREuropean();
    virtual ~jHWREuropean();

public:
    virtual bool initialize(const QLocale & locale);
    virtual bool deinitialize();
    virtual bool setWorkArea(const QRect & rect);
    virtual bool setCandidateNum(const int number);
    virtual bool recognize(QStringList & results);
    virtual bool setLocale(const QLocale & locale);
    virtual bool setSpecialRecognizeRange(SpecialRecognizeRange range);

private:
    void updateLanguage(const QLocale & locale);
    bool recognizeMultipleChars(short * points, long length, QStringList & result);
    bool recognizeSingleChar(short * points, QStringList & result);
};

};
#endif

