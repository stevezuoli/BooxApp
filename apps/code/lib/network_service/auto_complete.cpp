#include "auto_complete.h"

namespace network_service
{

static QString EncryptPassword(QString str)
{
    QByteArray xor1(QString("X!2$6*9(SKiasb+!v<.qF58_qwe~QsRTYvdeTYb").toUtf8());
    QString HEX("0123456789ABCDEF");
    wchar_t arr[256];
    memset(arr,0,sizeof(arr));
    int size = 0;//str.toWCharArray(arr);

    QString out;

    for (int i = 0; i < size && i < xor1.size(); i++)
    {
        arr[i] = arr[i] ^ xor1.at(i);
        for (int j=0; j<4; j++)
        {
            int m = arr[i] % 16;
            out.append( HEX[ m ]);
            arr[i] = arr[i] >> 4;
        }
    }
    return out;
}

static QString DecryptPassword(QString str)
{
    QByteArray xor1 (QString("X!2$6*9(SKiasb+!v<.qF58_qwe~QsRTYvdeTYb").toUtf8());
    wchar_t arr[256];
    memset(arr,0,sizeof(arr));

    int cnt = 0;
    for(int i = 0; i < str.size(); i+=4)
    {
        for (int j = 0; j < 4 ; j++)
        {
            char c = str[i + j].toAscii();
            int v = 0;
            if (c >= '0' && c <= '9')
                v = c - '0';
            else
            if (c >= 'A' && c <= 'F')
                v = 10 + c - 'A';

            int val = ( v << (j * 4));
            arr[cnt] = arr[cnt] + val;
        }
        arr[cnt] = arr[cnt] ^ xor1[cnt];
        cnt++;
    }
    return QString();//::fromWCharArray(arr);
}


AutoComplete::AutoComplete()
{
}

AutoComplete::~AutoComplete()
{
}

AutoComplete * AutoComplete::instance()
{
    static AutoComplete auto_complete;
    return &auto_complete;
}

QString AutoComplete::formUrl(QUrl url)
{
    if (url.isEmpty())
        return "";

    QString u = url.scheme() + "://" + url.host() + url.path();
    u = QUrl::toPercentEncoding(u);
    return u;
}

void AutoComplete::setFormHtml(QUrl url, QString data)
{
    forms_[formUrl(url)] = data;
}

bool AutoComplete::existInForms(QUrl url)
{
    QString u = formUrl(url);
    return forms_.contains(u);
}

void AutoComplete::setFormData(QUrl url, QString data)
{
    QString origin_data(QByteArray::fromPercentEncoding(data.toLocal8Bit()));
    forms_data_[formUrl(url)] = origin_data;
}

bool AutoComplete::existInFormsData(QUrl url)
{
    QString u = formUrl(url);
    return forms_data_.contains(u);
}

bool AutoComplete::evaluate(QUrl form_url)
{
    QString u = formUrl(form_url);

    if (!forms_.contains(u) || !forms_data_.contains(u))
        return false;

    QString form = forms_[u];
    QString data = forms_data_[u];

    // do not process AutoComplete if empty
    if (form.isEmpty() || data.isEmpty())
        return false;

    // Populate form data
    QMap<QString, QString> data_map;
    QUrl url( "?" + data );
    QPair<QString, QString> item;
    foreach(item, url.queryItems())
    {
        QString name = item.first;
        QString value = item.second;
        value = value.replace('+', ' ');
        data_map[name.toLower()] = value;
    }

    if (data_map.size() == 0)
        return false;

    // parse Form
    QString html = form.toLower();
    int form_ind = html.indexOf("<form");
    bool pwd_found = false;
    bool name_found = false;
    QString pwd_name, user_name;
    int current_form = -1;
    QMap<QString, QString> exclude_map;
    while (form_ind != -1)
    {
        current_form++;

        QString form = html.mid(form_ind);
        int end = form.indexOf("<form", form_ind + 5);
        if (end != -1)
            form = form.left( end );

        exclude_map.clear();
        // form - pure form
        int inp_ind = form.indexOf("<input");
        while (inp_ind != -1)
        {
            QString inp = form.mid(inp_ind);
            int end = inp.indexOf(">");
            if (end != -1)
            {
                inp = inp.left(end+1);
                inp.insert(inp.length() - 1, '/');

                QDomDocument d;
                d.setContent(inp);
                QDomElement de = d.documentElement();
                if (!de.isNull())
                {
                    QDomAttr dt = de.attributeNode("type");
                    QDomAttr dn = de.attributeNode("name");

                    if (!dn.isNull())
                    {
                        QString name = dn.value();
                        QString type = dt.value();
                        if (!name.isNull() )
                        {
                            if (! data_map.contains(name) )
                                goto next_form;

                            if (type == "password")
                            {
                                pwd_found = true;
                                pwd_name  = name;
                            }
                            if (!name_found && (type == "text" || type.isNull()))
                            {
                                name_found = true;
                                user_name  = name;
                            }

                            if (!(type == "text" || type == "password" || type.isNull()))
                            {
                                exclude_map[name] = type;
                            }

                        }
                    }
                }
            }
            inp_ind = form.indexOf("<input", inp_ind + 7);
        }

        // password is found and all elements matched
        if (pwd_found)
            break;

next_form:
        form_ind = html.indexOf("<form", form_ind + 5);
    }

    QWebSettings *globalSettings = QWebSettings::globalSettings();
    if (! globalSettings->testAttribute(QWebSettings::PrivateBrowsingEnabled)) 
    {
        // Store data somewhere
        if (pwd_found && current_form != -1 && !pwd_name.isEmpty())
        {
            QSettings settings;
            settings.beginGroup(QLatin1String("websettings"));
            if (! settings.value(QLatin1String("savePasswords"), true).toBool())
            {
                return false;
            }
            settings.endGroup();

            settings.beginGroup("AutoComplete/" + u);
            QString i("%1");
            settings.setValue( "form_index_number", i.arg(current_form));
            settings.setValue( "form_password_control", pwd_name);
            settings.setValue( "form_username_control", user_name);

            bool bFirst = true;
            foreach(item, url.queryItems())
            {
                QString name = item.first;
                QString lower_name = name.toLower();
                if (exclude_map.contains( lower_name ))
                {
                    continue;
                }

                QString value = item.second;
                if (bFirst &&  lower_name != pwd_name)
                {
                    settings.setValue( "form_first_control", lower_name);
                    bFirst = false;
                }

                value = value.replace('+', ' ');
                if (lower_name == pwd_name)
                {
                    value = QUrl::fromPercentEncoding(QByteArray(value.toUtf8()));
                    settings.setValue( name, EncryptPassword( value ) );
                }
                else
                {
                    settings.setValue( name, value);
                }
            }

            settings.endGroup();
        }
    }

    return true;
}


bool AutoComplete::complete( QWebFrame * frame)
{
    if (!frame || frame->url().isEmpty())
    {
        return false;
    }

    QString url = formUrl(frame->url());
    QSettings settings;
    settings.beginGroup("AutoComplete/" + url);
    QMap<QString, QString> keys;
    QString pwd_ctl;
    foreach(QString key, settings.childKeys())
    {
        keys[key] = settings.value(key).toString();
        if (key == "form_password_control")
        {
            pwd_ctl = keys[key];
        }
    }

    QString form_index = keys["form_index_number"];
    foreach(QString key, settings.childKeys() )
    {
        if (!form_index.isEmpty() && key != "form_index_number" && key != "form_password_control" 
            && key != "form_first_control" && key != "form_username_control" )
        {
            QByteArray arr = QByteArray::fromPercentEncoding( keys[key].toUtf8() );
            QString val( arr );
            if (key.toLower() == pwd_ctl)
            {
                val = DecryptPassword(val);
            }

            if (!val.isEmpty())
            {
                QString java1 = "document.getElementById('" + key + "').value = '" + val + "';";
                QString java2 = "document.forms[" + form_index + "]." + key + ".value='" + val + "';";
                frame->evaluateJavaScript(java1);
                frame->evaluateJavaScript(java2);
            }
        }
    }

    return true;
}

}
