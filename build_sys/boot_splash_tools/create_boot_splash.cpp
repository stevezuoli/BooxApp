#include <QtCore/QtCore>
#include <QtGui/QtGui>

int main(int argc, char* argv[])
{
    if (argc != 5)
    {
        qDebug("Usage: %s <input image> <target width> <target height> <output file>", argv[0]);
        return -1;
    }

    QString path = QString::fromLocal8Bit(argv[1]);
    QImage img(path);
    if (img.isNull())
    {
        qDebug("Null image!");
        return -2;
    }

    int screen_width = QString(argv[2]).toInt();
    int screen_height = QString(argv[3]).toInt();

    int img_width = img.width();
    int img_height = img.height();

    if (img_width != screen_width || img_height != screen_height)
    {
        img = img.scaled(screen_width, screen_height);
    }

    char* pgm_data = new char[screen_width * screen_height];
    for (int i = 0; i < screen_height; i++)
    {
        for (int j = 0; j < screen_width; j++)
        {
            pgm_data[i * screen_width + j] = qGray(img.pixel(j, i));
        }
    }

    QFile pgm(argv[4]);
    if (!pgm.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        qDebug("Can't open %s for writing!", argv[4]);
        return -4;
    }

    pgm.write(pgm_data, screen_width * screen_height);
    pgm.close();
    delete[] pgm_data;
    return 0;
}
