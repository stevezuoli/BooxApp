/**
 * Legacy definitions.
 *
 * PLEASE DO NOT USE DEFINITIONS OR APIS FOUND IN THIS FILE - THEY ARE
 * LIABLE TO BE REMOVED.
 *
 * This file contains definitions and declarations needed for the calls
 * between the Picsel application event handlers and the environment in which
 * they run.
 *
 * These are obsolete and deprecated, and should not be used in new Alien
 * applications.
 *
 * $Id: alien-legacy.h,v 1.20 2009/06/26 16:35:06 toms Exp $
 *
 * @file
 */
/* Copyright (C) Picsel, 2007-2008. All Rights Reserved. */

#ifndef ALIEN_LEGACY_H
#define ALIEN_LEGACY_H

#include "picsel-pointer.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* These definitions are for backwards compatibility with legacy applications
 * only. Use PicselCmdCode or key code definitions for new development.
 * For example, you should use PicselCmdZoomIn rather than PicselKeyZoomIn.
 */
#define  PicselKeyPanUp               PicselCmdPanUp               /**< for legacy code only */
#define  PicselKeyPanDown             PicselCmdPanDown             /**< for legacy code only */
#define  PicselKeyPanLeft             PicselCmdPanLeft             /**< for legacy code only */
#define  PicselKeyPanRight            PicselCmdPanRight            /**< for legacy code only */
#define  PicselKeyZoomIn              PicselCmdZoomIn              /**< for legacy code only */
#define  PicselKeyRotate              PicselCmdRotate              /**< for legacy code only */
#define  PicselKeyZoomOut             PicselCmdZoomOut             /**< for legacy code only */
#define  PicselKeyPreviousPage        PicselCmdPreviousPage        /**< for legacy code only */
#define  PicselKeyNextPage            PicselCmdNextPage            /**< for legacy code only */
#define  PicselKeyFitPage             PicselCmdFitPage             /**< for legacy code only */
#define  PicselKeyFirstPage           PicselCmdFirstPage           /**< for legacy code only */
#define  PicselKeyLastPage            PicselCmdLastPage            /**< for legacy code only */
#define  PicselKeySelect              PicselCmdSelect              /**< for legacy code only */
#define  PicselKeyFitWidth            PicselCmdFitWidth            /**< for legacy code only */
#define  PicselKeyFitHeight           PicselCmdFitHeight           /**< for legacy code only */
#define  PicselKeyPanUpFullScreen     PicselCmdPanUpFullScreen     /**< for legacy code only */
#define  PicselKeyPanDownFullScreen   PicselCmdPanDownFullScreen   /**< for legacy code only */
#define  PicselKeyPanLeftFullScreen   PicselCmdPanLeftFullScreen   /**< for legacy code only */
#define  PicselKeyPanRightFullScreen  PicselCmdPanRightFullScreen  /**< for legacy code only */

#define  PicselKeyCode                PicselKey                    /**< for legacy code only */
#define  PicselCmdCode                PicselKey                    /**< for legacy code only */

#define PicselConfigCuiBrowser_zoomInKey             PicselConfig_zoomInKey             /**< for legacy code only */
#define PicselConfigCuiBrowser_zoomOutKey            PicselConfig_zoomOutKey            /**< for legacy code only */
#define PicselConfigCuiBrowser_rotateKey             PicselConfig_rotateKey             /**< for legacy code only */
#define PicselConfigCuiBrowser_previousPageKey       PicselConfig_previousPageKey       /**< for legacy code only */
#define PicselConfigCuiBrowser_nextPageKey           PicselConfig_nextPageKey           /**< for legacy code only */
#define PicselConfigCuiBrowser_fitPageKey            PicselConfig_fitPageKey            /**< for legacy code only */
#define PicselConfigCuiBrowser_fitWidthKey           PicselConfig_fitWidthKey           /**< for legacy code only */
#define PicselConfigCuiBrowser_fitHeightKey          PicselConfig_fitHeightKey          /**< for legacy code only */
#define PicselConfigCuiBrowser_firstPageKey          PicselConfig_firstPageKey          /**< for legacy code only */
#define PicselConfigCuiBrowser_lastPageKey           PicselConfig_lastPageKey           /**< for legacy code only */
#define PicselConfigCuiBrowser_panUpKey              PicselConfig_panUpKey              /**< for legacy code only */
#define PicselConfigCuiBrowser_panDownKey            PicselConfig_panDownKey            /**< for legacy code only */
#define PicselConfigCuiBrowser_panLeftKey            PicselConfig_panLeftKey            /**< for legacy code only */
#define PicselConfigCuiBrowser_panRightKey           PicselConfig_panRightKey           /**< for legacy code only */
#define PicselConfigCuiBrowser_panUpFullScreenKey    PicselConfig_panUpFullScreenKey    /**< for legacy code only */
#define PicselConfigCuiBrowser_panDownFullScreenKey  PicselConfig_panDownFullScreenKey  /**< for legacy code only */
#define PicselConfigCuiBrowser_panLeftFullScreenKey  PicselConfig_panLeftFullScreenKey  /**< for legacy code only */
#define PicselConfigCuiBrowser_panRightFullScreenKey PicselConfig_panRightFullScreenKey /**< for legacy code only */
#define PicselConfigCuiBrowser_selectKey             PicselConfig_selectKey             /**< for legacy code only */
#define PicselConfigCuiBrowser_defocusKey            PicselConfig_defocusKey            /**< for legacy code only */
#define PicselConfigCuiBrowser_defocus2Key           PicselConfig_defocus2Key           /**< for legacy code only */
#define PicselConfigCuiBrowser_historyBackKey        PicselConfig_historyBackKey        /**< for legacy code only */
#define PicselConfigCuiBrowser_historyForwardKey     PicselConfig_historyForwardKey     /**< for legacy code only */

