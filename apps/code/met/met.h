
#ifndef METADATA_EXTRACTION_TOOL_H_
#define METADATA_EXTRACTION_TOOL_H_

#include "plugin_interface.h"

namespace met
{

// Metadata extraction tool
class Manager
{
public:
    Manager();
    ~Manager();

public:
    bool extract(const QString & path);

private:
    void loadPlugins();

private:
    Plugins plugins_;

};

};


#endif

