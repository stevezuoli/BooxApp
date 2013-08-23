
#include <cstring>
#include <cassert>
#include <cstdio>
#include "screen_manager/bs_chip_freescale.h"

/*
static const int GPIO_CNF1       = 23;             // IPU_LCS0         MCU3_23
static const int GPIO_HRDY       = 31;             // ATA_RESET_B      MCU3_31
static const int GPIO_HDC        = 27;             // ATA_CS1          MCU3_27
static const int GPIO_RESET_L    = 24;             // LCS1             MCU3_24
static const int GPIO_HRD_L      = 29;             // ATA_DIOW         MCU3_29
static const int GPIO_HWE_L      = 30;             // ATA_DMACK        MCU3_30
static const int GPIO_HCS_L      = 28;             // ATA_DIOR         MCU3_28
static const int GPIO_HIRQ       = 26;             // ATA_CS0          MCU3_26
static const int GPIO_HDB0       = 0;              // GPIO3_0          MCU3_0
static const int GPIO_HDB1       = 1;              // GPIO3_1          MCU3_1
static const int GPIO_HDB2       = 2;              // SCLK0            MCU3_2
static const int GPIO_HDB3       = 3;              // SRST0            MCU3_3
static const int GPIO_HDB4       = 4;              // CSI_D4           MCU3_4
static const int GPIO_HDB5       = 5;              // CSI_D5           MCU3_5
static const int GPIO_HDB6       = 6;              // CSI_D6           MCU3_6
static const int GPIO_HDB7       = 7;              // CSI_D7           MCU3_7
static const int GPIO_HDB8       = 8;              // CSI_D8           MCU3_8
static const int GPIO_HDB9       = 9;              // CSI_D9           MCU3_9
static const int GPIO_HDB10      = 10;             // CSI_D10          MCU3_10
static const int GPIO_HDB11      = 11;             // CSI_D11          MCU3_11
static const int GPIO_HDB12      = 12;             // CSI_D12          MCU3_12
static const int GPIO_HDB13      = 13;             // CSI_D13          MCU3_13
static const int GPIO_HDB14      = 14;             // CSI_D14          MCU3_14
static const int GPIO_HDB15      = 15;             // CSI_D15          MCU3_15
*/
static const char * PIN_NAME[]=
{
    "HDB0",
    "HDB1",
    "HDB2",
    "HDB3",
    "HDB4",
    "HDB5",
    "HDB6",
    "HDB7",
    "HDB8",
    "HDB9",
    "HDB10",
    "HDB11",
    "HDB12",
    "HDB13",
    "HDB14",
    "HDB15",
    "",         /// 16
    "",         /// 17
    "",         /// 18
    "",         /// 19
    "",         /// 20
    "",         /// 21
    "",         /// 22
    "CNF1",     /// 23
    "RESET_L",  /// 24
    "",         /// 25
    "HIRQ",     /// 26
    "HDC",      /// 27
    "HCS_L",    /// 28
    "HRD_L",    /// 29
    "HWE_L",    /// 30
    "HRDY",     /// 31
};
static const int SIZE = sizeof(PIN_NAME)/ sizeof(PIN_NAME[0]);

int get_pin(const char *name)
{
    for(int i = 0; i < SIZE; ++i)
    {
        if (strcmp(PIN_NAME[i], name) == 0)
        {
            return i;
        }
    }
    assert(false);
    return -1;
}

void list_all(bs_chip & a)
{
    printf("data register\t%0X\n", a.data_reg());
    printf("dir  register\t%0X\n", a.dir_reg());
    printf("psr  register\t%0X\n", a.psr_reg());
}

void get_value(bs_chip &bs, const char *pin)
{
    int index = get_pin(pin);
    bs.set_gpio_dir(index, 0);
    int value = bs.get_gpio_val(index);
    printf("Pin %s value %X\n", pin, value);
    list_all(bs);
}

void set_value(bs_chip &bs, const char *pin, const char *value)
{
    printf("set pin %s with value %s\n", pin, value);
    int index = get_pin(pin);
    int v = 0;
    sscanf(value, "%d", &v);
    bs.set_gpio_dir(index, 1);
    bs.set_gpio_val(index, v);
    list_all(bs);
}

int main(int argc, char *argv[])
{
    bs_chip bs;
    bs.init_controller();

    // Just list all registers.
    if (argc <= 1)
    {
        list_all(bs);
        return 0;
    }
    else if (argc == 2)
    {
        get_value(bs, argv[1]);
        return 0;
    }
    else if (argc == 3)
    {
        set_value(bs, argv[1], argv[2]);
        return 0;
    }
    return 0;
}