/**
 * This is deprecated; please use PicselFileviewer_gotoPage() instead.
 */
#define PicselCmdGotoPage             (PicselCmdDeprecatedBase)
#define PicselKeyGotoPage             (PicselCmdDeprecatedBase)
/**
 * This is deprecated; please use PicselScreen_resize() instead.
 */
#define PicselCmdFullScreenToggle     (PicselCmdDeprecatedBase+1)
#define PicselKeyFullScreenToggle     (PicselCmdDeprecatedBase+1)

#define Picsel_Database_messagesInternal PicselDatabase_messagesInternal /**< for legacy code only */
#define Picsel_Database_messagesAlien    PicselDatabase_messagesAlien    /**< for legacy code only */

#define PicselSearch_gotoPage PicselSearch_fromPage  /**< for legacy code only */

#define PicselApp_screenCapture PicselScreen_capture /**< for legacy code only */

#define Picsel_setTimeSlice   PicselApp_setTimeSlice /**< for legacy code only */
#define Picsel_yieldNow       PicselApp_yieldNow     /**< for legacy code only */

#define PicselLocale_setUTCDifference PicselLocale_setUtcDifference

/* Legacy names for alien event APIs.  Do not use in new code. */
#define AlienInformation_event                 AlienInformation_Event
#define AlienInformation_imageSubSampled       AlienInformation_ImageSubSampled
#define AlienInformation_documentLoaded        AlienInformation_DocumentLoaded
#define AlienInformation_documentLoadedInfo    AlienInformation_DocumentLoadedInfo
#define AlienInformation_zoom                  AlienInformation_Zoom
#define AlienInformation_zoomData              AlienInformation_ZoomInfo
#define AlienInformation_thumbnailDone         AlienInformation_ThumbnailDone
#define AlienInformation_thumbnail             AlienInformation_ThumbnailDoneInfo
#define AlienInformation_screenResized         AlienInformation_ScreenResized
#define AlienInformation_screenResize          AlienInformation_ScreenResizedInfo
#define AlienInformation_initComplete          AlienInformation_InitComplete
#define AlienInformation_splashScreenDone      AlienInformation_SplashScreenDone
#define AlienInformation_initFailed            AlienInformation_InitFailed
#define AlienInformation_requestShutdown       AlienInformation_RequestShutdown
#define AlienInformation_fileInfoResult        AlienInformation_FileInfoResult
#define AlienInformation_fileInfo              AlienInformation_FileInfo
#define AlienInformation_fileSaveExtension     AlienInformation_FileSaveExtensionInfo
#define AlienInformation_fileSaveDone          AlienInformation_FileSaveDoneInfo
#define AlienInformation_searchResult          AlienInformation_SearchResult
#define AlienInformation_thumbnailResult       AlienInformation_ThumbnailResult
#define AlienInformation_flowMode              AlienInformation_FlowMode
#define AlienInformation_flowModeData          AlienInformation_FlowModeInfo

/* Legacy names for config values.  Do not use in new code */
#define PicselConfig_selectWidgetAltSelect     PicselConfig_multiSelectWidgetAltSelect
#define PicselConfig_selectWidgetAltDefocus    PicselConfig_multiSelectWidgetAltDefocus

/* Legacy name for error codes.  Do not use in new code */
#define PicselDocumentError                    PicselError

/* Legacy name for bitmap buffer alignment.  Do not use in new code */
#define ByteAlignmentDefault                   BufferAlignmentDefault
#define ByteAlignment4                         BufferAlignment4
#define ByteAlignment8                         BufferAlignment8
#define ByteAlignment16                        BufferAlignment16
#define ByteAlignment32                        BufferAlignment32

/* Legacy name for PixelBlock rotation.  Do not use in new code */
#define Picsel_PixelBlock_RotateNone           PicselRotation0
#define Picsel_PixelBlock_RotateCW             PicselRotation90
#define Picsel_PixelBlock_Rotate180            PicselRotation180
#define Picsel_PixelBlock_RotateCCW            PicselRotation270

#define InputType_Command  PicselInputType_Command
#define InputType_Key      PicselInputType_Key

/* Legacy name to cap database max size. Do not use in new code */
#define PicselConfig_mediaDatabaseMaxSize PicselConfig_mainMediaDatabaseMaxSize

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !ALIEN_LEGACY_H */
