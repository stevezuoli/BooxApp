/**
 * Main command interaction between Picsel and Alien application.
 *
 * This file contains definitions and declarations needed for the commands
 * between the Picsel application event handlers and the Alien environment
 * in which they run. It includes initialising the Picsel library,
 * controlling it interactively, and some platform configuration.
 *
 * @file
 * $Id: picsel-app.h,v 1.42 2009/09/09 08:50:00 roger Exp $
 */
/* Copyright (C) Picsel, 2004-2008. All Rights Reserved. */
/**
 * @defgroup TgvCommand Commands for the TGV Library
 * @ingroup TgvCore
 *
 * The Picsel library primarily responds to commands issued by the Alien
 * application. These may originate as user keystrokes, menu options,
 * or application intialisation processes in the application. The
 * response to many of these commands will be as an asynchronous information
 * event notifying the application of the outcome.
 */

/**
 * @defgroup TgvUserInput User Input Commands
 * @ingroup TgvCommand
 *
 * Picsel TGV provides the functionality of several conventional
 * functions through a library API which can be used by an existing
 * or purpose-built application on a device. The commands include the main
 * state of the system such as initialisation, finalisation and suspension
 * as well user input such as keystrokes or menu options.
 *
 * @{
 */

#ifndef PICSEL_APP_H
#define PICSEL_APP_H

#include "alien-types.h"
#include "alien-legacy.h"
#include "picsel-entrypoint.h"
#include "picsel-threadmodel.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define PicselKeyNone PicselCmdNone

/**
 * User Input command abstraction methods.
 *
 * The Picsel library has two models for abstraction of user input from
 * the keyboard. Normally, the Alien application assigns individual keys to
 * commands like @ref PicselCmdPanDown. However Picsel libraries such as
 * CUI expect literal keys such as @ref PicselKeyDown, and will call
 * AlienConfig_setInputType(alienContext, PicselInputType_Key) to indicate this to
 * the Alien application.
 */
typedef enum PicselInputType
{
    /**
     * Some user input (such as pressing of the number 1 key) has been
     * assigned to a particular command (such as @ref PicselCmdZoomIn) by
     * the Alien application. The command code will be prefixed PicselCmd.
     * @product Most TGV products, including File Viewer.
     */
    PicselInputType_Command,
    /**
     * Some user input (such as @ref PicselKeySoftKey1) has been received
     * from the keyboard, but has not been interpreted yet. The command
     * code will be prefixed PicselKey.
     * @product CUI user interface products and applications.
     */
    PicselInputType_Key
}
PicselInputType;

/**
 * Options to control scaling the size of the document on screen.
 * @pre A document should already have been loaded. Although this need not
 *      have completed yet, at least one page should have been rendered.
 * @product Page-based document viewing products such as File Viewer and
 *          Browser.
 *
 * @see PicselApp_fitDocument()
 */
typedef enum PicselFit
{
    /**
     * Zoom the image on screen so that the width of the document page
     * fits exactly to the physical width of the device display. A typical
     * page is taller than it is wide, so the top or bottom of the
     * page is likely to be invisible.
     */
    PicselFitWidth = (1<<16),
    /**
     * Zoom the image on screen so that the height of the document page
     * fits exactly to the physical height of the device display. A typical
     * page will be shown with strips of background at the sides. This is
     * usually the minimum zoom scale.
     */
    PicselFitHeight,
    /**
     * Zoom the image on screen so that a whole page fits on the physical
     * device display. The view is likely to be too small to read the
     * content, and there are likely to be strips of background either at
     * sides or the top and bottom of the screen.
     */
    PicselFitPage
}
PicselFit;

/**
 * Commands specifying user input, e.g. from keystrokes or menu selections.
 * These have already been interpreted by the Alien application
 * into actions of the product. They should be passed to the Picsel
 * library using PicselApp_keyPress() and PicselApp_keyRelease().
 *
 * These will be handled asynchronously, so the screen will be updated
 * with a new image some time later. In some states, the Picsel library
 * will be unable to react to the command, and so will ignore it. For
 * example, after delivering the @ref AlienInformation_ZoomLimitReached
 * event indicating that it cannot zoom further in, the Picsel library
 * will silently ignore @ref PicselCmdZoomIn.
 *
 * Picsel does not expect application integrators to use all of these
 * commands in every application; it is a matter of choice.
 */
