#define CPP_SOURCE
#include "Chinese_HZRecog.h"
#include "SplitChar.h"

#include "jhwr_chinese.h"

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
        QString path = dir.filePath("chinese.dat");
        if (QFile::exists(path))
        {
            return path;
        }
    }
    return QString();
}

jHWRChinese::jHWRChinese()
{
}

jHWRChinese::~jHWRChinese()
{
    deinitialize();
}

bool jHWRChinese::initialize(const QLocale & locale)
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
    if(address_ == 0)
    {
        printf("malloc failed!");
        return false;
    }

    fseek(fp, 0, SEEK_SET);
    fread(address_, 1, size, fp);
    fclose(fp);
#ifdef BUILD_FOR_ARM
    if (HZInitCharacterRecognition(address_) != 0)
    {
        HZSetParam(PARAM_RECORANG, RECOG_RANGE_GESTURE | RECOG_RANGE_GB1 | RECOG_RANGE_GB2 |
                   RECOG_RANGE_BIG51 | RECOG_RANGE_BIG52);
        return true;
    }
#endif
    return false;
}

bool jHWRChinese::deinitialize()
{
    if (address_ == 0)
    {
        return false;
    }

#ifdef BUILD_FOR_ARM
    HZExitCharacterRecognition();
    if (address_ != 0)
    {
        free(address_);
        address_ = 0;
    }
#endif
    return true;
}

bool jHWRChinese::setWorkArea(const QRect & rect)
{
    return false;
}

bool jHWRChinese::setCandidateNum(const int number)
{
    if (address_ == 0)
    {
        return false;
    }

#ifdef BUILD_FOR_ARM
    // set candidate number
    HZSetParam(PARAM_CANDNUMB, number);
    candidates_num_ = number;
#endif
    return true;
}

bool jHWRChinese::setLocale(const QLocale & locale)
{
    if (address_ == 0)
    {
        return false;
    }

#ifdef BUILD_FOR_ARM
    HZSetParam(PARAM_RECORANG, RECOG_RANGE_GESTURE | RECOG_RANGE_GB1 | RECOG_RANGE_GB2 |
               RECOG_RANGE_BIG51 | RECOG_RANGE_BIG52);
#endif
    return true;
}

bool jHWRChinese::setSpecialRecognizeRange(SpecialRecognizeRange range)
{
#ifdef BUILD_FOR_ARM
    switch (range)
    {
    case NUMBER:
        HZSetParam(PARAM_RECORANG, RECOG_RANGE_NUMBER);
        break;
    case INTERPUNCTION:
        HZSetParam(PARAM_RECORANG, RECOG_RANGE_INTERPUNCTION);
        break;
    default:
        break;
    }
#endif
    return true;
}

bool jHWRChinese::recognize(QStringList & result)
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

bool jHWRChinese::recognizeMultipleChars(HZ_INT16 * points, long length, QStringList & result)
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
        int num_result = HZCharacterRecognize(split_points, (unsigned short*)code_array);
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

bool jHWRChinese::recognizeSingleChar(HZ_INT16 * points, QStringList & result)
{
    result.clear();
    HZ_WORD * code_array = new HZ_WORD[candidates_num_ + 20];
    code_array[candidates_num_] = 0;

#ifdef BUILD_FOR_ARM
    int reg_num = HZCharacterRecognize(points, code_array);
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
