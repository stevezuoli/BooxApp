#ifndef EXPLORER_CONFIGURATION_H__
#define EXPLORER_CONFIGURATION_H__

#include "onyx/ui/ui_global.h"

namespace explorer
{

class ExplorerOption
{
public:
    ~ExplorerOption();
    static ExplorerOption & instance()
    {
        static ExplorerOption instance_;
        return instance_;
    }

public:
    inline QSettings & settings() { return settings_; }

private:
    ExplorerOption();
    ExplorerOption(const ExplorerOption&);

private:
    QSettings settings_;
};

QSettings & explorerOption();

}  // namespace explorer

#endif  // EXPLORER_CONFIGURATION_H__