enum
{
    /**
     * Pan (scroll) the page up, so that the user can see a higher
     * part of the page. The page itself will move down the screen.
     * This is often mapped to the Up navigation key.
     *
     * When the top of the page is already visible, this command will be
     * ignored. In that case, the @ref AlienInformation_PanLimitsReached
     * event will have been notified to the application. However, if
     * @ref PicselConfigFV_panToPageEnable has been set and the top of the
     * current page is visible, this command will behave like
     * @ref PicselCmdPreviousPage.
     *
     * If the Picsel library is not in a state where a document is available
     * to be panned, for example if a CUI menu is on screen or if a document
     * has not been loaded successfully, then this may be ignored.
     *
     * In page-based Picsel products like File Viewer and Browser, the
     * distance panned by each step can be configured using
     * @ref PicselConfigFV_panUpAmount
     */
    PicselCmdPanUp = 65539,
    /**
     * Pan the page down; the opposite of @ref PicselCmdPanUp.
     * This is often mapped to the Down navigation key.
     */
    PicselCmdPanDown,
    /**
     * Pan the page to the left, so that the user can see content
     * on the left of the page. The page itself will move right. This is
     * often mapped to the Left navigation key.
     *
     * When the left margin of the page is already visible, this command
     * will be ignored. In that case, the
     * @ref AlienInformation_PanLimitsReached event will have been notified
     * to the application. However, if @ref PicselConfigFV_panToPageLeftRight
     * has been set is used and the left of the
     * current page is visible, this command will behave like
     * @ref PicselCmdPreviousPage.
     *
     * If the Picsel library is not in a state where a document is available
     * to be panned, for example if a CUI menu is on screen, then this may
     * be ignored.
     *
     * In File Viewer, the distance panned by each step can be configured
     * using @ref PicselConfigFV_panLeftAmount
     */
    PicselCmdPanLeft,
    /**
     * Pan the page to the right; the opposite of @ref PicselCmdPanLeft
     */
    PicselCmdPanRight,
    /**
     * Zoom in (scale up) the view of the page, so that content within
     * the page becomes larger, and a smaller proportion of the page is
     * visible at one time. This is often mapped to the number 1 key.
     *
     * This will be ignored if the maximum zoom scale has already been
     * reached, in which case @ref AlienInformation_ZoomLimitReached will
     * have been notified.
     *
     * If the Picsel library is not in a state where a document is available
     * to be zoomed, for example if a CUI menu is on screen or no document
     * has been loaded, then this may be ignored.
     *
     * The scale is incremented by @ref PicselConfigFV_zoomInAmount
     * for each iteration of @ref PicselCmdZoomIn normally, although
     * when in the @ref FlowMode_PowerZoom mode, the scale is incremented
     * by @ref PicselConfigFV_powerZoomInAmount.
     */
    PicselCmdZoomIn,
    /**
     * Rotate the document on screen by a multiple of 90 degrees, so that
     * text which was horizontal on screen becomes vertical. There are
     * usually two orientations available, so this command toggles between
     * them. The orientations are enabled by the
     * @ref PicselConfig_rotatedScreens configuration. It is often mapped
     * to the number 2 key.
     */
    PicselCmdRotate,
    /**
     * Zoom out (scale down) the view of the page.
     * This is the opposite of @ref PicselCmdZoomIn. It is often mapped to
     * the number 3 key.
     */
    PicselCmdZoomOut,
    /**
     * Display the previous page (lower numbered page) of the current
     * document, if a multi-page document has been loaded. The zoom and
     * pan position of the new page will normally be the same as the
     * previous view.
     *
     * If there is no previous page in the document, this command will be
     * ignored. This is not equivalent to a 'back' feature in a web browser
     * history. This is often mapped to the number 4 key.
     * @see AlienScreen_pagesChanged()
     */
    PicselCmdPreviousPage,
    /**
     * Display the next page (higher numbered page) of the current
     * document. This is the opposite of @ref PicselCmdPreviousPage.
     */
    PicselCmdNextPage,
    /**
     * This will zoom (scale) the view to fit the whole of the current page
     * on screen at once. However, PicselApp_fitDocument() is recommended
     * in preference to this.
     * @see PicselApp_fitDocument()
     */
    PicselCmdFitPage,
    /**
     * Display the first page of the current document, if a multi-page
     * document has been loaded.
     * @see PicselCmdPreviousPage, PicselCmdGotoPage.
     */
    PicselCmdFirstPage,
    /**
     * Display the last (highest numbered) page of the current document.
     *  This is the opposite of @ref PicselCmdFirstPage
     */
    PicselCmdLastPage,
    /**
     * Activate the item which is focused. This may mean selecting a menu
     * option or following a link to another web page. It is often mapped
     * to the Select button, for Picsel products which require selection.
     *
     * This command will be ignored if an interactive item is not in focus.
     * @see @ref TgvFocus_Model.
     * @product Picsel products with interactive content, such as Browser.
     */
    PicselCmdSelect,
    /**
     * Cancel the current focus within the content.
     * @see @ref TgvFocus_Model.
     */
    PicselCmdDefocus,
    /**
     * Zoom the view to fit the width of the page on screen.
     * @see PicselFitWidth
     */
    PicselCmdFitWidth,
    /**
     * Zoom the view to fit the height of the page on screen.
     * @see PicselFitHeight
     */
    PicselCmdFitHeight,
    /**
     * Scroll the current view of the document up by the height of the
     * screen.
     * @see PicselCmdPanUp
     */
    PicselCmdPanUpFullScreen,
    /**
     * Scroll the current view of the document down by the height of the
     * screen.
     * @see PicselCmdPanDown
     */
    PicselCmdPanDownFullScreen,
    /**
     * Scroll the current view of the document left by the width of the
     * screen.
     * @see PicselCmdPanLeft
     */
    PicselCmdPanLeftFullScreen,
    /**
     * Scroll the current view of the document right by the width of the
     * screen.
     * @see PicselCmdPanRight
     */
    PicselCmdPanRightFullScreen,

    /**
     * Reopen the previously viewed document; normally the previous web
     * page. This will normally cause network traffic and processing.
     *
     * @product CUI Browser, or any with the history list module.
     */
    PicselCmdHistoryBack,
    /**
     * Reopen the later viewed document, after having gone back through
     * the history with @ref PicselCmdHistoryBack. This will normally
     * cause network traffic and processing.
     *
     * @pre @ref PicselCmdHistoryBack was used recently.
     * @product CUI Browser, or any with the history list module.
     */
    PicselCmdHistoryForward,

    /**
     * Exit one step out of the mode hierarchy of the Picsel library. This
     * typically means closing a mode or shutting down an embedded instance
     * of a Picsel library. In normal configurations, this command is
     * ignored.
     *
     * @pre An instance of a Picsel library must be active and embedded
     * within another using PicselApp_addEmbedded().
     * @see PicselApp_addEmbedded()
     */
    PicselCmdBack,
    /**
     * Invalid input. This will be ignored. It is used where the
     * application architecture must deliver a command, but none is actually
     * required.
     * 0x100000 is not a valid character input on existing platforms,
     * as it is a Unicode Private Use Area-B, Supplementary codepoint.
     */
    PicselCmdNone = 0x100000,

    /**
     * Reserved for deprecated commands.
     */
    PicselCmdDeprecatedBase,

    /**
     * Reserved for Photo Lab Development Kit (@ref TgvProducts).
     * See picsel-photolab-control.h
     */
    PicselCmdPhotoLabBase = 0x200000

};

