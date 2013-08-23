#include <unistd.h>
#include <assert.h>
#include <stdio.h>

#ifdef USER_SPACE
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#define DBG printf
#define USLEEP usleep
#else
#include <cyg/hal/hal_soc.h>
#include <cyg/infra/diag.h>
#define DBG diag_printf
#define USLEEP hal_delay_us
#endif

#include "bs_chip.h"

#ifdef USER_SPACE
// Global variables.
// Kernel space gpio address.
static unsigned int* const PHY_GPIO_DR   = (unsigned int *)0x53FA4000;
static unsigned int* const PHY_GPIO_GDIR = (unsigned int *)0x53FA4004;
static unsigned int* const PHY_GPIO_PSR  = (unsigned int *)0x53FA4008;
static unsigned int* const PHY_GPIO_IMR  = (unsigned int *)0x53FA4014;

// For memory map.
static const size_t MAP_SIZE = 4096;
static const size_t MAP_MASK = 4095;

// GPIO address used in program, needs to be mapped to user space addresses
// if running in user space.
static unsigned int* GPIO_DR   = 0;
static unsigned int* GPIO_GDIR = 0;
static unsigned int* GPIO_PSR  = 0;
static unsigned int* GPIO_IMR  = 0;

// File descriptor for /dev/mem.
static int mem_fd = -1;
#endif

static const int WAIT_MAX = 100000;
static int       hdb_dir = 0;

// Function implementations.
void bs_chip_init( void )
{
#ifdef USER_SPACE
    const int MAP_PROT = PROT_READ | PROT_WRITE;
    size_t pgoff    = 0;
    void *p = 0;

    // Check if already opened.
    if (GPIO_DR || GPIO_GDIR || GPIO_PSR || GPIO_IMR)
    {
        return;
    }

    mem_fd = open( "/dev/mem", O_RDWR | O_SYNC );
    assert( mem_fd >= 0 );

    // DR
    pgoff = (unsigned int)PHY_GPIO_DR & ~MAP_MASK;
    p = mmap( 0, MAP_SIZE, MAP_PROT, MAP_SHARED, mem_fd, pgoff );
    assert ( p != MAP_FAILED );
    GPIO_DR   = p + ((unsigned int)PHY_GPIO_DR & MAP_MASK);
    GPIO_GDIR = p + ((unsigned int)PHY_GPIO_GDIR & MAP_MASK);
    GPIO_PSR  = p + ((unsigned int)PHY_GPIO_PSR & MAP_MASK);
    GPIO_IMR  = p + ((unsigned int)PHY_GPIO_IMR & MAP_MASK);
#endif
}

void bs_chip_final( void )
{
#ifdef USER_SPACE
    assert( munmap( GPIO_DR,   MAP_SIZE ) == 0 );
    close(mem_fd);
    mem_fd = -1;
#endif
}

void init_gpio( void )
{
    int i = 0;
    set_gpio_dir( GPIO_CNF1, 1 );
    set_gpio_dir( GPIO_HRDY, 0 );
    set_gpio_dir( GPIO_HDC, 1 );
    set_gpio_dir( GPIO_RESET_L, 1 );
    set_gpio_dir( GPIO_HRD_L, 1 );
    set_gpio_dir( GPIO_HWE_L, 1 );
    set_gpio_dir( GPIO_HCS_L, 1 );
    set_gpio_dir( GPIO_HIRQ, 0 );
    for ( i = 0; i < 16; i++ ) set_gpio_dir( GPIO_HDB0 + i, 0 );
    hdb_dir = 0;

    assert( get_gpio_dir( GPIO_CNF1 ) == 1 );
    assert( get_gpio_dir( GPIO_HRDY ) == 0 );
    assert( get_gpio_dir( GPIO_HDC ) == 1 );
    assert( get_gpio_dir( GPIO_RESET_L ) == 1 );
    assert( get_gpio_dir( GPIO_HRD_L ) == 1 );
    assert( get_gpio_dir( GPIO_HWE_L ) == 1 );
    assert( get_gpio_dir( GPIO_HCS_L ) == 1 );
    assert( get_gpio_dir( GPIO_HIRQ ) == 0 );
    for ( i = 0; i < 16; i++ ) assert( get_gpio_dir( GPIO_HDB0 + i ) == 0 );

    set_gpio_val( GPIO_RESET_L, 0 );
    set_gpio_val( GPIO_CNF1, 0 );
    set_gpio_val( GPIO_HDC, 0 );
    set_gpio_val( GPIO_HRD_L, 1 );
    set_gpio_val( GPIO_HWE_L, 1 );
    set_gpio_val( GPIO_HCS_L, 1 );
    set_gpio_val( GPIO_RESET_L, 1 );
}

