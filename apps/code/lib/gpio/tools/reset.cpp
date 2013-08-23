#include <cstdio>
#include <unistd.h>
#include "gpio/gpio.h"

int main(int argc, char * argv[])
{
    if (argc != 3)
    {
        printf("Usage: reset group pin\n");
        return -1;
    }

    int group = 0;
    int pin = 0;
    sscanf(argv[1], "%d", &group);
    sscanf(argv[2], "%d", &pin);
    Gpio gpio;
    gpio.setValue(group, pin, 0);
    usleep(500 * 1000);
    gpio.setValue(group, pin, 1);
    printf("\nReset group %d pin %d\n", group, pin);
    return 0;
}
