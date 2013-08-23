#include <cstdio>
#include "gpio/marvell_gpio.h"

int main(int argc, char * argv[])
{
    if (argc != 3)
    {
        printf("Usage: value group pin\n");
        return -1;
    }

    int group = 0;
    int pin = 0;
    sscanf(argv[1], "%d", &group);
    sscanf(argv[2], "%d", &pin);
    MarvellGpio gpio;
    int value = gpio.value(group, pin);
    printf("\nGPIO group %d pin %d value %d\n", group, pin, value);
    return value;
}