void test_gpio( void )
{
    wr_reg( 0x0304, 0x0123 ); // frame begin/end length reg
    wr_reg( 0x030A, 0x4567 ); // line begin/end length reg

    if ( rd_reg( 0x0304 ) == 0x0123 ) { DBG("rd_reg( 0x0304 ) == 0x0123 failed\n"); }
    if ( rd_reg( 0x030A ) == 0x4567 ) { DBG("rd_reg( 0x030A ) == 0x4567 failed\n"); }

    wr_reg( 0x0304, 0xFEDC );
    wr_reg( 0x030A, 0xBA98 );

    if ( rd_reg( 0x0304 ) == 0xFEDC ) { DBG("rd_reg( 0x0304 ) == 0xFEDC failed\n"); }
    if ( rd_reg( 0x030A ) == 0xBA98 ) { DBG("rd_reg( 0x030A ) == 0xBA98 failed\n"); }
}

void reset( void )
{
    set_gpio_val( GPIO_RESET_L, 0 );
    bs_sleep(5);
    set_gpio_val( GPIO_RESET_L, 1 );
}

void bs_sleep( int ms )
{
    // TODO: Need the value from datasheet.
    USLEEP(1000 * ms);
}

void ifmode( bs_ifm m )
{
    int ready = 0;

    if ( m == BS_IFM_REG )
    {
        set_gpio_val( GPIO_CNF1, 0 );
    }
    else
    {
        set_gpio_val( GPIO_CNF1, 1 );
    }

    while (ready == 0)
    {
        reset();
        ready = wait_until_ready();
    }
}

void command( int v )
{
    wait_for_ready();
    gpio_hdc( 0 );
    gpio_hcs_l( 0 );
    gpio_hwe_l( 0 );
    wr_gpio_hdb( v );
    gpio_hwe_l( 1 );
    gpio_hcs_l( 1 );
}

int  rd_data( void )
{
    int d;
    wait_for_ready();
    gpio_hdc( 1 );
    gpio_hcs_l( 0 );
    gpio_hrd_l( 0 );
    d = rd_gpio_hdb( );
    gpio_hrd_l( 1 );
    gpio_hcs_l( 1 );
    return d;
}

int  wait_for_ready( void )
{
    int d = gpio_hrdy( );
    int count = 0;
    while ( d == 0 && count < WAIT_MAX)
    {
        d = gpio_hrdy( );
        count++;
    }
    if (d == 0 || count >= WAIT_MAX)
    {
        DBG("not ready %d!\n", count);
    }
    return count;
}

/// This function may cause deadlock. Make sure you call this function
/// correctly. Otherwise use wait_for_ready instead. This function is introduced
/// to solve screen update problem. We may have a better way later.
int  wait_until_ready( void )
{
    // TODO, change the usleep to nanosleep later.
    static const int MAX = 40;
    int d = gpio_hrdy( );
    int count = 0;
    while ( d == 0 && count < MAX)
    {
        USLEEP(100 * 1000);
        d = gpio_hrdy( );
        count++;
    }
    if (d == 0)
    {
        DBG("Still not ready, should not happen");
    }
    return count;
}

void wr_reg( int ra, int rd )
{
    command( ra );
    wr_data( rd );
}

int  rd_reg( int ra )
{
    command( ra );
    return rd_data();
}

void set_gpio_dir( int pin, int val )
{
#ifdef USER_SPACE
    *GPIO_GDIR = ((*GPIO_GDIR) & ~(1 << pin)) | (val << pin);
#else
    unsigned int x = readl(GPIO3_BASE_ADDR + 0x04);
    writel((x & ~(1 << pin)) | (val << pin), GPIO3_BASE_ADDR + 0x04);
#endif
}

int  get_gpio_dir( int pin )
{
#ifdef USER_SPACE
    return ((*GPIO_GDIR) >> pin) & 0x1;
#else
    unsigned int x = readl(GPIO3_BASE_ADDR + 0x04);
    return (x >> pin) & 0x1;
#endif
}

