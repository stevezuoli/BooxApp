#ifndef __BS_CHIP_FREESCALE_H__
#define __BS_CHIP_FREESCALE_H__

// See the details of BOOK_MCIMX31.pdf Page 15 and datasheet of imx31.
// All these GPIO pins use the same group address 3
static const int GPIO_GROUP      = 2;              // Start from 0.
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

// Define calculated values used to improve performance.
static const unsigned int VAL_HDC     = 0x08000000;
static const unsigned int VAL_HDR     = 0x80000000;
static const unsigned int VAL_CS_WE_0 = 0xafffffff;
static const unsigned int VAL_CS_WE_1 = 0x50000000;

// Type definitions.
typedef enum
{
    BS_IFM_REG = 0,
    BS_IFM_CMD
} bs_ifm;

void bs_chip_init( void );
void bs_chip_final( void );
void init_gpio( void );
void test_gpio( void );
void reset( void );
void bs_sleep( int ms );

void ifmode( bs_ifm m );
void command( int v );
int  rd_data( void );
int  wait_for_ready( void );
int  wait_until_ready( void );

void wr_reg( int ra, int rd );
int  rd_reg( int ra );

void set_gpio_dir( int pin, int val );
int  get_gpio_dir( int pin );
int  get_gpio_val( int pin );
void set_gpio_val( int pin, int val );

void gpio_reset_l( int v );
void gpio_hdc( int v );
void gpio_hrd_l( int v );
void gpio_hwe_l( int v );
void gpio_hcs_l( int v );
int  gpio_hrdy( void );
int  gpio_hirq( void );

void gpio_hdb_dir( int val );
int  rd_gpio_hdb( void );
void wr_data( int value );
void wr_gpio_hdb( int val );
#ifdef USER_SPACE
unsigned int * get_data_addr( void );
#endif

#endif // __BS_CHIP_FREESCALE_H__
