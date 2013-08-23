

#include <QApplication>
#include <QMessageBox>
#include <QTimer>
#include "onyx/ui/languages.h"
#include "calibration.h"

/*
const int good[] = {-2287, 1,  59751,    5, 2280, -27771556, -34758};
const int bad[] =  {-2339, 32, -819486, -7, 2383, -28960112, -37156};

int new_x(int x, int y, const int *a)
{
    return (a[2] + a[0] * x + a[1] * y ) / a[6];
}

int new_y(int x, int y, const int *a)
{
    return (a[5] + a[3] * x + a[4] * y ) / a[6];
}

void test()
{
    const int org_x = 12;
    const int org_y = 608;

    int xx = new_x(org_x, org_y, &good[0]);
    int yy = new_y(org_x, org_y, &good[0]);

    int kk_xx = new_x(org_x, org_y, &bad[0]);
    int kk_yy = new_y(org_x, org_y, &bad[0]);
}
*/

int main(int argc, char **argv)
{
    // Always running as gui client.
    // The system manager should be launched anyway.
    QApplication app(argc, argv);

    Q_INIT_RESOURCE(onyx_ui_images);
    ui::loadTranslator(QLocale::system().name());

    Calibration cal;
    cal.exec();

    return 0;
}


