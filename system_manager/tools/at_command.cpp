
#include "3G_reporter.h"

int main(int argc, char * argv[])
{
    if (argc < 2)
    {
        printf("Usage at_command <command> ...\n");
        return -1;
    }

    QApplication app(argc, argv);

    ModemMap mm;
    SerialPort sp(mm.signalChannel().toAscii());
    Reporter * reporter = 0;
    if (mm.provider() == "ZTE")
    {
        reporter = new ZTEReport(mm);
    }
    else
    {
        reporter = new HuaWeiReport(mm);
    }
    QByteArray data;
    data.append(argv[1]);
    if (!data.endsWith('\r'))
    {
        data.append('\r');
    }
    reporter->sendCommand(sp, data, 5000);
    qDebug("Result:");
    qDebug() << data;

    return 0;
}
