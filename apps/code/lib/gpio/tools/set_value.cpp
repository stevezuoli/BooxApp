#include <cstdio>
#include "gpio/gpio.h"

int main(int argc, char * argv[])
{
    if (argc != 4)
    {
        printf("Usage: value group pin 0/1\n");
        return -1;
    }

    int group = 0;
    int pin = 0;
    int v = 0;
    sscanf(argv[1], "%d", &group);
    sscanf(argv[2], "%d", &pin);
    sscanf(argv[3], "%d", &v);
    Gpio gpio;
    gpio.setValue(group, pin, v);
    return 0;
}