/**
 * Keystrokes specifying user input, from the physical keypad without
 * interpretation by the application.
 * The normal control mode is @ref PicselInputType_Command, and the
 * keystrokes below are normally ignored. If a compatible Picsel product
 * is initialised, it will call
 * AlienConfig_setInputType(alienContext, PicselInputType_Key) and then expect
 * these literal keystrokes using PicselApp_keyPress() and
 * PicselApp_keyRelease().
 *
 * In addition to the values defined here, any keys which can be mapped
 * to ASCII alphanumeric input (0-9,a-z,A-Z) can be provided, coded as the
 * UCS2 value (Least significant byte last, so it's the same as ASCII).
 *
 * @product CUI user interface products and applications.
 */
enum
{
    PicselKeyUp = 66000, /**< The Up navigation key */
    PicselKeyDown,       /**< The Down navigation key */
    PicselKeyLeft,       /**< The Left navigation key */
    PicselKeyRight,      /**< The Right navigation key */
    PicselKeyDedZoomIn,  /**< A dedicated zoom-in key, if available */
    PicselKeyDedZoomOut, /**< A dedicated zoom-out key, if available */
    PicselKeyDedSelect,  /**< The navigation Selection key. */
    PicselKeyOK,         /**< A dedicated OK key, if distinct from Select */
    PicselKeyCancel,     /**< The navigation Cancel key */
    PicselKeyBack,       /**< A dedicated Back history key, if available */
    PicselKeySoftKey1 = 66100, /**< Key with Application-defined purpose */
    PicselKeySoftKey2,   /**< Second application-defined purpose key */
    PicselKeySoftKey3,   /**< Third application-defined purpose key, if available */
    PicselKeySoftKey4,   /**< Fourth application-defined purpose key, if available */

    /* Alphanumeric ASCII key values are also acceptable. */

    /**
     * The key codes from PicselKeyOemBase up to PicselCmdNone are
     * available for purposes agreed between Picsel and the OEM.
     *
     * @product Key codes above PicselKeyOemBase are only meaningful in
     *          a customised Picsel library.
     */
    PicselKeyOemBase         =66500
};

/**
 * User input keystroke or command data type.
 *
 * This should normally be an enum value with a name beginning PicselCmd*
 * such as @ref PicselCmdPanUp, but can also be one beginning PicselKey*
 * such as @ref PicselKeyUp if @ref PicselInputType_Key has been configured.
 *
 * It can be passed to PicselApp_keyPress() and PicselApp_keyRelease().
 *
 */
typedef int PicselKey;

/**
 * Options to be specified during initialisation with PicselApp_start().
 *
 * Whereas most configuration options are specified by the Alien application
 * using PicselConfig_setInt() etc, these flags take effect before that
 * function can be called, and so are available here for passing as
 * parameters to PicselApp_start(). To specify more than one of these flags,
 * they may be bitwise OR'd (|) together.
 */
enum
{
    /**
     * Allocate memory from the system heap in many blocks.
     * By default, Picsel will use a single large block of memory, and will
     * call AlienMemory_malloc() exactly once.  On some multi-tasking
     * platforms, many smaller blocks may be preferable to allow dynamic
     * allocation when loading large documents, but may also slow down
     * or prevent the display of large bitmap images. Please contact Picsel
     * for advice or your device.
     * @see AlienMemory_malloc()
     */
    PicselApp_ExpandingHeap = 1
};

/**
 * A type representing a combination of the above flags, such as
 * @ref PicselApp_ExpandingHeap, for passing to PicselApp_start().
 */
typedef unsigned int PicselApp_StartFlags;

/**
 * Optional flags for the PicselApp_loadDocument() and related functions.
 * These flags may be bitwise OR'd (|) together, except where they are
 * mutually exclusive. The following flags are mutually exclusive, and at
 * most one of these may be used at a time:
 * @ref PicselApp_LoadFlags_ForceCache,
 * @ref PicselApp_LoadFlags_Refetch,
 * @ref PicselApp_LoadFlags_RefetchIfChanged
 * @ref PicselApp_LoadFlags_PreferCache
 */
enum
{
    /**
     * Attempt to fetch the document from the cache of previously loaded
     * files. If it is not available from the cache then the Picsel
     * library will notify @ref PicselLoadedStatus_NotLoaded without
     * attempting to load from the network or any other filing system.
     * @product Intended for use in web Browser products
     */
    PicselApp_LoadFlags_ForceCache       = 0x01,
    /**
     * Attempt to load a document from its source. The Picsel library will
     * ignore any cached copy of the file.
     * @product Intended for use in web Browser products
     */
    PicselApp_LoadFlags_Refetch          = 0x02,
    /**
     * Load a document using a local cache if available, but validate
     * content freshness with server and refetch it if changed. This is
     * normal behaviour for some web browsers. It consumes some network
     * bandwidth during the validation, but not as much as with
     * @ref PicselApp_LoadFlags_Refetch.
     * @product Intended for use in web Browser products
     */
    PicselApp_LoadFlags_RefetchIfChanged = 0x04,
    /**
     * Deprecated option; please do not use this.
     * A new view (tab/window) will be created, rather than replacing an
     * existing document.
     * @product Intended for use in web Browser products. This requires a
     *          Picsel library supporting multiple documents.
     */
    PicselApp_LoadFlags_NewView          = 0x08,
    /**
     * Deprecated option; please do not use this.
     * When a new document is loaded into a new view using
     * @ref PicselApp_LoadFlags_NewView, this can be specified as well, to
     * keep the current view in the foreground, and load the new document
     * in the background.
     * @product Intended for use in web Browser products. This requires a
     *          Picsel library supporting multiple documents.
     */
    PicselApp_LoadFlags_InBackground     = 0x10,
    /**
     * Attempt to refetch the document from the cache of previously loaded
     * files, even if the cached copy has expired. This may be useful for
     * loading pages in a history list, or to reduce network traffic. It
     * is almost the opposite of @ref PicselApp_LoadFlags_RefetchIfChanged.
     */
    PicselApp_LoadFlags_PreferCache      = 0x20
};

