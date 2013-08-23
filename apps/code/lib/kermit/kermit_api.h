/**
 * \name kermit_api.h (proprietary kermit driver)
 * \brief Kermit APIs
 *
 * Copyright (C) 2010, Marvell Semiconductors
 *
 * \author Alice Hsia (ahsia@Marvell.com)
 */
#ifndef _KERMIT_API_H
#define _KERMIT_API_H

#ifdef __cplusplus
  extern "C" {
#endif

typedef enum {WF_INIT=0, WF_DU, WF_GC4, WF_GC8, WF_GC16, WF_GU, WF_NOP} Waveforms_T;

/** \fn kermit_init
 * \brief initialize kermit API.
 * \param  none
 * \return kermit ID
 */
int kermit_init(void);

/** \fn kermit_exit
 * \brief release kermit device.
 * \param  none
 * \return none
 */
void kermit_exit(void);

/** \fn kermit_mmap
 * \brief maps kermit memory (Frame Buffer)
 * \param  memory size
 * \return pointer to mapped memory or NULL if fail
 */
void * kermit_mmap(unsigned long * size);

/** \fn kermit_free
 * \brief unmap's kermit memory
 * \param  memory address
 * \param  memory size
 * \return none
 */
void kermit_free(void *vaddr, unsigned long size);

/** \fn kermit_flash_screen
 * \brief Full screen flashing
 * \param  none
 * \return none
 */
void kermit_full_update(void);

/** \fn kermit_partial_update
 * \brief partial update 
 * \param  none
 * \return none
 */
void kermit_partial_update(void);

/** \fn kermit_display_on
 * \brief Turn display ON
 * \param  none
 * \return status
 */
int kermit_display_on (void);

/** \fn kermit_display_off
 * \brief Turn display OFF
 * \param  none
 * \return status
 */
int kermit_display_off (void);

/** \fn kermit_update_done
 * \brief Wait for update finished.
 * \param  none
 * \return status
 */
int kermit_update_done(void);

/** \fn kermit_is_update_done
 * \brief Check update is done or not.
 * \param  none
 * \return 0 means update is not finished.
 */
int kermit_is_update_done(void);

/** \fn kermit_update_done
 * \brief Change current waveform gray scale.
 * \param  none
 * \return status
 */
int kermit_set_grayscale(int gray_scale);

/** \fn kermit_panel_on
 * \brief Turn panel power ON
 * \param  none
 * \return status
 */
int kermit_panel_on (void);

/** \fn kermit_panel_off
 * \brief Turn panel power OFF
 * \param  none
 * \return status
 */
int kermit_panel_off (void);

/** \fn kermit_set_waveform
 * \brief Switch waveform's mode
 * \param  waveform mode
 * \return none
 */
void kermit_set_waveform(Waveforms_T wf);

/** \fn kermit_versions
 * \brief get kermit driver expected revision id and kermit hw real revision id
 * \param  expectedRevId, revisionId
 * \return none
 */
void kermit_versions (unsigned int * expectedRevId, unsigned int * revisionId);

int kermit_set_vcom(unsigned int vcom);

#ifdef __cplusplus
}
#endif

#endif // KERMIT_API_H
