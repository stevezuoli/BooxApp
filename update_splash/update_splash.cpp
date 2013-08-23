#include <QtCore/QtCore>
#include <QtGui/QtGui>

static const char* tmp_pgm = "/tmp/splash.dat";

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        qDebug("Usage: %s <PNG>", argv[0]);
        return -1;
    }

    QString path = QString::fromLocal8Bit(argv[1]);
    QImage img(path);
    if (img.isNull())
    {
        qDebug("Null image!");
        return -2;
    }

    int screen_width = QString(getenv("SCREEN_WIDTH")).toInt();
    int screen_height = QString(getenv("SCREEN_HEIGHT")).toInt();

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

    QFile pgm(tmp_pgm);
    if (!pgm.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        qDebug("Can't open %s for writing!", tmp_pgm);
        return -4;
    }

    pgm.write(pgm_data, screen_width * screen_height);
    pgm.close();
    delete[] pgm_data;

    // The pgm data is written to /tmp/splash.dat
    system("flash_erase /dev/mtd1 0 4");
    system("cat /tmp/splash.dat | nandwrite -p /dev/mtd1");
    return 0;
}
