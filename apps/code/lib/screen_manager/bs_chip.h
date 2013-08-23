/// bs_chip.h onyx internation inc.

#ifndef __BS_CHIP_FREESCALE_H__
#define __BS_CHIP_FREESCALE_H__



#include <cassert>


enum bs_ifm { BS_IFM_REG = 0, BS_IFM_CMD };

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
static const int VAL_HDC       = (1 << GPIO_HDC);
static const int VAL_CS_WE_0   = (~(1 << GPIO_HCS_L)) & (~(1 << GPIO_HWE_L));
static const int VAL_CS_WE_1   = (1 << GPIO_HCS_L) | (1 << GPIO_HWE_L);
static const unsigned int VAL_HDR = (1 << GPIO_HRDY);
static const int WAIT_MAX = 100000;

// Define the waveform type. See Chapter 16  Waveform Modes
static const int WAVEFORM_2_MODE = 1;       // 2 gray level.
static const int WAVEFORM_16_MODE = 2;      // 16 gray level.
static const int WAVEFORM_8_MODE  = 3;      // 8 gray level.

// Define pixel format.
static const int DEFAULT_PIXEL_FORMAT = 3;  // 1 byte per pixel.

#ifdef ENABLE_EINK_SCREEN
#define force_inline __attribute__((always_inline))


/// Reimplement the boardsheet chip for freescale.
/// For application that wants to access GPIO resources
/// they can take it as a reference.
class bs_chip
{
public:
    bs_chip( void );
    ~bs_chip( void );

public:
    void init_controller( void );
    void test_gpio( void );
    void reset( void );
    void sleep( int ms = 500 );

public:
    void ifmode( bs_ifm m );
    void command( int v );
    int  data( void );
    int  wait_for_ready( void );
    int  wait_until_ready( void );

    void wr_reg( int ra, int rd );
    int  rd_reg( int ra );

    inline void set_gpio_dir( int pin, int val ) force_inline;
    inline int  get_gpio_dir( int pin ) force_inline;
    inline int  get_gpio_val( int pin ) force_inline { return (((*GPIO_DR) >> pin) & 0x1); }
    inline void set_gpio_val( int pin, int val ) force_inline { *GPIO_DR = ((*GPIO_DR) & ~(1<<pin)) | (val<<pin); }

    inline void gpio_reset_l( int v )   force_inline { set_gpio_val( GPIO_RESET_L, v ); }
    inline void gpio_hdc( int v )       force_inline { set_gpio_val( GPIO_HDC, v ); }
    inline void gpio_hrd_l( int v )     force_inline { set_gpio_val( GPIO_HRD_L, v ); }
    inline void gpio_hwe_l( int v )     force_inline { set_gpio_val( GPIO_HWE_L, v ); }
    inline void gpio_hcs_l( int v )     force_inline { set_gpio_val( GPIO_HCS_L, v ); }
    inline int  gpio_hrdy( void )       force_inline { return get_gpio_val( GPIO_HRDY ); }
    inline int  gpio_hirq( void )       force_inline { return get_gpio_val( GPIO_HIRQ ); }

    inline void gpio_hdb_dir( int val )   force_inline
    {
        if ( val & 0x1 )
        {
            *GPIO_GDIR |= 0x0000ffff;
            hdb_dir_ = 1;
        }
        else
        {
            *GPIO_GDIR &= 0xffff0000;
            hdb_dir_ = 0;
        }

        // Clear all 0~15 bits.
        *GPIO_IMR &= 0xffff0000;
    }

    inline int  gpio_hdb( void )        force_inline
    {
        if ( hdb_dir_ )
        {
            gpio_hdb_dir( 0 );
        }

        // As the hdb uses 0~15 pins, so we can use 0xffff directly.
        // We are told we need to read it twice. Not sure.
        volatile int val = (*GPIO_DR & 0xffff);
        val = (*GPIO_DR & 0xffff);
        return val;
    }

    /// Caller should set the gpio_hdb direction.
    inline void data( int value ) force_inline
    {
        wait_for_ready( );
        gpio_hdc( 1 );
        gpio_hcs_l( 0 );
        gpio_hwe_l( 0 );
        gpio_hdb( value );
        gpio_hwe_l( 1 );
        gpio_hcs_l( 1 );
    }

    inline void fast_write_data(int value) force_inline
    {
        int ready = ((*GPIO_DR) & VAL_HDR);
        int count = 0;
        while ( ready == 0 && count++ < WAIT_MAX)
        {
            ready = ((*GPIO_DR) & VAL_HDR);
        }

        // hdc 1
        *GPIO_DR |= VAL_HDC;

        // hcs_l 0 / hwe_l 0
        *GPIO_DR &= VAL_CS_WE_0;

        // data
        *GPIO_DR = (*GPIO_DR & 0xffff0000) | (value & 0x0000ffff);

        // hcs_l 1 / hwe_l 1
        *GPIO_DR |= VAL_CS_WE_1;
    }

    /// This function returns the current values of the HDB[15:0] pins.
    /// Basically, we can call set_gpio_val for every pin which is slow.
    /// We can optimize it by accessing the address directly. TODO.
    inline void gpio_hdb( int val )       force_inline
    {
        // As the hdb uses 0~15 pins, so we can use 0xffff directly
        if ( !hdb_dir_ )
        {
            gpio_hdb_dir( 1 );
        }

        *GPIO_DR = (*GPIO_DR & 0xffff0000) | (val & 0x0000ffff);
    }

    volatile unsigned int * data_addr() { return GPIO_DR; }

private:
    bool open_gpio( void );
    void close_gpio( void );
    void set_gpio_mode( int pin );
    int  get_gpio_mode( int pin );

    void dump();

private:
    int fd_;                        ///< The memory device.
    int hdb_dir_;                   ///< Data direction.

    static volatile unsigned int * GPIO_GDIR;
    static volatile unsigned int * GPIO_DR;
    static volatile unsigned int * GPIO_IMR;
    static volatile unsigned int * GPIO_PSR;
    static unsigned int * map_;


};

#endif // ENABLE_EINK_SCREEN

#endif // __BS_CHIP_FREESCALE_H__

// end of file
