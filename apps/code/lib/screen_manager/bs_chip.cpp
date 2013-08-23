// $Id: bs_chip.cpp,v 1.5 2008/06/03 23:27:03 maprea Exp $

#ifdef ENABLE_EINK_SCREEN

#include <cassert>
#include <cstdio>
#include <error.h>
#ifndef _WINDOWS
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#endif
#include "bs_chip.h"

// 0x53f00000 is the physical address.
static unsigned int* const PHY_GPIO_DR[3]      = { reinterpret_cast<unsigned int *>(0x53FCC000),
                                                   reinterpret_cast<unsigned int *>(0x53FD0000),
                                                   reinterpret_cast<unsigned int *>(0x53FA4000) };
static unsigned int* const PHY_GPIO_GDIR[3]    = { reinterpret_cast<unsigned int *>(0x53FCC004),
                                                   reinterpret_cast<unsigned int *>(0x53FD0004),
                                                   reinterpret_cast<unsigned int *>(0x53FA4004) };
static unsigned int* const PHY_GPIO_IMR[3]     = { reinterpret_cast<unsigned int *>(0x53FCC014),
                                                   reinterpret_cast<unsigned int *>(0x53FD0014),
                                                   reinterpret_cast<unsigned int *>(0x53FA4014) };

static unsigned int* const PHY_GPIO_PSR[3]     = { reinterpret_cast<unsigned int *>(0x53FCC008),
                                                   reinterpret_cast<unsigned int *>(0x53FD0008),
                                                   reinterpret_cast<unsigned int *>(0x53FA4008) };

static const size_t MAP_SIZE = 4096;
static const size_t MAP_MASK = MAP_SIZE - 1;
static const int    MUX_CTL_BIT_LEN = 8;

volatile unsigned int * bs_chip::GPIO_GDIR;
volatile unsigned int * bs_chip::GPIO_DR;
volatile unsigned int * bs_chip::GPIO_IMR;
volatile unsigned int * bs_chip::GPIO_PSR;
unsigned int * bs_chip::map_;

bs_chip::bs_chip( void )
{
    open_gpio();
}

bs_chip::~bs_chip( void )
{
    close_gpio();
}

bool bs_chip::open_gpio( void )
{
    // Already opened.
    if (GPIO_GDIR)
    {
        return true;
    }

    static const int MAP_PROT = PROT_READ | PROT_WRITE;
    fd_ = open( "/dev/mem", O_RDWR | O_SYNC );
    assert( fd_ >= 0 );

    // Init address one by one. Not sure which one to use.
    size_t pgoff = 0;
    unsigned int *p = 0;
    int i = GPIO_GROUP;
    {
        // DR
        pgoff = reinterpret_cast<int>(PHY_GPIO_DR[i]) &  ~MAP_MASK;
        p = reinterpret_cast<unsigned int *>(mmap( 0, MAP_SIZE, MAP_PROT, MAP_SHARED, fd_, pgoff ));
        assert ( p != reinterpret_cast<void *>(-1) );
        GPIO_DR = p + ((reinterpret_cast<int>(PHY_GPIO_DR[i]) & MAP_MASK) / 4);

        // GDIR
        GPIO_GDIR = p + ((reinterpret_cast<int>(PHY_GPIO_GDIR[i]) & MAP_MASK) / 4);

        // IMR
        GPIO_IMR = p + ((reinterpret_cast<int>(PHY_GPIO_IMR[i]) & MAP_MASK) / 4);

        // PSR
        GPIO_PSR =  p + ((reinterpret_cast<int>(PHY_GPIO_PSR[i]) & MAP_MASK) / 4);
        map_ = p;
    }

    // dump();
    return true;
}

void bs_chip::dump()
{
    printf("GPIO_DR\t%p\n", GPIO_DR);
    printf("GPIO_GDIR\t%p\n", GPIO_GDIR);
    printf("GPIO_IMR\t%p\n", GPIO_IMR);
    printf("GPIO_PSR\t%p\n", GPIO_PSR);
}

void bs_chip::close_gpio( void )
{
    assert( munmap( map_, MAP_SIZE ) == 0 );
    close(fd_);
    fd_ = 0;
}

void bs_chip::reset( void )
{
    set_gpio_val( GPIO_RESET_L, 0 );
    sleep(5);       ///< According to the spec P33.
    set_gpio_val( GPIO_RESET_L, 1 );
}

void bs_chip::sleep( int ms )
{
    // TODO: Need the value from datasheet.
    usleep(1000 * ms);
}

/// Seems we don't need this function.
void bs_chip::set_gpio_mode( int pin )
{
}

/// Seems we don't need this function.
int bs_chip::get_gpio_mode( int pin )
{
    return 0;
}

/// This function sets the I/O direction of the pin pin to the value.
/// 0 for input and 1 for output.
void bs_chip::set_gpio_dir( int gpio_index, int val )
{
    if( val & 0x1)
    {
        *GPIO_GDIR |= (1<<gpio_index);
    }
    else
    {
        *GPIO_GDIR &= ~(1<<gpio_index);
    }

    *GPIO_IMR &= ~(1<<gpio_index);
}

/// Need to confirm.
int bs_chip::get_gpio_dir( int pin )
{
    return ((*GPIO_GDIR >> pin) & 0x1);
}

