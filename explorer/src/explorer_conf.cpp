#include "explorer_conf.h"

namespace explorer
{

ExplorerOption::ExplorerOption()
    : settings_(QSettings::IniFormat,
                QSettings::UserScope,
                "onyx", "explorer")
{
    qDebug("Settings at %s", qPrintable(settings_.fileName()));
}

ExplorerOption::~ExplorerOption()
{
}

QSettings & explorerOption()
{
    return ExplorerOption::instance().settings();
}

}