/**
 * A type representing a combination of the above flags, for example
 * @ref PicselApp_LoadFlags_ForceCache, for passing to functions like
 * PicselApp_loadDocument().
 */
typedef unsigned int PicselApp_LoadFlags;

/**
 * Details of a file to be accessed by PicselApp_start() etc.
 *
 * Filenames passed to the picsel library must use the '/'
 * character as the directory separator. If this is not the separator used
 * on the host OS, then the Alien application must substitute it before
 * passing filenames to the Picsel library and perform the reverse
 * substitution when AlienFile_open() and similar functions are called.
 *
 * The Alien application could deliver content to Picsel from a variety of
 * sources, such as encrypted data stores and dynamically generated
 * content, while avoiding the overhead of buffering it in a temporary
 * file. A standard file command should be used but with a proprietary
 * directory name, and AlienFile_read() modified to intercept that pattern
 * and to construct the content from the appropriate sources. Please
 * contact Picsel for examples of how to implement this.
 */
typedef struct PicselFileCommand
{
    /** The full name of the file to be accessed. This is often a literal
     * file name for the host OS, but can be customised as explained above.
     * Characters in the file name must be encoded with UTF-8. The
     * directory separator must be '/' as explained above. This must be a
     * full path, from the root of the filing system.
     */
    const void    *fileContents;
    /** Deprecated. Please set to 0. */
    unsigned long  fileLength;
    /** Deprecated. Please set to "\0" */
    char           fileExtension[8];
}
PicselFileCommand;

/** @} */ /* End of Doxygen group for Commands */

/**
 * @addtogroup TgvInitialisation
 * @{
 */

/**
 * Initialise the Picsel Library.
 *
 * This must be the first function to be called in the Picsel library.
 * It defines which Picsel product is to be used in this instance, and
 * provides some information that must be specified early.
 *
 * @runtime The whole initialisation will take a significant time, and
 * will consume significant platform resources. However, this function
 * will return once the Picsel Library's core environment is working and
 * ready to be paused. See @ref TgvAsync_Queue.
 *
 * @param[in] alienContext An opaque pointer to global data that the Alien
 *            application may wish to store. Picsel will pass this back in
 *            many Alien* calls, for example AlienConfig_ready(). Picsel will
 *            not access the memory it points to, so it is offered as a
 *            convenience for the application integrator. See
 *            AlienEvent_setPicselContext() for the comparable pointer that
 *            Picsel uses.
 * @param[in] fileCommand Details of a file to load during start-up, or
 *            @c NULL to initialise the library without loading a file.
 *            This is intended for File Viewer products. Or use
 *            PicselApp_loadDocument()*  later.
 * @param[in] initFn Configuration of the Picsel product to initialise. This
 *            is specified as a function pointer, for example
 *            Picsel_EntryPoint_FileViewer(). Each release of the Picsel TGV
 *            library is built with only certain products enabled.  See
 *            picsel-entrypoint.h for choices.
 * @param[in] threadFn Configuration of the threading model for the Picsel
 *            library, for example Picsel_ThreadModel_softThreads().  See
 *            picsel-threadmodel.h for details.
 * @param[in] flags Configuration options for the Picsel library which must
 *            be specified early in the process. This can be 0 or any
 *            combination of flags like @ref PicselApp_ExpandingHeap from the
 *            enum defined above.
 *
 * @return The queue status, normally 1. See @ref TgvAsync_Queue. The
 *         @ref AlienInformation_InitComplete event will be notified when
 *         the whole initialisation has been completed.
 * @post   After finishing use of the Picsel library, call
 *         PicselApp_shutDown().
 */
int PicselApp_start(Alien_Context       *alienContext,
                    PicselFileCommand   *fileCommand,
                    Picsel_initialiseFn  initFn,
                    Picsel_ThreadModelFn threadFn,
                    PicselApp_StartFlags flags);

/**
 * Begin another Picsel product within the same context as another.
 *
 * @pre Another Picsel product must already have been started with
 * PicselApp_start().
 * @post Call PicselApp_removeEmbedded() before PicselApp_shutDown().
 *
 * Success in starting the embedded product will be notified by
 * @ref AlienInformation_InitComplete and failure by
 * @ref AlienInformation_InitFailed. The @ref AlienInformation_AppHandle
 * structure will identify which product was being started.
 *
 * @product This API is not normally required, and not all builds or
 * products support embedding within each other. Please ask Picsel about
 * compatibility with your library.
 *
 * @param[in] picselContext See AlienEvent_setPicselContext().
 * @param[in] initFn        Configuration of the Picsel product to initialise.
 *                          This is specified as a function pointer, e.g.,
 *                          @ref Picsel_EntryPoint_FileViewer.
 *                          Each release of the Picsel TGV library is built
 *                          with only certain products enabled.
 *                          See @ref picsel-entrypoint.h for choices.
 * @return The queue status, normally 1. See @ref TgvAsync_Queue. The
 *         @ref AlienInformation_InitComplete event will be notified when
 *         the initialisation has been completed; this will include a
 *         @ref AlienInformation_AppHandle which will be needed later.
 *
 *         If the initialisation fails, an @ref AlienInformation_InitFailed
 *         event will be notified, specifying the @ref Picsel_initialiseFn
 *         which was passed to this function.
 * @see PicselApp_removeEmbedded()
 */
int PicselApp_addEmbedded(Picsel_Context      *picselContext,
                          Picsel_initialiseFn  initFn);