/// Initialize the bs chip controller
void bs_chip::init_controller( void )
{
    // We don't need to change them to gpio mode. We do it in the kernel.
    /*
    set_gpio_mode( GPIO_CNF1 );
    set_gpio_mode( GPIO_HRDY );
    set_gpio_mode( GPIO_HDC );
    set_gpio_mode( GPIO_RESET_L );
    set_gpio_mode( GPIO_HRD_L );
    set_gpio_mode( GPIO_HWE_L );
    set_gpio_mode( GPIO_HCS_L );
    set_gpio_mode( GPIO_HIRQ );
    for ( int i = 0; i < 16; i++ )
    {
      set_gpio_mode( GPIO_HDB0 + i );
    }

    assert( get_gpio_mode( GPIO_CNF1 ) == 1 );
    assert( get_gpio_mode( GPIO_HRDY ) == 1 );
    assert( get_gpio_mode( GPIO_HDC ) == 1 );
    assert( get_gpio_mode( GPIO_RESET_L ) == 1 );
    assert( get_gpio_mode( GPIO_HRD_L ) == 1 );
    assert( get_gpio_mode( GPIO_HWE_L ) == 1 );
    assert( get_gpio_mode( GPIO_HCS_L ) == 1 );
    assert( get_gpio_mode( GPIO_HIRQ ) == 1 );
    for ( int i = 0; i < 16; i++ )
    {
      assert( get_gpio_mode( GPIO_HDB0 + i ) == 1 );
    }
    */

    set_gpio_dir( GPIO_CNF1, 1 );
    set_gpio_dir( GPIO_HRDY, 0 );
    set_gpio_dir( GPIO_HDC, 1 );
    set_gpio_dir( GPIO_RESET_L, 1 );
    set_gpio_dir( GPIO_HRD_L, 1 );
    set_gpio_dir( GPIO_HWE_L, 1 );
    set_gpio_dir( GPIO_HCS_L, 1 );
    set_gpio_dir( GPIO_HIRQ, 0 );
    for ( int i = 0; i < 16; i++ )
    {
        set_gpio_dir( GPIO_HDB0 + i, 0 );
    }
    hdb_dir_ = 0;

    assert( get_gpio_dir( GPIO_CNF1 ) == 1 );
    assert( get_gpio_dir( GPIO_HRDY ) == 0 );
    assert( get_gpio_dir( GPIO_HDC ) == 1 );
    assert( get_gpio_dir( GPIO_RESET_L ) == 1 );
    assert( get_gpio_dir( GPIO_HRD_L ) == 1 );
    assert( get_gpio_dir( GPIO_HWE_L ) == 1 );
    assert( get_gpio_dir( GPIO_HCS_L ) == 1 );
    assert( get_gpio_dir( GPIO_HIRQ ) == 0 );
    for ( int i = 0; i < 16; i++ )
    {
        assert( get_gpio_dir( GPIO_HDB0 + i ) == 0 );
    }

    set_gpio_val( GPIO_RESET_L, 0 );
    set_gpio_val( GPIO_CNF1, 0 );
    set_gpio_val( GPIO_HDC, 0 );
    set_gpio_val( GPIO_HRD_L, 1 );
    set_gpio_val( GPIO_HWE_L, 1 );
    set_gpio_val( GPIO_HCS_L, 1 );
    set_gpio_val( GPIO_RESET_L, 1 );
}

/// Add our own test cases.
void bs_chip::test_gpio( void )
{
    wr_reg( 0x0304, 0x0123 ); // frame begin/end length reg
    wr_reg( 0x030A, 0x4567 ); // line begin/end length reg

    if ( rd_reg( 0x0304 ) == 0x0123 ) { printf("rd_reg( 0x0304 ) == 0x0123 failed\n"); }
    if ( rd_reg( 0x030A ) == 0x4567 ) { printf("rd_reg( 0x030A ) == 0x4567 failed\n"); }

    wr_reg( 0x0304, 0xFEDC );
    wr_reg( 0x030A, 0xBA98 );

    if ( rd_reg( 0x0304 ) == 0xFEDC ) { printf("rd_reg( 0x0304 ) == 0xFEDC failed\n"); }
    if ( rd_reg( 0x030A ) == 0xBA98 ) { printf("rd_reg( 0x030A ) == 0xBA98 failed\n"); }
}

void bs_chip::ifmode( bs_ifm m )
{
    if ( m == BS_IFM_REG )
    {
        set_gpio_val( GPIO_CNF1, 0 );
    }
    else
    {
        set_gpio_val( GPIO_CNF1, 1 );
    }

    int ready = 0;
    while (ready == 0)
    {
        reset();
        ready = wait_until_ready();
    }
}

void bs_chip::command( int v )
{
    wait_for_ready();
    gpio_hdc( 0 );
    gpio_hcs_l( 0 );
    gpio_hwe_l( 0 );
    gpio_hdb( v );
    gpio_hwe_l( 1 );
    gpio_hcs_l( 1 );
}

/// Read data from controller.
int bs_chip::data( void )
{
    wait_for_ready();
    gpio_hdc( 1 );
    gpio_hcs_l( 0 );
    gpio_hrd_l( 0 );
    int d = gpio_hdb( );
    gpio_hrd_l( 1 );
    gpio_hcs_l( 1 );
    return d;
}

void bs_chip::wr_reg( int ra, int rd )
{
    command( ra );
    data( rd );
}

int bs_chip::rd_reg( int ra )
{
    command( ra );
    return data(  );
}

int bs_chip::wait_for_ready()
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
        printf("not ready %d!\n", count);
    }
    return count;
}

/// This function may cause deadlock. Make sure you call this function
/// correctly. Otherwise use wait_for_ready instead. This function is introduced
/// to solve screen update problem. We may have a better way later.
int bs_chip::wait_until_ready( void )
{
    int d = gpio_hrdy( );
    int count = 0;
    while ( d == 0 && count < 4000 )
    {
        d = gpio_hrdy( );
        count++;
        usleep(500);
    }
    return count;
}

#endif  // #ifdef ENABLE_EINK_SCREEN
// end of file
