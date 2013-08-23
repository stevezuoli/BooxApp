#include "jhwr_associated_char.h"
#include "TwoCharWord.h"

namespace handwriting
{

static QString getLanModelPath()
{
#ifdef BUILD_FOR_ARM
    QDir dir(SHARE_ROOT);
#else
    QDir dir(QDir::home());
#endif
    dir.cd("handwriting");
    {
        QString path = dir.filePath("model.dat");
        if (QFile::exists(path))
        {
            return path;
        }
    }
    return QString();
}

jHWRAssociatedCharMgr::jHWRAssociatedCharMgr()
: candidates_num_(20)
, lan_model_file_len_(0)
, lan_model_data_(0)
{
}

jHWRAssociatedCharMgr::~jHWRAssociatedCharMgr()
{
    deinitialize();
}

bool jHWRAssociatedCharMgr::initialize(const QLocale & locale)
{
    QString path = getLanModelPath();
    qDebug("Open lan model file:%s", qPrintable(path));
    if (path.isEmpty())
    {
        return false;
    }

    FILE *fp = fopen(path.toUtf8().constData(), "rb");
    if (fp == NULL)
    {
        return false;
    }

    // read data from lan model file
    fseek(fp,0,SEEK_END);
    lan_model_file_len_ = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    lan_model_data_ = (unsigned char*)malloc(lan_model_file_len_);
    fread(lan_model_data_, 1, lan_model_file_len_, fp);
    fclose(fp);

#ifdef BUILD_FOR_ARM
    // set code page to be unicode
    jtTwoCharWord_SetParam_CodePage(jtHWR_FUNC_CODEPAGE_UNICODE);
    qDebug("Initialize Lan Model successfully");
#endif
    return true;
}

bool jHWRAssociatedCharMgr::deinitialize()
{
    if (lan_model_data_ == 0)
    {
        return false;
    }

    // save data to lan model file
    QString path = getLanModelPath();
    qDebug("Saving lan model file:%s", qPrintable(path));
    if (path.isEmpty())
    {
        return false;
    }

    FILE *fp = fopen(path.toUtf8().constData(), "wb");
    if (fp == NULL)
    {
        return false;
    }

    size_t ret = fwrite(lan_model_data_, 1, lan_model_file_len_, fp);
    fclose(fp);
    qDebug("Write lan mode data file:%d, Count:%d", ret, lan_model_file_len_);
    return ret == lan_model_file_len_;
}

bool jHWRAssociatedCharMgr::setCandidateNum(const int number)
{
    if (number <= 0)
    {
        return false;
    }

    candidates_num_ = number;
    return true;
}

bool jHWRAssociatedCharMgr::getAssociatedChar(const QString & current_text, QStringList & result)
{
    if (current_text.isEmpty())
    {
        return false;
    }

    QChar current_char = current_text[current_text.size() - 1];
    unsigned short current_code = current_char.unicode();

    unsigned short * code_array = new unsigned short[candidates_num_ + 20];
    memset(code_array, 0, candidates_num_ + 20);

#ifdef BUILD_FOR_ARM
    int num_associated_chars = jtTwoCharWord_GetSuffixAssociateWord(code_array, candidates_num_, current_code, lan_model_data_);
    if (num_associated_chars <= 0)
    {
        qWarning("Could not get associated characters.");
        return false;
    }

    for (int i = 0; i < num_associated_chars; i++)
    {
        qDebug("Associated: %d", code_array[i]);
        result.push_back(QChar(code_array[i]));
    }
#endif

    delete []code_array;
    return true;
}

bool jHWRAssociatedCharMgr::adjustAssociatedChar(const QString & dst_text, int index)
{
    if (dst_text.isEmpty())
    {
        return false;
    }

    QChar dst_char = dst_text[dst_text.size() - 1];
    unsigned short dst_code = dst_char.unicode();

    // adjust the position of associated word by the frequence
    int ret = 0;

#ifdef BUILD_FOR_ARM
    ret = jtTwoCharWord_AdjustWordFrequency(dst_code, index, 1, lan_model_data_);
    qDebug("Adjust word frequency:%d", ret);
#endif

    return ret > 0;
}

}