/**
 * Shut down a Picsel product running embedded within another.
 *
 * @pre  The first Picsel product must have been initialised with
 *       PicselApp_start() and a second must have been initialised with
 *       PicselApp_addEmbedded().
 *
 * @param[in] picselContext See AlienEvent_setPicselContext().
 * @param[in] appHandle     The identifier for the Picsel library instance to
 *                          shut down. This may be @ref PICSEL_APP_ID_NONE to
 *                          identify the most recently added embedded product,
 *                          or may be the value returned with
 *                          @ref AlienInformation_InitComplete.
 * @return    The queue status, normally 1. See @ref TgvAsync_Queue.
 * @see PicselApp_addEmbedded()
 */
int PicselApp_removeEmbedded(Picsel_Context   *picselContext,
                             Picsel_AppHandle *appHandle);

/** The most recently started Picsel library instance, if it does not
 *  yet have an ID. This is used with PicselApp_removeEmbedded() and
 *  PicselApp_bringEmbeddedToFront().
 */
#define PICSEL_APP_ID_NONE  ((void*)0)


/**
 * Bring an embedded Picsel product to the front.
 *
 * Brings the specified product to the front and resumes its execution.
 * The previously active application will be placed on a stack of suspended
 * products.
 *
 * @pre  The first Picsel product must have been initialised with
 *       PicselApp_start() and a second must have been initialised with
 *       PicselApp_addEmbedded().
 *
 * @param[in] picselContext Set by AlienEvent_setPicselContext().
 * @param[in] initFn        Pointer to the Picsel product initialisation
 *                          function, e.g. @ref Picsel_EntryPoint_FileViewer.
 *                          The value for @c initFn should be the one received
 *                          in an earlier @ref AlienInformation_InitComplete
 *                          event.
 *
 * @param[in] appHandle The identifier of the application to bring to the front.
 *                      The identifier is provided in
 *                      @ref AlienInformation_AppHandle.appHandle sent with the
 *                      @ref AlienInformation_InitComplete event.
 *                      Use @ref PICSEL_APP_ID_NONE to open the product at the
 *                      top of the stack of suspended products.  If there
 *                      are only two products running,
 *                      use @ref PICSEL_APP_ID_NONE to switch to the
 *                      suspended product.
 *
 * @return    The queue status, normally 1. See @ref TgvAsync_Queue.
 *            The Alien application cannot assume that the requested product
 *             will have been brought to the front when this function returns.
 *
 * @see PicselApp_addEmbedded().
 */
int PicselApp_bringEmbeddedToFront(Picsel_Context      *picselContext,
                                   Picsel_initialiseFn  initFn,
                                   Picsel_AppHandle    *appHandle);

/**
 * Exit the Picsel product.
 *
 * Shuts down all Picsel products and frees all internal resources before
 * returning.
 *
 * If the Picsel library supports annotation and there are any unsaved
 * annotations, PicselApp_shutDown() will return 0 and the product will
 * keep running.  Later, an @ref AlienInformation_AnnotationSaveOrDiscard
 * event will be sent. See @ref TgvAsync_Queue.
 *
 * If the Picsel library supports editing, unsaved edits will be discarded.
 * Use PicselEdit_isDocumentEdited() to, for example, give the user the option
 * to save their edits before calling PicselApp_shutDown().
 *
 * @param[in] picselContext Set by AlienEvent_setPicselContext().
 *
 * @retval 1  The product has shut down.
 * @retval 0  The product is still running.
 *
 * @product All Picsel TGV products.
 *
 */
int PicselApp_shutDown(Picsel_Context *picselContext);

/** @} */ /* End of Doxygen group: Initialisation */

/**
 * @addtogroup TgvUserInput
 * @{
 */


/**
 * Load a document, closing the currently open one.
 *
 * This function loads documents from the device's local filesystem. To
 * specify the document using a URL, call PicselApp_loadDocumentUrl() or
 * PicselApp_loadUrlUserInput().
 *
 * @param[in] picselContext  Set by AlienEvent_setPicselContext().
 * @param[in] fileCommand    Details of file to open.
 * @param[in] picselView     Set to NULL. Reserved for future use.
 * @param[in] flags          Flags specifying loading options. A bitwise-or
 *                           combination of flag values, e.g.,
 * (@ref PicselApp_LoadFlags_ForceCache|@ref PicselApp_LoadFlags_InBackground),
 *                           or pass 0 for the default options.
 *
 * @return  The queue status, normally 1. See @ref TgvAsync_Queue.
 *
 */
int PicselApp_loadDocument(Picsel_Context          *picselContext,
                           PicselFileCommand       *fileCommand,
                           Picsel_View             *picselView,
                           PicselApp_LoadFlags      flags);

/**
 * Load a document from memory, closing the currently open one.
 *
 * @param[in] picselContext  Set by AlienEvent_setPicselContext().
 * @param[in] fileContents   Pointer to the start of the file.
 * @param[in] fileLength     File length, in bytes.
 * @param[in] fileExtension  File extension  e.g. "doc", "html".  Providing
 *                           the name of the file extension will help the
 *                           Picsel library to choose a suitable Document
 *                           Agent for the file. See @ref TgvInitialiseAgents.
 * @param[in] picselView     Set to NULL. Reserved for future use.
 * @param[in] flags          Flags specifying loading options. A bitwise-or
 *                           combination of flag values, e.g.
 *(@ref PicselApp_LoadFlags_ForceCache|@ref PicselApp_LoadFlags_InBackground),
 *                           or pass 0 for the default options.
 *
 * @return   The queue status, normally 1. See @ref TgvAsync_Queue.
 */
int PicselApp_loadDocumentFromMemory(Picsel_Context      *picselContext,
                                     unsigned char       *fileContents,
                                     unsigned long        fileLength,
                                     const char          *fileExtension,
                                     Picsel_View         *picselView,
                                     PicselApp_LoadFlags  flags);

