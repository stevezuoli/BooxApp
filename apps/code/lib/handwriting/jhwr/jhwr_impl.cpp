#include "jhwr_impl.h"
#include "jhwr_chinese.h"
#include "jhwr_europe.h"
#include "jhwr_japanese.h"

namespace handwriting
{

jHWRPlugin::jHWRPlugin()
    : jhwr_instance_(0)
    , jhwr_associated_char_mgr_(new jHWRAssociatedCharMgr())
{
    initialize(QLocale::system());
    jhwr_associated_char_mgr_->initialize(QLocale::system());
}

jHWRPlugin::~jHWRPlugin()
{
    deinitialize();
}

bool jHWRPlugin::initialize(const QLocale & locale)
{
    if (current_locale_ == locale && jhwr_instance_ != 0)
    {
        return jhwr_instance_->setLocale(current_locale_);
    }
    current_locale_ = locale;

    if (locale.language() == QLocale::Chinese || locale.country() == QLocale::China ||
        locale.country() == QLocale::Taiwan || locale.country() == QLocale::HongKong)
    {
        jhwr_instance_.reset(new jHWRChinese());
    }
    else if (locale.language() == QLocale::Japanese)
    {
        jhwr_instance_.reset(new jHWRJapanese());
    }
    else
    {
        jhwr_instance_.reset(new jHWREuropean());
    }

    return jhwr_instance_->initialize(locale);
}

void jHWRPlugin::deinitialize()
{
    if (jhwr_instance_ != 0)
    {
        jhwr_instance_->deinitialize();
    }
}

bool jHWRPlugin::setWorkArea(const QRect & rect)
{
    if (jhwr_instance_ != 0)
    {
        return jhwr_instance_->setWorkArea(rect);
    }
    return false;
}

bool jHWRPlugin::setCandidateNum(const int number)
{
    if (jhwr_instance_ != 0)
    {
        return jhwr_instance_->setCandidateNum(number);
    }
    return false;
}

bool jHWRPlugin::setSpecialRecognizeRange(SpecialRecognizeRange range)
{
    if (jhwr_instance_ != 0)
    {
        return jhwr_instance_->setSpecialRecognizeRange(range);
    }
    return false;
}

QLocale jHWRPlugin::currentLocale()
{
    return current_locale_;
}

void jHWRPlugin::clearPoints()
{
    if (jhwr_instance_ != 0)
    {
        jhwr_instance_->clearPoints();
    }
}

void jHWRPlugin::collectPoint(int x, int y)
{
    if (jhwr_instance_ != 0)
    {
        jhwr_instance_->collectPoint(x, y);
    }
}

bool jHWRPlugin::recognize(QStringList & result)
{
    if (jhwr_instance_ != 0)
    {
        return jhwr_instance_->recognize(result);
    }
    return false;
}

void jHWRPlugin::finishStroke()
{
    if (jhwr_instance_ != 0)
    {
        jhwr_instance_->finishStroke();
    }
}

void jHWRPlugin::finishCharacter()
{
    if (jhwr_instance_ != 0)
    {
        jhwr_instance_->finishCharacter();
    }
}

bool jHWRPlugin::getAssociatedChar(const QString & current_text, QStringList & result)
{
    if (jhwr_associated_char_mgr_ != 0)
    {
        return jhwr_associated_char_mgr_->getAssociatedChar(current_text, result);
    }
    return false;
}

bool jHWRPlugin::adjustAssociatedChar(const QString & dst_text, int index)
{
    if (jhwr_associated_char_mgr_ != 0)
    {
        return jhwr_associated_char_mgr_->adjustAssociatedChar(dst_text, index);
    }
    return false;
}

}

Q_EXPORT_PLUGIN2(jhwr_plugin, handwriting::jHWRPlugin)

