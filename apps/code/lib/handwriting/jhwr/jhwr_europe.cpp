#define CPP_SOURCE
#include "MultiLanguage_HZRecog.h"
#include "SplitChar.h"

#include "jhwr_europe.h"

namespace handwriting
{

static QString getDicPathByLocal(const QLocale & locale)
{
#ifdef BUILD_FOR_ARM
    QDir dir(SHARE_ROOT);
#else
    QDir dir(QDir::home());
#endif
    dir.cd("handwriting");
    {
        QString path = dir.filePath("europe.dat");
        if (QFile::exists(path))
        {
            return path;
        }
    }
    return QString();
}

jHWREuropean::jHWREuropean()
{
}

jHWREuropean::~jHWREuropean()
{
    deinitialize();
}

void jHWREuropean::updateLanguage(const QLocale & locale)
{
    if (address_ == 0)
    {
        return;
    }

    HWRLanguage lang = HWR_LANGUAGE_ENGLISH;
    switch (locale.language())
    {
    case QLocale::Byelorussian:
        lang = HWR_LANGUAGE_BELORUSSIAN;
        break;
    case QLocale::Bulgarian:
        lang = HWR_LANGUAGE_BULGARIAN;
        break;
    case QLocale::Danish:
        lang = HWR_LANGUAGE_DENISH;
        break;
    case QLocale::German:
        lang = HWR_LANGUAGE_GERMAN;
        break;
    case QLocale::Greek:
        lang = HWR_LANGUAGE_GREEK;
        break;
    case QLocale::Spanish:
        lang = HWR_LANGUAGE_SPANISH;
        break;
    case QLocale::Finnish:
        lang = HWR_LANGUAGE_FINNISH;
        break;
    case QLocale::French:
        lang = HWR_LANGUAGE_FRENCH;
        break;
    case QLocale::Hungarian:
        lang = HWR_LANGUAGE_HUNGARIAN;
        break;
    case QLocale::Italian:
        lang = HWR_LANGUAGE_ITALIAN;
        break;
    case QLocale::Dutch:
        lang = HWR_LANGUAGE_DUTCH;
        break;
    case QLocale::Polish:
        lang = HWR_LANGUAGE_POLISH;
        break;
    case QLocale::Portuguese:
        lang = HWR_LANGUAGE_PORTUGUESE;
        break;
    case QLocale::Russian:
        lang = HWR_LANGUAGE_RUSSIAN;
        break;
    case QLocale::Swedish:
        lang = HWR_LANGUAGE_SWEDISH;
        break;
    default:
        break;
    }
#ifdef BUILD_FOR_ARM
    HZSetParam_Multi(PARAM_LANGUAGE, lang);
#endif
}

bool jHWREuropean::initialize(const QLocale & locale)
{
    QString path = getDicPathByLocal(locale);
    qDebug("Open data file:%s", qPrintable(path));
    if (path.isEmpty())
    {
        return false;
    }

    FILE *fp = fopen(path.toUtf8().constData(), "rb");
    if (fp == NULL)
    {
        return false;
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);

    address_ = (unsigned char *)malloc(size);
    if(address_ == NULL)
    {
        printf("malloc failed!");
        return false;
    }

    fseek(fp, 0, SEEK_SET);
    fread(address_, 1, size, fp);
    fclose(fp);
#ifdef BUILD_FOR_ARM
    if (HZInitCharacterRecognition_Multi(address_) != 0)
    {
        updateLanguage(locale);
        HZSetParam_Multi(PARAM_RECORANG, RECOG_RANGE_SYMBOL | RECOG_RANGE_GESTURE);
        return true;
    }
#endif
    return false;
}

bool jHWREuropean::deinitialize()
{
    if (address_ == 0)
    {
        return false;
    }

#ifdef BUILD_FOR_ARM
    HZExitCharacterRecognition_Multi();
    if (address_ != 0)
    {
        free(address_);
        address_ = 0;
    }
#endif
    return true;
}

bool jHWREuropean::setWorkArea(const QRect & rect)
{
    return false;
}

bool jHWREuropean::setCandidateNum(const int number)
{
    if (address_ == 0)
    {
        return false;
    }

#ifdef BUILD_FOR_ARM
    // set candidate number
    HZSetParam_Multi(PARAM_CANDNUMB, number);
    candidates_num_ = number;
#endif
    return true;
}

bool jHWREuropean::setLocale(const QLocale & locale)
{
    if (address_ == 0)
    {
        return false;
    }

#ifdef BUILD_FOR_ARM
    updateLanguage(locale);
    HZSetParam_Multi(PARAM_RECORANG, RECOG_RANGE_SYMBOL | RECOG_RANGE_GESTURE);
#endif
    return true;
}

bool jHWREuropean::setSpecialRecognizeRange(SpecialRecognizeRange range)
{
#ifdef BUILD_FOR_ARM
    switch (range)
    {
    case NUMBER:
        HZSetParam_Multi(PARAM_RECORANG, RECOG_RANGE_NUMBER);
        break;
    case INTERPUNCTION:
        HZSetParam_Multi(PARAM_RECORANG, RECOG_RANGE_INTERPUNCTION);
        break;
    default:
        break;
    }
#endif
    return true;
}

bool jHWREuropean::recognize(QStringList & result)
{
    if (address_ == 0)
    {
        return false;
    }

    result.clear();
    if (raw_points_.empty())
    {
        return false;
    }

    HZ_INT16 * raw_points = new HZ_INT16[raw_points_.size()];
    memset(raw_points, 0, raw_points_.size());
    for (unsigned int i = 0; i < raw_points_.size(); ++i)
    {
        raw_points[i] = raw_points_[i];
    }

    //recognizeSingleChar(raw_points, result);
    recognizeMultipleChars(raw_points, raw_points_.size(), result);

    delete []raw_points;
    return true;
}

bool jHWREuropean::recognizeMultipleChars(HZ_INT16 * points, long length, QStringList & result)
{
    result.clear();
    HZ_WORD * code_array = new HZ_WORD[candidates_num_ + 20];
    code_array[candidates_num_] = 0;

#ifdef BUILD_FOR_ARM
    short split_points[1024 * 2] = {0};
    jtSplitCharBox split_char_boxes[200] = {0};
    memset(split_char_boxes, 0, sizeof(jtSplitCharBox) * 200);

    int split_size = jtSplitChar_SplitPointArray(points, length/2, (jtSplitCharBox *)split_char_boxes, 200);
    qDebug("Split point array successfully:%d", split_size);

    for(int i = 0; i < split_size; i++)
    {
        memset(code_array, 0, sizeof(code_array));
        memset(split_points, 0, sizeof(split_points));

        int start = split_char_boxes[i].nPos * 2;
        int end = (i == split_size - 1) ? length : (split_char_boxes[i + 1].nPos) * 2;

        getSplitPoint(points, start, (end - start), split_points, 1024 * 2);
        int num_result = HZCharacterRecognize_Multi(split_points, (unsigned short*)code_array);
        qDebug("Recognize Done:%d", num_result);

        for (int idx_result = 0; idx_result < num_result; idx_result++)
        {
            if (idx_result < result.size())
            {
                result[idx_result].push_back(QChar(code_array[idx_result]));
            }
            else
            {
                result.push_back(QChar(code_array[idx_result]));
            }
        }
    }
#else
    result.push_back(QString(QChar(0x82A6)));
    result.push_back(QString(QChar(0x82A7)));
    result.push_back(QString(QChar(0x82A8)));
    result.push_back(QString(QChar(0x82A9)));
    result.push_back(QString(QChar(0x82AA)));
    result.push_back(QString(QChar(0x82AB)));
    result.push_back(QString(QChar(0x82AC)));
    result.push_back(QString(QChar(0x82AD)));
    result.push_back(QString(QChar(0x82AE)));
    result.push_back(QString(QChar(0x82AF)));
#endif
    delete []code_array;
    return true;
}

bool jHWREuropean::recognizeSingleChar(HZ_INT16 * points, QStringList & result)
{
    result.clear();
    HZ_WORD * code_array = new HZ_WORD[candidates_num_ + 20];
    code_array[candidates_num_] = 0;

#ifdef BUILD_FOR_ARM
    int reg_num = HZCharacterRecognize_Multi(points, code_array);
    if (reg_num <= 0)
    {
        qWarning("Could not recognize the data.");
        return false;
    }

    for (int i = 0; i < reg_num; i++)
    {
        qDebug("Recognized: %d", code_array[i]);
        result.push_back(QChar(code_array[i]));
    }
#else
    result.push_back(QString(QChar(0x82A6)));
    result.push_back(QString(QChar(0x82A7)));
    result.push_back(QString(QChar(0x82A8)));
    result.push_back(QString(QChar(0x82A9)));
    result.push_back(QString(QChar(0x82AA)));
    result.push_back(QString(QChar(0x82AB)));
    result.push_back(QString(QChar(0x82AC)));
    result.push_back(QString(QChar(0x82AD)));
    result.push_back(QString(QChar(0x82AE)));
    result.push_back(QString(QChar(0x82AF)));
#endif
    delete []code_array;
    return true;
}

}