/**
 * Load a document from a URL, closing the currently open one.
 *
 * @c urlName must be a well-formed URL, e.g. "http://www.picsel.com/", or this
 * function will fail to load the document.
 * If the URL was entered by the user, call PicselApp_loadUrlUserInput().
 *
 *
 *
 * @param[in] picselContext  Set by AlienEvent_setPicselContext().
 * @param[in] urlName        A valid URL of the document to open.
 * @param[in] picselView     Set to NULL. Reserved for future use.
 * @param[in] flags          Flags specifying loading options. A bitwise-or
 *                           combination of flag values, e.g.,
 *(@ref PicselApp_LoadFlags_ForceCache|@ref PicselApp_LoadFlags_InBackground),
 *                           or pass 0 for the default options.
 *
 * @return   The queue status, normally 1. See @ref TgvAsync_Queue.
 */
int PicselApp_loadDocumentUrl(Picsel_Context     *picselContext,
                              const char         *urlName,
                              Picsel_View        *picselView,
                              PicselApp_LoadFlags flags);

/**
 * Load a document from a URL entered by the user, closing the currently open
 * one.
 *
 * Use this function when @c urlName may be badly-formed e.g. when it was
 * entered by the user. The Picsel library will try to correct common
 * errors in URL entry, e.g. it will add "http://" when this is
 * missing.  Otherwise call PicselApp_loadDocumentUrl().
 *
 * @param[in] picselContext  Set by AlienEvent_setPicselContext().
 * @param[in] urlName        The URL to be opened.
 * @param[in] picselView     Set to NULL. Reserved for future use.
 * @param[in] flags          Flags specifying loading options. A bitwise-or
 *                           combination of flag values, e.g.,
 * (@ref PicselApp_LoadFlags_ForceCache|@ref PicselApp_LoadFlags_InBackground),
 *                           or pass 0 for the default options.
 *
 * @return   The queue status, normally 1. See @ref TgvAsync_Queue.
 */
int PicselApp_loadUrlUserInput(Picsel_Context     *picselContext,
                               const char         *urlName,
                               Picsel_View        *picselView,
                               PicselApp_LoadFlags flags);

/**
 * Reload the document.
 *
 * Refresh the document displayed by the Picsel library by reloading
 * from the source of the document.
 *
 * If the document is a remote document and @ref PicselApp_LoadFlags_Refetch
 * is specified in @c flag this will force a refetch over the network of the
 * entire document, and all referenced images, frames,
 * external stylesheets and script files.
 *
 * @warning If the document is a web page which was
 * dynamically generated in response to a POST request, then the request
 * will be POSTed again. There is a risk that the second request will cause
 * an undesired side-effect on the web server, for example repeating an online
 * transaction. The Alien application should confirm this interactively with
 * the user before calling PicselApp_reloadDocument().
 *
 * @param[in] picselContext  Set by AlienEvent_setPicselContext().
 * @param[in] picselView     Set to NULL. Reserved for future use.
 * @param[in] flag           Flag specifying loading options. Only
 *                           PicselApp_LoadFlags_Refetch and
 *                           PicselApp_LoadFlags_RefetchIfChanged are valid
 *                           for this function.
 *                           Pass 0 for the default options.
 *
 * @return   The queue status, normally 1. See @ref TgvAsync_Queue.
 */
int PicselApp_reloadDocument(Picsel_Context      *picselContext,
                             Picsel_View         *picselView,
                             PicselApp_LoadFlags  flag);

/**
 * Close the current document.
 *
 * @param[in] picselContext  Set by AlienEvent_setPicselContext().
 * @param[in] picselView     Set to NULL. Reserved for future use.
 *
 * @return   The queue status, normally 1. See @ref TgvAsync_Queue.
 */
int PicselApp_closeDocument(Picsel_Context  *picselContext,
                            Picsel_View     *picselView);

/** @} */ /* End of Doxygen group: UserInput */

/**
 *
 * Hide the Picsel product.
 *
 * Call when the library should keep running but should halt
 * screen updates.  e.g. when an error dialog is displayed or a system
 * dialog is visible and using the same screen area as the Picsel library.
 *
 * The Picsel library continues to run, only screen updates (i.e. calls to
 * AlienScreen_update()) are halted.  PicselApp_suspend() should be called
 * if processing should also be suspended.
 *
 * @note This function operates synchronously, unlike most Picsel functions,
 * and returns after performing its task. However, other Alien* functions may
 * be called from within this function.
 *
 * @param[in] picselContext Set by AlienEvent_setPicselContext().
 *
 * @retval 1  On success.
 * @retval 0  On failure.
 *
 * @post No calls to AlienScreen_update() will be made until
 *       PicselApp_exposed() is called.
 *
 * @see PicselApp_exposed().
 *
 * @ingroup TgvInitialisation
 */
int PicselApp_hidden(Picsel_Context *picselContext);

/**
 * Show the Picsel product, after it has been hidden with PicselApp_hidden().
 *
 * Call when Picsel library can safely start calling AlienScreen_update()
 * again. If the Picsel library is already exposed, call PicselScreen_redraw()
 * to request a single redraw.
 *
 * Calling this function will cause Picsel to redraw the screen, however
 * because of the asynchronous architecture of TGV, it is not guaranteed that
 * the screen will be redrawn by the time this
 * function returns.  See @ref TgvAsync_Queue.
 *
 * @param[in] picselContext Set by AlienEvent_setPicselContext().
 *
 * @return   The queue status, normally 1. See @ref TgvAsync_Queue.
 *
 * @ingroup TgvInitialisation
 */
int PicselApp_exposed(Picsel_Context *picselContext);

/**
 *
 * Pause the Picsel product.
 *
 * Call when the Picsel product should be suspended, e.g. to handle an
 * incoming telephone call.  Screen updates, document processing and handling
 * of requests are halted.
 *
 * It is still possible to call asynchronous Picsel library functions after
 * this function has been called, however they will not be actioned until
 * PicselApp_resume() is called.  The event queue may become full, causing
 * further events to be rejected. See @ref TgvAsync_Queue.
 *
 * Calls to PicselApp_suspend() and PicselApp_resume() aren't
 * reference counted, so calling PicselApp_resume() will cause the
 * product to start running again, regardless of the number of prior
 * calls to PicselApp_suspend().
 *
 * Call PicselApp_hidden() if only suspending of screen updates is required,
 * and application processing should continue.
 *
 * @post The Picsel library will carry out no tasks until PicselApp_resume()
 *  is called.
 *
 * @param[in] picselContext Set by AlienEvent_setPicselContext().
 *
 * @ingroup TgvInitialisation
 */
