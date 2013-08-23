#include "my_widget.h"

static const int INTERVAL = 4;
static int count = 0;

MyWidget::MyWidget()
    : QWidget(0, Qt::FramelessWindowHint)
{
    QTimer::singleShot(INTERVAL * 1000, this, SLOT(refresh()));
}

void MyWidget::paintEvent(QPaintEvent *e)
{
    ++count;
    QPainter p(this);
    if (count % 5 == 0)
    {
        QFont f;
        f.setPointSize(70);
        p.setFont(f);

        QString t("Count %1\nSeconds %2\nBattery %3");
        t = t.arg(count).arg(count * INTERVAL).arg(voltage());
        p.drawText(rect(), Qt::AlignCenter, t);
    }
    else
    {
        p.fillRect(rect(), QBrush(Qt::white));
    }
}

void MyWidget::refresh()
{
    update();
    onyx::screen::instance().flush(this);

    QTimer::singleShot(INTERVAL * 1000, this, SLOT(refresh()));
}

int MyWidget::voltage()
{
    const int BUFFER_SIZE = 1024;
    const int VOLTAGE_MAX = 4200;
    int voltage = VOLTAGE_MAX;
    FILE * fd = fopen("/proc/driver/bq27510", "r");
    if (fd == 0)
    {
        return voltage;
    }

    // Query all data:
    // cat /proc/driver/bq27510
    // Flags: 0x000064b4
    // Voltage: 4218mV
    // Current: -8mA
    // Relative State of Charge: 100%
    // Remaining capacity: 11520mAh
    // Time to Empty: 65535min
    // Time to Full: 0min
    fseek(fd, 0, SEEK_SET);

    while (!feof(fd))
    {
        char buf[BUFFER_SIZE + 1] = {0};
        char ignore[20] = {0};

        fgets(buf, BUFFER_SIZE, fd);
        if (strstr(&buf[0], "Voltage") != 0)
        {
            sscanf(buf, "%s %d", ignore, &voltage);
            if (voltage <= 0 || voltage >= VOLTAGE_MAX)
            {
                voltage = VOLTAGE_MAX;
            }
            break;
        }
    }

    fclose(fd);
    return voltage;
}

