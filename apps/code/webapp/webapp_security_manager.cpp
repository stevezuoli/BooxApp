#include "private_conf/conf.h"
#include "onyx/sys/sys_conf.h"
#include "private_conf/conf.h"
#include "webapp_security_manager.h"
#include "webapp_keygen.h"

using namespace sys;

namespace webapp
{

SecurityManager::SecurityManager(QObject *parent)
{
}

SecurityManager::~SecurityManager()
{
}

QVariantList SecurityManager::generateCredentials()
{
    QVariantList credentials;
    KeyGen key_gen;
    QString auth_key = key_gen.generateKey(32, KeyGen::COMPLEX);
    QString otp = key_gen.generateKey(8, KeyGen::SIMPLE);
    credentials.push_back(auth_key);
    credentials.push_back(otp);

    PrivateConfig private_conf;
    QStringList credential_strings;
    credential_strings.push_back(auth_key);
    credential_strings.push_back(otp);
    private_conf.setCredentials(credential_strings);
    return credentials;
}

QString SecurityManager::getClientId()
{
    SystemConfig conf;
    return conf.serialNumber();
}

QString SecurityManager::getAuthKey()
{
    QString auth_key;
    PrivateConfig private_conf;
    QStringList credentials = private_conf.getCredentials();
    if (!credentials.isEmpty())
    {
        auth_key = credentials[0];
    }
    return auth_key;
}

}
