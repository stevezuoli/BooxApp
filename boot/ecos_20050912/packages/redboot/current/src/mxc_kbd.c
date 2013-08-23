/*
 * Copyright 2004-2006 Freescale Semiconductor, Inc. All Rights Reserved.
 */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

/*!
 * @file mxc_keyb.c
 *
 * @brief Driver for the Freescale Semiconductor MXC keypad port.
 *
 * The keypad driver is designed as a standard Input driver which interacts
 * with low level keypad port hardware. Upon opening, the Keypad driver
 * initializes the keypad port. When the keypad interrupt happens the driver
 * calles keypad polling timer and scans the keypad matrix for key
 * press/release. If all key press/release happened it comes out of timer and
 * waits for key press interrupt. The scancode for key press and release events
 * are passed to Input subsytem.
 *
 * @ingroup keypad
 */

#include <string.h>
#include <cyg/hal/hal_soc.h>
#include <cyg/infra/diag.h>
#include <mxc_kbd.h>

#define ROWS 4
#define COLS 4

#define KPCR    (KPP_BASE_ADDR + 0x0)
#define KPSR    (KPP_BASE_ADDR + 0x2)
#define KDDR    (KPP_BASE_ADDR + 0x4)
#define KPDR    (KPP_BASE_ADDR + 0x6)
#define MXC_CCM_CGR1            (CCM_BASE_ADDR + 0x24)
#define MXC_CCM_CGR1_KPP     (0x3 << 20)

#define KBD_STAT_KPKD        0x01
#define KBD_STAT_KPKR        0x02
#define KBD_STAT_KDSC        0x04
#define KBD_STAT_KRSS        0x08
#define KBD_STAT_KDIE        0x100
#define KBD_STAT_KRIE        0x200
#define KBD_STAT_KPPEN       0x400
#define TEST_BIT(c, n) ((c) & (0x1 << (n)))
#define BITSET(c, n)   ((c) | (1 << (n)))

static void mxc_clks_enable(void)
{
    unsigned long reg;
    reg = readl(MXC_CCM_CGR1);
    reg |= MXC_CCM_CGR1_KPP;
    writel(reg, MXC_CCM_CGR1);
}

static void gpio_keypad_active(void)
{
    writel(0x12121212, 0x43FAC06C);
    writel(0x12121212, 0x43FAC064);
}

void mxc_kpp_init(void)
{
    unsigned int reg_val;

    /* Enable keypad clock */
    mxc_clks_enable();

    /* IOMUX configuration for keypad */
    gpio_keypad_active();

    /* Configure keypad */

    /* Enable number of rows in keypad (KPCR[7:0])
     * Configure keypad columns as open-drain (KPCR[15:8])
     *
     * Configure the rows/cols in KPP
     * LSB nibble in KPP is for 8 rows
     * MSB nibble in KPP is for 8 cols
     */
    reg_val = readw(KPCR);
    reg_val |= (1 << ROWS) - 1;            /* LSB */
    reg_val |= ((1 << COLS) - 1) << 8;    /* MSB */
    writew(reg_val, KPCR);

    /* Write 0's to KPDR[15:8] */
    reg_val = readw(KPDR);
    reg_val &= 0x00ff;
    writew(reg_val, KPDR);

    /* Configure columns as output, rows as input (KDDR[15:0]) */
    reg_val = readw(KDDR);
    reg_val |= 0xff00;
    reg_val &= 0xff00;
    writew(reg_val, KDDR);

    reg_val = readw(KPSR);
    reg_val &= ~(KBD_STAT_KPKR | KBD_STAT_KPKD);
    reg_val |= KBD_STAT_KPKD;
    reg_val |= KBD_STAT_KRSS | KBD_STAT_KDSC;
    writew(reg_val, KPSR);
    reg_val |= KBD_STAT_KDIE;
    reg_val &= ~KBD_STAT_KRIE;
    writew(reg_val, KPSR);
}

