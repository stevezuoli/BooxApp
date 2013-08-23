#ifndef JHWR_JAPANESE_H_
#define JHWR_JAPANESE_H_

#include "jhwr_base.h"

namespace handwriting
{

class jHWRJapanese : public jHWRBase
{
public:
    jHWRJapanese();
    virtual ~jHWRJapanese();

public:
    virtual bool initialize(const QLocale & locale);
    virtual bool deinitialize();
    virtual bool setWorkArea(const QRect & rect);
    virtual bool setCandidateNum(const int number);
    virtual bool recognize(QStringList & results);
    virtual bool setLocale(const QLocale & locale);
    virtual bool setSpecialRecognizeRange(SpecialRecognizeRange range);

private:
    bool recognizeMultipleChars(short * points, long length, QStringList & result);
    bool recognizeSingleChar(short * points, QStringList & result);
};

};
#endif