void set_gpio_val( int pin, int val )
{
#ifdef USER_SPACE
    *GPIO_DR = ((*GPIO_DR) & ~(1 << pin)) | (val << pin);
#else
    unsigned int x = readl(GPIO3_BASE_ADDR);
    writel((x & ~(1 << pin)) | (val << pin), GPIO3_BASE_ADDR);
#endif
}

int  get_gpio_val( int pin )
{
#ifdef USER_SPACE
    return ((*GPIO_DR) >> pin) & 0x1;
#else
    unsigned int x = readl(GPIO3_BASE_ADDR);
    return (x >> pin) & 0x1;
#endif
}

void gpio_reset_l( int v )
{
    set_gpio_val( GPIO_RESET_L, v );
}

void gpio_hdc( int v )
{
    set_gpio_val( GPIO_HDC, v );
}

void gpio_hrd_l( int v )
{
    set_gpio_val( GPIO_HRD_L, v );
}

void gpio_hwe_l( int v )
{
    set_gpio_val( GPIO_HWE_L, v );
}

void gpio_hcs_l( int v )
{
    set_gpio_val( GPIO_HCS_L, v );
}

int  gpio_hrdy( void )
{
    return get_gpio_val( GPIO_HRDY );
}

int  gpio_hirq( void )
{
    return get_gpio_val( GPIO_HIRQ );
}

void gpio_hdb_dir( int val )
{
    if ( val & 0x1 )
    {
#ifdef USER_SPACE
        *GPIO_GDIR |= 0x0000ffff;
#else
        unsigned int x = readl(GPIO3_BASE_ADDR + 0x04);
        writel(x | 0x0000ffff, GPIO3_BASE_ADDR + 0x04);
#endif
        hdb_dir = 1;
    }
    else
    {
#ifdef USER_SPACE
        *GPIO_GDIR &= 0xffff0000;
#else
        unsigned int x = readl(GPIO3_BASE_ADDR + 0x04);
        writel(x & 0xffff0000, GPIO3_BASE_ADDR + 0x04);
#endif
        hdb_dir = 0;
    }
}

int  rd_gpio_hdb( void )
{
    if ( hdb_dir )
    {
        gpio_hdb_dir( 0 );
    }

    // As the hdb uses 0~15 pins, so we can use 0xffff directly.
    // We are told we need to read it twice. Not sure.
#ifdef USER_SPACE
    return (*GPIO_DR & 0xffff);
#else
    return readl(GPIO3_BASE_ADDR) & 0xffff;
#endif
}

void wr_data( int value )
{
#ifdef USER_SPACE
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
#else
    unsigned int x = readl(GPIO3_BASE_ADDR) & VAL_HDR;
    int count = 0;
    while ( x == 0 && count++ < WAIT_MAX)
    {
        x = readl(GPIO3_BASE_ADDR) & VAL_HDR;
    }

    // hdc 1
    x = readl(GPIO3_BASE_ADDR);
    writel( x | VAL_HDC, GPIO3_BASE_ADDR);

    // hcs_l 0 / hwe_l 0
    x = readl(GPIO3_BASE_ADDR);
    writel( x & VAL_CS_WE_0, GPIO3_BASE_ADDR);

    // data
    x = readl(GPIO3_BASE_ADDR);
    writel((x & 0xffff0000) | (value & 0x0000ffff), GPIO3_BASE_ADDR);

    // hcs_l 1 / hwe_l 1
    x = readl(GPIO3_BASE_ADDR);
    writel(x | VAL_CS_WE_1, GPIO3_BASE_ADDR);
#endif
}

/// This function returns the current values of the HDB[15:0] pins.
/// Basically, we can call set_gpio_val for every pin which is slow.
/// We can optimize it by accessing the address directly. TODO.
void wr_gpio_hdb( int val )
{
    // As the hdb uses 0~15 pins, so we can use 0xffff directly
    if ( !hdb_dir )
    {
        gpio_hdb_dir( 1 );
    }

#ifdef USER_SPACE
    *GPIO_DR = (*GPIO_DR & 0xffff0000) | (val & 0x0000ffff);
#else
    unsigned int x = readl(GPIO3_BASE_ADDR);
    writel((x & 0xffff0000) | (val & 0x0000ffff), GPIO3_BASE_ADDR);
#endif
}

#ifdef USER_SPACE
unsigned int * get_data_addr()
{
    return GPIO_DR;
}
#endif
