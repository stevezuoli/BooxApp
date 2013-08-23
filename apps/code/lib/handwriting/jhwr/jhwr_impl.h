#ifndef JHWR_PLUGIN_IMPL_H_
#define JHWR_PLUGIN_IMPL_H_

#include "onyx/base/base.h"
#include "onyx/data/handwriting_interface.h"
#include "jhwr_base.h"
#include "jhwr_associated_char.h"

namespace handwriting
{

class jHWRPlugin : public HandwritingInterface
{
    Q_OBJECT
public:
    jHWRPlugin();
    virtual ~jHWRPlugin();

public:
    virtual bool initialize(const QLocale & locale);
    virtual bool setWorkArea(const QRect & rect);
    virtual bool setCandidateNum(const int number);
    virtual bool setSpecialRecognizeRange(SpecialRecognizeRange range);
    virtual QLocale currentLocale();

    virtual void clearPoints();
    virtual void collectPoint(int x, int y);
    virtual void finishStroke();
    virtual void finishCharacter();
    virtual bool recognize(QStringList & results);

    virtual bool getAssociatedChar(const QString & current_text, QStringList & result);
    virtual bool adjustAssociatedChar(const QString & dst_text, int index);

private:
    void deinitialize();

private:
    QLocale              current_locale_;
    scoped_ptr<jHWRBase> jhwr_instance_;
    scoped_ptr<jHWRAssociatedCharMgr> jhwr_associated_char_mgr_;
};

};
#endif

