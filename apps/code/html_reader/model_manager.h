
#ifndef MODEL_MANAGER_H_
#define MODEL_MANAGER_H_

#include <QtGui/QtGui>
#include "model_interface.h"

namespace reader
{

/// Model
class ModelManager
{
public:
    static ModelManager & instance()
    {
        static ModelManager instance_;
        return instance_;
    }
    ~ModelManager();

public:
    ModelInterface * createInstance(const QString &path);

private:
    ModelManager();
    ModelManager(const ModelManager &){}
};

}   // namespace reader

#endif  // MODEL_MANAGER_H_
