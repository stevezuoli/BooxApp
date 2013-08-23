
#include "3G_reporter.h"

int main(int argc, char * argv[])
{
    if (argc < 2)
    {
        printf("Usage check_3g retry ...\n");
        return -1;
    }

    QApplication app(argc, argv);

    ModemMap mm;
    Reporter * reporter = 0;
    if (mm.provider() == "ZTE")
    {
        reporter = new ZTEReport(mm);
    }
    else
    {
        reporter = new HuaWeiReport(mm);
    }

    int count = QByteArray(argv[1]).toInt();
    for(int i = 0; i < count; ++i)
    {
        printf("\nTrying %d\t:", i);
        int signal, total, network = 0;
        if (reporter->network(signal, total, network))
        {
            printf("Signal %d", signal);
            if (network != 0)
            {
                printf("Network type %d\n", network);
                return 0;
            }
        }
#ifndef _WINDOWS
        sleep(1);
#endif
        qDebug("Retry as network is limited");
    }

    qDebug("No service");
    return 1;
}
