#ifndef PASSWORD_MODEL_H
#define PASSWORD_MODEL_H

#include "ns_utils.h"

namespace network_service
{

class PasswordModel
{
public:
    PasswordModel();
    ~PasswordModel();

    void getModel(QStandardItemModel & passwords_model);
    bool removePassword(const QString & url);

private:
    QSettings   data_;
};

};

#endif
