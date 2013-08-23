#include "3G_manager.h"

int main(int argc, char* argv[])
{
    Gpio gpio;
    ThreeGManager pppd(gpio);
    pppd.enableDevice(true);
    pppd.setOptions("*99#", "", "");
    pppd.stop();
}