void PicselApp_suspend(Picsel_Context *picselContext);

/**
 *
 * Resume the Picsel product.
 *
 * The counterpart to PicselApp_suspend(). Call when the Picsel library
 * can start updating the screen, processing the document and
 * responding to requests again.
 *
 * Calls to PicselApp_suspend() and PicselApp_resume() aren't
 * reference counted, so calling PicselApp_resume() will cause the
 * product to start running again, regardless of the number of prior
 * calls to PicselApp_suspend().
 *
 * @param[in] picselContext Set by AlienEvent_setPicselContext().
 *
 * @ingroup TgvInitialisation
 */
void PicselApp_resume(Picsel_Context *picselContext);

/**
 * Inform the Picsel library that a key has been pressed.
 *
 * If AlienConfig_setInputType() has been called with
 * @ref PicselInputType_Key, @c key should be one of the PicselKey* values,
 * e.g. @ref PicselKeyDown.
 *
 * By default, or if AlienConfig_setInputType() has been called with
 * @ref PicselInputType_Command, @c key should be one of the PicselCmd*
 * values, e.g. @ref PicselCmdPanDown.
 *
 * If the Alien application is generating key repeat events
 * (see PicselApp_setKeyBehaviour()), it should call this function repeatedly
 * while the key is held down.  The time between calls is up to the Alien
 * application.
 *
 *
 * @param[in] picselContext Set by AlienEvent_setPicselContext().
 * @param[in] key           The key or command code.
 * @param[in] repeats       0 for the key down event; the number of repeats
 *                          so far for each event while the key
 *                          is held down.
 *
 * @pre If calling with @c repeats > 0, PicselApp_setKeyBehaviour() must
 *      have been called with @c softRepeats set to 0.
 *
 * @return  The queue status, normally 1. See @ref TgvAsync_Queue.
 *
 * @see PicselApp_keyRelease()
 *
 * @product All Picsel products.
 *
 * @ingroup TgvUserInput
 */
int PicselApp_keyPress(Picsel_Context *picselContext,
                       PicselKey       key,
                       int             repeats);

/**
 * Inform the Picsel library that a key has been released.
 *
 * @param[in] picselContext Set by AlienEvent_setPicselContext().
 * @param[in] key           The key released.
 *
 * @pre PicselApp_setKeyBehaviour() has been called with @c softRelease
 * set to 0.
 *
 * @return  The queue status, normally 1. See @ref TgvAsync_Queue.
 *
 * @product All Picsel products.
 *
 * @ingroup TgvUserInput
 *
 * @see PicselApp_keyPress()
 */
int PicselApp_keyRelease(Picsel_Context *picselContext,
                         PicselKey       key);

/**
 * Inform the Picsel library that a timer set by AlienTimer_request()
 * has expired.
 *
 * @runtime The Picsel library will process its internal event queue for
 * around @c slice milliseconds (as set by PicselApp_setTimeSlice())
 * before returning from this function.  If there are events remaining on the
 * queue, the Picsel library will call AlienTimer_request() to request another
 * @c slice milliseconds to continue processing events.
 *
 * @param[in] picselContext Set by AlienEvent_setPicselContext().
 * @param[in] id            The timer reference, as set by AlienTimer_request().
 *
 * @return  Always 1.
 *
 * @product All Picsel products.
 *
 * @ingroup TgvSystemTimer
 *
 * @see PicselApp_setTimeSlice(), @ref TgvAsync_Queue, @ref TgvHost_Thread.
 */
int PicselApp_timerExpiry(Picsel_Context *picselContext,
                          unsigned long   id);

/**
 * @addtogroup TgvConfigureCore
 * @{
 */

/**
 * Inform the Picsel library whether the Alien application will deliver key
 * repeat and release events.
 *
 * Picsel highly recommend that all OEMs provide key release events. The
 * Picsel library will perform much better and be more responsive to the
 * end user if the Alien application provides key release events.
 *
 * The default settings are that the Alien application should send neither
 * repeat nor release events.
 *
 * @c softRepeats should be set to 1 if the Picsel library should generate
 * key repeat events internally.
 * The Picsel library will generate key repeat events after a call to
 * PicselApp_keyPress(), until PicselApp_keyRelease() is called.
 *
 * The Picsel library can only generate key repeats if the Alien application
 * delivers key release events.
 *
 *
 * @param picselContext Set earlier by AlienEvent_setPicselContext().
 * @param softRepeats   0: Key repeats will be delivered by the Alien
                           application, using PicselApp_keyPress() . @n
 *                      1: No key repeats will be delivered by the Alien
                           application. @c softRelease must be set to 0
                           to use this option.
 * @param softRelease   0: Key releases will be delivered by the Alien
 *                         application, using PicselApp_keyRelease(). @n
 *                      1: No key releases will be delivered by the Alien
 *                      application.
 *
 * @see PicselApp_getKeyBehaviour()
 * @ref PicselConfig_keyAutorepeatDelay
 */
void PicselApp_setKeyBehaviour(Picsel_Context *picselContext,
                               int             softRepeats,
                               int             softRelease);

/**
 * Discover whether the Picsel library expects key repeat and key
 * release events from the Alien application.
 *
 * One of @c softRepeats and @c softRelease can be set to NULL if the setting
 * is not required. They cannot both be set to NULL.
 *
 * @param[in]  picselContext Set earlier by AlienEvent_setPicselContext().
 * @param[out] softRepeats   Will point to whether Picsel library expects key
 *                           repeat events.@n
 *                      0: Alien application should deliver key repeat events.@n
 *                      1: Picsel library will generate key repeat events
 *                         internally.
 * @param[out] softRelease   Will point to whether Picsel library expects key
 *                           repeat events.@n
 *                      0: Alien application should deliver key release
 *                         events.@n
 *                      1: Picsel library will generate key release events
 *                         internally.
 *
 * @see PicselApp_setKeyBehaviour()
 *
 */
