
#include "model_manager.h"
#include "chm/chm_model_impl.h"
// #include "html/html_model_impl.h"
// #include "mobi/mobi_model_impl.h"

namespace reader
{

ModelManager::ModelManager()
{
}

ModelManager::~ModelManager()
{
}

ModelInterface * ModelManager::createInstance(const QString &path)
{
    if (path.endsWith(".chm", Qt::CaseInsensitive))
    {
        return new chm::ChmModelImpl;
    }
    /*
    John: Disabled, we're using onyx_read to open them now.
    else if (path.endsWith(".htm") || path.endsWith(".html"))
    {
        return new html::HtmlModelImpl;
    }
    else if (path.endsWith(".mobi") || path.endsWith(".prc"))
    {
        return new mobipocket::MobiModelImpl;
    }
    */
    return 0;
}


}   // namespace reader