/*!
 * This function is called to scan the keypad matrix to find out the key press
 * and key release events. Make scancode and break scancode are generated for
 * key press and key release events.
 *
 * The following scanning sequence are done for
 * keypad row and column scanning,
 * -# Write 1's to KPDR[15:8], setting column data to 1's
 * -# Configure columns as totem pole outputs(for quick discharging of keypad
 * capacitance)
 * -# Configure columns as open-drain
 * -# Write a single column to 0, others to 1.
 * -# Sample row inputs and save data. Multiple key presses can be detected on
 * a single column.
 * -# Repeat steps the above steps for remaining columns.
 * -# Return all columns to 0 in preparation for standby mode.
 * -# Clear KPKD and KPKR status bit(s) by writing to a 1,
 *    Set the KPKR synchronizer chain by writing "1" to KRSS register,
 *    Clear the KPKD synchronizer chain by writing "1" to KDSC register
 *
 * @result    Number of key pressed/released.
 */
int mxc_kpp_scan_matrix(void)
{
    unsigned short reg_val;
    int col, row;
    int ret = 0;

    for (col = 0; col < ROWS; col++) {    /* Col */
        /* 2. Write 1.s to KPDR[15:8] setting column data to 1.s */
        reg_val = readw(KPDR);
        reg_val |= 0xff00;
        writew(reg_val, KPDR);

        /*
         * 3. Configure columns as totem pole outputs(for quick
         * discharging of keypad capacitance)
         */
        reg_val = readw(KPCR);
        reg_val &= 0x00ff;
        writew(reg_val, KPCR);

        hal_delay_us(2);

        /*
         * 4. Configure columns as open-drain
         */
        reg_val = readw(KPCR);
        reg_val |= ((1 << COLS) - 1) << 8;
        writew(reg_val, KPCR);

        /*
         * 5. Write a single column to 0, others to 1.
         * 6. Sample row inputs and save data. Multiple key presses
         * can be detected on a single column.
         * 7. Repeat steps 2 - 6 for remaining columns.
         */

        /* Col bit starts at 8th bit in KPDR */
        reg_val = readw(KPDR);
        reg_val &= ~(1 << (8 + col));
        writew(reg_val, KPDR);

        /* Delay added to avoid propagating the 0 from column to row
         * when scanning. */

        hal_delay_us(2);

        /* Read row input */
        reg_val = readw(KPDR);
        for (row = 0; row < ROWS; row++) {    /* sample row */
            if (TEST_BIT(reg_val, row) == 0) {
                ret = row * COLS + col;
            }
        }
    }

    /*
     * 8. Return all columns to 0 in preparation for standby mode.
     * 9. Clear KPKD and KPKR status bit(s) by writing to a .1.,
     * set the KPKR synchronizer chain by writing "1" to KRSS register,
     * clear the KPKD synchronizer chain by writing "1" to KDSC register
     */
    reg_val = 0x00;
    writew(reg_val, KPDR);
    reg_val = readw(KPSR);
    reg_val |= KBD_STAT_KPKD | KBD_STAT_KPKR | KBD_STAT_KRSS | KBD_STAT_KDSC;
    writew(reg_val, KPSR);

    return ret;
}

void mxc_kpp_uninit(void)
{
    unsigned short reg_val;

       /*
     * Clear the KPKD status flag (write 1 to it) and synchronizer chain.
     * Set KDIE control bit, clear KRIE control bit (avoid false release
     * events. Disable the keypad GPIO pins.
     */
    writew(0x00, KPCR);
    writew(0x00, KPDR);
    writew(0x00, KDDR);

    reg_val = readw(KPSR);
    reg_val |= KBD_STAT_KPKD;
    reg_val &= ~KBD_STAT_KRSS;
    reg_val |= KBD_STAT_KDIE;
    reg_val &= ~KBD_STAT_KRIE;
    writew(reg_val, KPSR);
}
