#include <cstdio>
#include "gpio/gpio.h"

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
    Gpio gpio;
    int value = gpio.value(group, pin);
    printf("%d\n", value);
    return value;
}