void PicselApp_getKeyBehaviour(Picsel_Context *picselContext,
                               int            *softRepeats,
                               int            *softRelease);
/** @} */ /* End of Doxygen group: ConfigureCore */

/**
 * Request information about a loaded document.
 *
 * The title, URL and MIME type of the document will be returned via an
 * @ref AlienInformation_FileInfoResult event.
 *
 * @param[in] picselContext  Set by AlienEvent_setPicselContext().
 * @param[in] picselView     Set to NULL. Reserved for future use.
 *
 * @return   The queue status, normally 1. See @ref TgvAsync_Queue.
 *           Later, the Picsel library will call AlienEvent_information()
 *           with a @ref AlienInformation_FileInfoResult event
 *           and a @ref AlienInformation_FileInfo struct.
 *
 * @ingroup TgvContentInformation
 */
int PicselApp_getFileInfo(Picsel_Context  *picselContext,
                          Picsel_View     *picselView);

/**
 * Stop the loading of a document.
 *
 * If the document is remote, its download will be cancelled.  Any animations
 * will be stopped and no further pictures will be loaded.
 *
 * @param[in] picselContext  Set by AlienEvent_setPicselContext().
 * @param[in] picselView     Set to NULL. Reserved for future use.
 *
 * @return  The queue status, normally 1. See @ref TgvAsync_Queue.
 *
 * @ingroup TgvUserInput
 */
int PicselApp_stopDocument(Picsel_Context  *picselContext,
                           Picsel_View     *picselView);

/**
 * Configure how long Picsel code runs for before yielding and for
 * how long the Picsel library will typically yield.
 *
 * The Picsel library will set default times if this function is never called.
 *
 * The optimal values for @c delay and @c slice will depend on the device
 * hardware, the tasks required of the Picsel library and the reponsiveness
 * required from the Picsel library. Typical values are 10ms for @c delay and
 * 200ms for @c slice.  See @ref TgvDeveloping_App_Optimise.
 *
 *
 * @param[in] picselContext Set by AlienEvent_setPicselContext().
 * @param[in] delay         The time, in milliseconds, that Picsel will
 *                          yield for. This will be the default delay
 *                          passed to AlienTimer_request().
 * @param[in] slice         The time, in milliseconds, that Picsel will
 *                          run for. This will approximately how long a
 *                          call to PicselApp_timerExpiry() will take
 *                          before returning.
 *
 * @see @ref TgvAsync_Queue, @ref TgvHost_Thread, @ref TgvSystemTimer.
 *
 * @ingroup TgvConfigureCore
 *
 */
void PicselApp_setTimeSlice(Picsel_Context *picselContext,
                            int             delay,
                            int             slice);

/**
 * Request that Picsel yield (soon after this call completes).
 *
 * The Picsel library will yield as soon as it can, without waiting for the
 * end of its current timeslice.   This may be useful if all other
 * processing should be stopped to handle another, processor intensive, task
 * (e.g. an incoming video call).
 *
 * The Picsel library will yield some time after this call completes.
 * Callers who require a guaranteed immediate yield, within their call to
 * the Picsel library, should use PicselApp_yieldImmediately instead.
 *
 * @param[in] picselContext Set by AlienEvent_setPicselContext().
 *
 * @see @ref TgvHost_Thread, @ref TgvThreading.
 *
 * @ingroup TgvInitialisation
 */
void PicselApp_yieldNow(Picsel_Context *picselContext);

/**
 * Instruct that the Picsel product yields immediately.
 *
 * This function is designed to be called from an alien function which is
 * called from the Picsel product and requires that the Picsel product
 * yields before it can complete.
 *
 * Do not use this function to request processing be stopped to handle
 * another processor intensive task (e.g. an incoming video call).  Use
 * PicselApp_yieldNow for this instead.
 *
 * This function must be called from the same thread as the scheduler, if
 * the Picsel application is already threaded.
 *
 * When the Picsel product later resumes execution, there is no guarantee
 * which of its threads will be scheduled so other threads may call other
 * Alien functions before the original Alien function is rescheduled and
 * gets the opportunity to complete.  It is therefore possible that other
 * Picsel threads may call the Alien function which forced the yield.
 * Callers of this function should thus support being re-entered.
 *
 * @param[in] picselContext Set by AlienEvent_setPicselContext().
 *
 * @see @ref TgvHost_Thread, @ref TgvThreading.
 *
 * @retval 1  The Picsel library did yield or is not running;
 * @retval 0  The Picsel library could not yield.  The Alien
 *            function should return immediately, reporting an
 *            error if appropriate.
 *
 * @ingroup TgvInitialisation
 */
int PicselApp_yieldImmediately(Picsel_Context *picselContext);


/**
 * Fit the current document page to the screen.
 *
 * The current page can be zoomed so that it occupies the entire width or
 * height of the screen, or so that the entire page is visible.
 *
 * @ref PicselFitWidth may be useful for reading a page on a landcape screen,
 * @ref PicselFitHeight may be useful viewing a vertical slice of a
 * spreadsheet page, and @ref PicselFitPage may be useful for scanning through
 * the pages in a document to find a particular one.
 *
 * @param[in] picselContext Set by AlienEvent_setPicselContext()
 * @param[in] fit           How the page should be fitted to the screen.
 *
 * @return  The queue status, normally 1. See @ref TgvAsync_Queue.
 *
 * @product Fileviewer and Browser products.
 *
 * @ingroup TgvUserInput
 */
int PicselApp_fitDocument(Picsel_Context *picselContext,
                          PicselFit       fit);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !PICSEL_APP_H */
