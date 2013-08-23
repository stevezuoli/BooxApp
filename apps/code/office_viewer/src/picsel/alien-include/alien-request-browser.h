/**
 * Requests for User Input in relation to web browsing.
 *
 * This file is effectively part of alien-request.h but has been separated
 * out because it contains many definitions used only in web browsing.
 *
 * @file
 * $Id: alien-request-browser.h,v 1.1 2009/08/12 12:24:43 roger Exp $
 */
/* Copyright (C) Picsel, 2005-2009. All Rights Reserved. */

#ifndef ALIEN_REQUEST_BROWSER_H
#define ALIEN_REQUEST_BROWSER_H

#include "alien-types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ************************** Transition Request ************************* */

/**
 * @defgroup TgvBrowserPageTransitions Browser Page Transitions
 * @ingroup TgvBrowser
 *
 * The web Browser requests confirmation from the Alien application before
 * opening a new page. There are various reasons whiy this may be required,
 * and in some cases the Alien application may wish to deny it.
 *
 * The enums and structs which will form part of a call to
 * AlienUserRequest_request(), to request a transition from one page to
 * another.
 */

/**
 * @var PicselUserRequest_Type_Transition
 * A transition to a new document is requested, for example when a
 * hyperlink is clicked by the user.
 *
 * This request type is provided primarily as a hook to allow
 * application-specific code to be run when a transition is triggered
 * e.g. playing a sound, checking the new URL against a blacklist, etc.
 * This may be handled quietly by the application, without prompting the
 * end user.
 *
 * See @ref PicselUserRequest_Transition for a description of the struct
 * sent along with this request type.
 */

/**
 * @ingroup TgvBrowserPageTransitions
 *
 * See @ref PicselUserRequest_Transition for details on how this value should
 * be used within a @ref PicselUserRequest_Type_Transition type event.
 */
typedef enum PicselUserRequest_Transition_Type
{
    /**
     * A normal transition; the user clicked on a link or a control to change
     * the page.
     */
    PicselUserRequest_Transition_Type_Normal     = (1<<16),

    /**
     * @note: Popup windows are not currently supported.
     *
     * The currently loaded document asked the Picsel Library to display a popup
     * window. The Alien Application can decide, or the user could be prompted
     * for a decision, if the popup should be displayed.  Since popup windows
     * are not supported, if it is to be displayed then it will @em replace the
     * current document.
     */
    PicselUserRequest_Transition_Type_Popup,

    /**
     * The user requested that the page is loaded in a new view.
     */
    PicselUserRequest_Transition_Type_NewView,

    /**
     * A refresh occurred, often caused by a META-refresh tag in the document.
     * This is different from a @ref PicselUserRequest_Transition_Type_Reload,
     * as it is possible to redirect the browser to another location - see
     *  <A href="http://www.w3.org/TR/WCAG20-TECHS/H76.html">http://www.w3.org/TR/WCAG20-TECHS/H76.html</A>
     * for more information on the META tag.
     */
    PicselUserRequest_Transition_Type_Refresh,

    /**
     * A Javascript function requested a page change.
     */
    PicselUserRequest_Transition_Type_Javascript,

    /**
     * Sent if a reload is triggered from within the Picsel Library. This will
     * usually be sent when the end user has pressed a "Reload" or "Refresh"
     * button.
     */
    PicselUserRequest_Transition_Type_Reload
}
PicselUserRequest_Transition_Type;

/**
 * @ingroup TgvBrowserPageTransitions
 *
 * The web Browser intends to display a new page.
 *
 * @c PicselUserRequest_Transition_Response is sent in response to
 * @ref PicselUserRequest_Type_Transition requests.
 *
 * See @ref PicselUserRequest_Transition for information on the struct which
 * this value will be part of.
 */
typedef enum PicselUserRequest_Transition_Response
{
    /**
     * Accept the transition request. A new document will be loaded.
     */
    PicselUserRequest_Transition_Accept = (1<<16),
    /**
     * Reject the transition request. The current document will remain loaded.
     */
    PicselUserRequest_Transition_Reject
}
PicselUserRequest_Transition_Response;

/**
 * @ingroup TgvBrowserPageTransitions
 *
 * Part of a request to transition from an existing URL to a
 * new URL.  If the request is accepted, the URL will be loaded in place of the
 * original document.
 *
 * It is not necessary to display a prompt to the user for this request.  A
 * transition could be accepted straight away, in which case
 * PicselUserRequest_notify() should be called next.
 *
 * Transitions with "frame" set to 0 are main page transitions.  When the
 * main page has loaded, @ref AlienInformation_DocumentLoaded will be sent.
 *
 * Transitions with "frame" set to 1 indicate that the source file of a
 * frame or iframe is changing.  When this completes,
 * @ref AlienInformation_DocumentLoaded will be sent. This is simply flag, and
 * is not analogous to the <code>frames</code> array in, for instance, Javascript; setting
 * frame to 2 will not change the target frame of the page transition.
 *
 * @em Note: if a frame changes while the main document is loading, only one
 * @ref AlienInformation_DocumentLoaded will be sent.
 *
 * This struct is sent along with a @ref PicselUserRequest_Type_Transition type
 * event.
 */
typedef struct PicselUserRequest_Transition
{
    Picsel_View     *picselView;     /**< [in]  NULL, reserved for future use.*/
    const char      *targetUrl;      /**< [in]  URL of page to which
                                                transition is requested. */
    char            *postbuf;        /**< [in]  POST request data        */
    int              postlen;        /**< [in]  length of POST request data */
    int              frame;          /**< [in]  1 if this is a frame
                                                transition */
    PicselUserRequest_Transition_Type     type;     /**< [in]  Type of
                                                               transition */
    PicselUserRequest_Transition_Response response; /**< [out] Response */
}
PicselUserRequest_Transition;

/* ************************* Authentication Input *************************** */

/**
 * @defgroup TgvBrowserHttpFeatures Authentication, Cookies and other HTTP features
 * @ingroup TgvBrowser
 *
 * The HTTP protocol includes a number of features requiring user input, or consent.
 * The enums and structs described here support that,
 * by notifying the Alien application.
 */

/**
 * @ingroup TgvBrowserHttpFeatures
 *
 * Use the @c PicselUserRequest_Authentication_Fields value as part of
 * a @ref PicselUserRequest_Authentication struct to specify which
 * authentication fields should be requested from the end user.
 */
typedef enum PicselUserRequest_Authentication_Fields
{
    /** Username and password fields will be both be displayed. */
    PicselUserRequest_Authentication_UserNamePassword = (1<<16),

    /** Only the username field will be displayed */
    PicselUserRequest_Authentication_UserNameOnly,

    /** Only the password field will be displayed */
    PicselUserRequest_Authentication_PasswordOnly
}
PicselUserRequest_Authentication_Fields;

/**
 * @var PicselUserRequest_Type_Authentication
 *
 * A request for authentication details.
 *
 * Usernames and passwords are typically requested when a web server has
 * returned a status code of "401 Unauthorised" . The @ref
 * PicselUserRequest_Authentication struct contains further information
 * about this request, such as which authentication elements
 * (e.g. password/username) should be requested.
 *
 * See @ref PicselUserRequest_Authentication for a description of the
 * struct sent along with this type of request.
*/

/**
 * @ingroup TgvBrowserHttpFeatures
 *
 * Information required for HTTP user Authentication.
 *
 * The @ref passwordDefault and @ref usernameDefault values will only be filled
 * in when required, as specified in the @ref fields value. For example if @ref
 * fields is @ref PicselUserRequest_Authentication_UserNameOnly,  then @c
 * passwordDefault will be NULL and @c usernameDefault will point to a string.
 * The default may be the empty string (""). When responding with
 * PicselUserRequest_notify(), fill in the @c username and @c password fields
 * as specified in @ref PicselUserRequest_Authentication_Fields "fields".
 *
 * The @c realm is the area of the host computer that the password is for. For
 * example, the following prompts are appropriate: "Please enter the password
 * for @c realm on @c host" or "The server @c host requires a password for @c
 * realm"
 *
 * @see http://www.ietf.org/rfc/rfc2617.txt for more information on HTTP
 * authentication.
 */
typedef struct PicselUserRequest_Authentication
{
    PicselUserRequest_Authentication_Fields fields;

    const char *url;             /**< [in]  The URL that caused the
                                            authentication request. */
    int         digest;          /**< [in]  Set to 1 if this is a digest
                                            authentication. */
    const char *usernameDefault; /**< [in]  default for username.    */
    char       *username;        /**< [out] user's entered username. */

    const char *passwordDefault; /**< [in]  default for password.    */
    char       *password;        /**< [out] user's entered password. */


    const char *realm;          /**< [in] Area of host.              */
    const char *host;           /**< [in] The host requesting
                                 authentication (from URL). */

}
PicselUserRequest_Authentication;



/* ************************* Confirmation request *************************** */

/**
 * @defgroup TgvBrowserPompts Scripted User Prompts
 * @ingroup TgvBrowser
 *
 * The enums and structs sent with AlienUserRequest_request(), to deliver a
 * notification, confirmation or request for information (often
 * Javascript-like) to the user.
 *
 * The Alien Application could also respond to the request without
 * displaying anything to the device user, based on local policy.
 */

/**
 * @var PicselUserRequest_Type_Notify
 *
 * Inform the user of an event.
 *
 * Text will typically be generated by JavaScript or a web server.
 * This request should not have a reject option.
 *
 * See @ref PicselUserRequest_Notify for a description of the struct
 * sent along with this type of request.
 */

/**
 * @ingroup TgvBrowserPompts
 *
 * This struct is sent along with a @ref PicselUserRequest_Type_Notify event,
 * which is intended to be similar to the Javascript function alert('message').
 * A dialogue box displaying the caption, and only allowing the user to
 * acknowledge the message, should be displayed.
 *
 * The message text is provided as part of the Javascript call.
 *
 */
typedef struct PicselUserRequest_Notify
{
    const char *caption;     /**< [in] Text to be displayed in request */
}
PicselUserRequest_Notify;


/**
 * @var PicselUserRequest_Type_Query
 *
 * Ask the user to confirm an action, usually with a Yes/No (OK/Cancel)
 * answer.
 *
 * See @ref PicselUserRequest_Query for a description of the struct sent
 * along with this type of request, and @ref
 * PicselUserRequest_Query_Response for details of the struct sent in
 * response to this request.
 */

/**
 * @ingroup TgvBrowserPompts
 *
 * Sent in response to @ref PicselUserRequest_Type_Query requests.
 */
typedef enum PicselUserRequest_Query_Response
{
    PicselUserRequest_Query_Response_Yes = (1<<16),
    PicselUserRequest_Query_Response_No
}
PicselUserRequest_Query_Response;


/**
 * @ingroup TgvBrowserPompts
 *
 * This struct is sent along with a @ref PicselUserRequest_Type_Query event,
 * and is intended to be similar to the Javascript function confirm('question').
 * A dialogue box showing the question, and allowing the user to reply (usually
 * with "OK" and "Cancel" buttons) could be displayed.
 *
 * The question text is provided in @c caption.
 */
typedef struct PicselUserRequest_Query
{
    const char              *caption;  /**< [in]  Text to be displayed.    */
    PicselUserRequest_Query_Response  response; /**< [out] User's response */
}
PicselUserRequest_Query;

/**
 * @var PicselUserRequest_Type_CreateBookmark
 *
 * Ask the Alien Application if it is OK to create a bookmark for a URL.
 *
 * See @ref PicselUserRequest_CreateBookmark for a description of the
 * struct sent along with this type of request.
 */


/**
 * @ingroup TgvBrowserPompts
 *
 * Type of input format specification to be applied to text input dialogue box.
 *
 * This value will form part of the nested struct, @ref
 * PicselUserRequest_String,  sent along with a @ref
 * PicselUserRequest_Type_String
 *
 */
typedef enum PicselUserRequest_InputFormat
{
    /**
     * PicselUserRequest_InputFormatSpec_None. No input format specification is
     * used. There are no restrictions on what data the user can enter.
     */
    PicselUserRequest_InputFormatSpec_None = (1<<16),
    /**
     * PicselUserRequest_InputFormatSpec_WCss. User input will be restricted
     * using the Wireless-CSS input format specification, detailed in
     * <A href="http://www.openmobilealliance.org/technical/release_program/docs/Browsing/V2_3-20080331-A/OMA-WAP-WCSS-V1_1-20061020-A.pdf">OMA-WAP-WCSS-V1_1-20061020-A</A>, section 19.
     */
     PicselUserRequest_InputFormatSpec_WCss

}
PicselUserRequest_InputFormat;

/**
 * @ingroup TgvBrowserPompts
 *
 * Wireless-CSS Input Format Specification. These values indicate
 * whether data entry is mandatory or not.
 *
 * This value forms part of the struct, @ref PicselUserRequest_String, sent
 * along with a @ref PicselUserRequest_Type_String event.
 */
typedef enum PicselUserRequest_InputRequired
{
    /**
     * The source document has not specified if user input is mandatory. A zero
     * length string is allowed, but only if the Wireless-CSS for the field
     * allows it.
     */
    PicselUserRequest_InputRequired_NotSpecified = (1<<16),
    /**
     * User input is not mandatory. A zero length string is permitted,
     * even if the Wireless-CSS for the field does not allow it.
     */
    PicselUserRequest_InputRequired_False,
    /**
     * User input is mandatory and the user is required to enter some data. A
     * zero length string is not permitted, even if the Wireless-CSS for the
     * field allows it.
     */
    PicselUserRequest_InputRequired_True
}
PicselUserRequest_InputRequired;

/**
 * @ingroup TgvBrowserPompts
 *
 * Settings to be applied to a Javascript "prompt" dialogue box.
 *
 * This struct should be sent along with a @ref PicselUserRequest_Type_String
 * request.
 */
typedef struct PicselUserRequest_String
{
    /**
     * [in] The default contents of the dialogue input field.
     */
    const char *stringDefault;
    /**
     * [in]     Text to be displayed in the request.
     */
    const char *caption;
    /** [in]    Maximum allowed input characters.
     *          If zero (0) is specified, then there will be no limit on the
     *          number of input characters.
     */
    int         maxChar;
    /** [in]    boolean flag:
     *
     *          1 - entry may include newlines.
     *          0 - entry may not include newlines.
     */
    int         multiline;
    /**
     *  [out]   User's response, as typed into the text field of the Javascript
     *          prompt.
     */
    char       *string;
    /**
     * [in]  If set to 1, entry is secret - for example, a password.  The
     * characters should be obscured.
     */
    int         secret;
    /**
     * [in]     dialogue name:  allows the named instance of the dialogue to
     *          override the default behaviour using an alien-defined override
     *          mechanism. Set this to NULL if it is not used.
     */
    const char *name;

    struct
    {
        /** [in] Indicates which input format specification is used.
         *       If type is not PicselUserRequest_InputFormat_None,
         *       the appropriate data is passed via the data union
         *       below.
         */
        PicselUserRequest_InputFormat type;

        union
        {
            /*
             * Wireless-CSS Input Format Specification
             */
            struct
            {
                /** [in] Specifies whether this field is
                 *       mandatory.
                 */
                PicselUserRequest_InputRequired required;

                /** [in] The actual input format specification. May
                 *       be NULL if no format specification is
                 *       provided.
                 */
                const char *specification;
            }
            wcss;
        }
        data;
    }
    format;
}
PicselUserRequest_String;

/* ************************** AntiVirus ********************************** */
/**
 * @defgroup TgvBrowserAntiVirus Antivirus scan requests
 * @ingroup TgvBrowser
 *
 * Structs and enums sent/received to process antivirus scan requests.
 *
 * The Alien Application Antivirus library should validate the URL and
 * display a dialogue to the user if necessary.
 *
 * If the Alien Application calls PicselUserRequest_notify() with @ref
 * PicselUserRequest_Result_Accepted, then the Picsel Library will continue to
 * download the file.
 *
 * If the Alien Application calls PicselUserRequest_notify() with @ref
 * PicselUserRequest_Result_Rejected, then the Picsel Library will cease
 * downloading the file.
 *
 * @em Note: The Alien Application should deal with removing any partial files
 * from the file system.
 */

/**
 * @var PicselUserRequest_Type_AntiVirus_URL
 *
 * Ask the Alien Application if a URL is safe to open. If the Alien
 * Application responds by sending @ref
 * PicselUserRequest_Result_Accepted, then the URL is opened and
 * AntiVirus scanning can begin. If the Alien Application responds with
 * @ref PicselUserRequest_Result_Rejected, then the Alien Application
 * should display an appropriate message - a handle to the file will not
 * be opened.
 *
 * See @ref PicselUserRequest_AntiVirus_URL for a description of the
 * struct sent along with this type of request.
 */

/**
 * @var PicselUserRequest_Type_AntiVirus_Content
 *
 * Sent along with a portion of a file to be scanned for viruses. If the
 * Alien Application accepts this event, then reading and scanning of
 * the file will continue.
 *
 * Rejecting it will close the handle to the file, and the amount of
 * data read so far (and marked as safe) will be returned via the
 * @ref PicselUserRequest_AntiVirus_Content.
 *
 * See @ref PicselUserRequest_AntiVirus_Content for a description of the
 * struct sent along with this type of request.
 */

/**
 * @ingroup TgvBrowserAntiVirus
 *
 * This forms part of @ref PicselUserRequest_Type_AntiVirus_URL requests.
 */
typedef struct PicselUserRequest_AntiVirus_URL
{
    /**
     * [in] Set by antiVirusInitialise(@c alienContext),
     * see PicselApp_start() for @c alienContext information.
     */
    Alien_AntiVirus_Context *antiVirusContext;
    const char              *url; /**< [in]  URL to be checked. */
}
PicselUserRequest_AntiVirus_URL;


/**
 * @ingroup TgvBrowserAntiVirus
 *
 * Sent in response to a request for a file to be scanned for viruses.
 *
 * The Alien antivirus library should scan the contents and display an
 * error dialogue to the user if necessary.
 *
 * Returning @ref PicselUserRequest_Result_Accepted informs the Picsel
 * library that this data block did not contain any viruses known to the
 * scanner being used by the Alien Application. If @ref
 * PicselUserRequest_Result_Rejected is returned, the Picsel Library will
 * process the amount of data specifed by safeLength and ignore the rest of
 * the file.
 *
 * Any subsequent files will be checked using new @ref
 * PicselUserRequest_Type_AntiVirus_URL and @ref
 * PicselUserRequest_Type_AntiVirus_Content requests.
 */
typedef struct PicselUserRequest_AntiVirus_Content
{
    /**
     * [in] Alien antivirus context.
     */
    Alien_AntiVirus_Context *antiVirusContext;
    /**
     * [in] URL of the file currently being scanned.
     */
    const char              *url;
    /**
     * [in] MIME type of file, may be NULL if unavailable.
     */
    const char              *mimeType;
    /**
     * [in] Data to be checked.
     */
    const char              *data;
    /** [in] Length of data block      */
    int                      dataLength;
    /**
     * [in] If the end of the file has been reached, the Alien Application
     * should set this to 1.     */
    int                      eof;
    /**
     *  [out] A virus was found.
     */
    int                      virusDetected;
    /**
     * [out] If request is rejected, alien should specify how much data can be
     * safely used
     */
    int                      safeLength;
}
PicselUserRequest_AntiVirus_Content;

/* ***************************** Confirm Cookie *************************** */
/**
 * @var PicselUserRequest_Type_Cookie
 *
 * Confirm whether a cookie should be accepted or rejected.
 *
 * See @ref PicselUserRequest_Cookie for a description of the struct sent
 * along with this type of request.
 */

/**
 * @ingroup TgvBrowserHttpFeatures
 *
 * @section HttpCookies HTTP Cookies
 *
 * Specifies what the Picsel Library should do with a cookie.
 *
 * This value will form part of a @ref PicselUserRequest_Cookie struct, sent
 * as part of a @ref PicselUserRequest_Type_Cookie event.
 */
typedef enum PicselUserRequest_Cookie_Action
{
    /**
     * Accept a new cookie.
     */
    PicselUserRequest_Cookie_Receive = (1<<16),
    /**
     * Receive a cookie that is already stored, replacing the old value with the
     * new value.
     */
    PicselUserRequest_Cookie_ReceiveReplace,
    /**
     * Send a cookie back to the site that initially placed it.
     */
    PicselUserRequest_Cookie_Send
}
PicselUserRequest_Cookie_Action;

/**
 * @ingroup TgvBrowserHttpFeatures
 *
 * Specifies the cookie target/source. This tells the Alien Application if the
 * cookie is being generated by a server (in which case the cookie details will
 * be in the incoming http headers) or by a Javascript call (in which case the
 * cookie detail will be contained within the information sent by the Javascript
 * function).
 *
 * This value will form part of a @ref PicselUserRequest_Cookie struct, sent
 * as part of a @ref PicselUserRequest_Type_Cookie event.
 */
typedef enum PicselUserRequest_Cookie_Type
{
    /**
     *  Send/receive server cookie.
     */
    PicselUserRequest_Cookie_Server = (1<<16),
    /**
     * Send/receive a cookie generated by Javascript.
     */
    PicselUserRequest_Cookie_Javascript
}
PicselUserRequest_Cookie_Type;

/**
 * @ingroup TgvBrowserHttpFeatures
 *
 * This struct is sent in response to a new cookie being received, or before
 * sending a stored cookie.
 *
 * A suitable dialogue, allowing the user to accept or reject the cookie, could
 * be displayed. Picsel recommends that any such message be used infrequently,
 * so as not to interfere with the end user experience.
 *
 * This is used for @ref PicselUserRequest_Type_Cookie requests.
 */
typedef struct PicselUserRequest_Cookie
{
    /**
     * [in] Cookie name.
     */
    char                           *name;
    /**
     * [in] Cookie domain.
     */
    char                           *domain;
    /**
     * [in] Cookie data
     */
    char                           *data;
    /**
     * [in] Whether cookie is to be sent or received
     */
    PicselUserRequest_Cookie_Action action;
    /**
     * [in]  Whether cookie is from a server, or created by javascript.
     */
    PicselUserRequest_Cookie_Type   type;
}
PicselUserRequest_Cookie;

/* ************************* Send HTTP "Referer" header ********************* */

/**
 * @addtogroup TgvBrowserHttpFeatures
 *
 * @section HttpReferer Referer HTTP Header
 *
 * Some websites use HTTP <a href="http://www.w3.org/Protocols/rfc2616/rfc2616-sec14.html#sec14.36">Referer</a>
 * as, for instance, method of establishing where their traffic
 * is coming from, and other trend-analysis. Many users, though, see this as a
 * privacy concern and prefer not to send the "HTTP-referer" header when
 * requesting URL's.
 *
 * It is not recommended to ask the device user to confirm their preference for
 * every page they view, and so Picsel suggests that the integrator makes this
 * part of the device policy - perhaps asking the user to confirm their
 * preference during the device's initial setup phase, and allowing the user to
 * alter their preference later, if desired.
 *
 * See @ref PicselUserRequest_HttpReferrer
 */


/**
 * @var PicselUserRequest_Type_HttpReferrer
 *
 * Confirm whether the HTTP referrer header should be sent along with an
 * http request.
 *
 * See @ref PicselUserRequest_HttpReferrer for a description of the
 * struct sent along with this type of request, and further discussion on
 * how to deal with this event.
 */

/**
 * @ingroup TgvBrowserHttpFeatures
 *
 * This request is made to confirm sending of the HTTP "referer" header as part
 * of HTTP requests.
 *
 * Although it is possible to prompt the end user for a decision about whether
 * to send the HTTP referer header, it is better that a policy decision be
 * taken on a default response to this request. It could be beneficial to
 * educate end users as to how they can reverse the policy, should they wish to
 * do so.
 *
 * This is used for @ref PicselUserRequest_Type_HttpReferrer requests.
 */
typedef struct PicselUserRequest_HttpReferrer
{
    const char *referrerUrl;    /**< [in]  URL of referrer */
    const char *targetUrl;      /**< [in]  URL of page to which referrer
                                           will be sent */
}
PicselUserRequest_HttpReferrer;


/* ************************* Confirm Security Certificate ******************* */

/**
 * @defgroup TgvBrowserEncryption SSL Security Certificates
 * @ingroup TgvBrowser
 *
 * When accessing a secure website (https), a Security Certificate is sent by
 * the site, as part of the HTTP response sent before any HTML.
 *
 * The Picsel Library will examine the security certificate for authenticity
 * and accuracy. If errors are discovered, the Picsel Library will ask the Alien
 * Application how to proceed.
 */

/**
 * @var PicselUserRequest_Type_Certificate
 *
 * Confirm whether the security certificate should be trusted.
 *
 * See @ref PicselUserRequest_Certificate for a description of the
 * struct sent along with this type of request.
 */

/**
 * @ingroup TgvBrowserEncryption
 *
 * This indicates various types of error which may be detected with a website's
 * security certificate.
 *
 * This value forms part of a @ref PicselUserRequest_Certificate struct.
 */
typedef enum PicselUserRequest_Certificate_Error
{
    /**
     * An error has been found that does not fit any
     * of the other criteria.
     */
    PicselUserRequest_Certificate_CertError = (1<<16),
    /**
     * The certificate has expired.
     */
    PicselUserRequest_Certificate_DateError,
    /**
     * The certicate has been revoked; it may have been obtained or used
     * fraudulently.
     */
    PicselUserRequest_Certificate_AuthError,
    /**
     * The hostname the certificate was issued by did not match the host the
     * certificate claimed to be issued by.
     */
    PicselUserRequest_Certificate_HostError
}
PicselUserRequest_Certificate_Error;

/**
 * @ingroup TgvBrowserEncryption
 *
 * This request is made to inform the Alien Application that errors have been
 * found with the security certificate. A dialogue box with an appropriate
 * prompt about the certificate's error(s) is typically displayed and the user
 * asked to accept or reject the certificate/connection.
 *
 * To proceeed with the connection, the Alien Application should respond by
 * setting:
 *
 * @code PicselUserRequest_Request.result = PicselUserRequest_Result_Accepted; @endcode
 *
 * The connection can be rejected by setting:
 *
 * @code PicselUserRequest_Request.result = PicselUserRequest_Result_Rejected; @endcode
 */
typedef struct PicselUserRequest_Certificate
{
    PicselUserRequest_Certificate_Error error;

    /**
     * Error message, if available.
     */
    const char *message;
    /**
     * Hostname of the issuing host.
     */
    const char *host;

    /*  Certificate details */
    /**
     * Common name (CN) of the host, for example
     * "www.picsel.com/emailAddress=developer@picsel.com"
     */
    const char *subjectCN;
    /**
     * Entire "Subject" field, for example "C=UK, ST=Strathclyde, L=Braehead, O=Picsel
     * Technologies Ltd, OU=Picsel,
     * CN=www.picsel.com/emailAddress=developer\@picsel.com"
     */
    const char *subject;
    /**
     * Common name (CN) of the issuer, for example "AAA Certificate Services".
     */
    const char *issuerCN;
    /**
     * Entire "Issuer" field, for example "CN=AAA Certificate Services,
     * O=Comodo CA Limited,L=Salford,ST=Greater Manchester,C=GB"
     */
    const char *issuer;
    /**
     * Not valid before this date, for example "10/04/08 01:00:00 GMT"
     */
    const char *notBefore;
    /**
     * Not valid after this date, for example "11/04/11 00:59:59 GMT"
     */
    const char *notAfter;
    /**
     * DNS attribute of "Subject Alternative Name" (SAN) property,  or subject's
     * Common Name if SAN does not exist.
     */
    const char *dNS;
}
PicselUserRequest_Certificate;

/* ************************* Confirm Redirection **************************** */

/**
 * @addtogroup TgvBrowserHttpFeatures
 *
 * @section HttpRedirection HTTP Redirection
 *
 * Example: The device user has requested page http://example.com/index.html -
 * example.com is no longer available, though. but it's http response to the GET
 * request is "Redirect: another-example.com/incoming.html".
 *
 * The Alien Application needs to inform the Picsel Library if this is
 * acceptable or not - this may be a policy decision, if it is from a non-secure
 * site to another non-secure site. However, if the redirect is from secure to
 * non-secure, then Picsel suggests that the device user should be asked to make
 * the decision or, at least, be informed of what has happened so that they do
 * not enter confidential information into a non-secure site.
 *
 * See @ref PicselUserRequest_Redirect
 */

/**
 * @var PicselUserRequest_Type_Redirect
 *
 * Ask the user to confirm whether to redirect from one site to another.
 *
 * See @ref PicselUserRequest_Redirect for a description of the struct
 * sent along with this type of request.
 */

/**
 * @ingroup TgvBrowserHttpFeatures
 *
 * Information surrounding the redirect event, such as which page was initially
 * requested and which page is now the target.
 *
 * If there is POST data which will be forwarded to the new host, then RFC2616
 * states that the end user must be asked to confirm it is ok to forward that
 * data.
 */
typedef struct PicselUserRequest_Redirect
{
    /**
     * The URL that caused the redirection, i.e. the page which was initially
     * requested.
     */
    const char *sourceUrl;
    /**
     * The URL being redirected to.
     */
    const char *destinationUrl;
    /**
     * Non-zero if the redirect is insecure. Redirecting from a secure
     * site to a non-secure site is a potential security risk and the device
     * user should always be made aware when such a redirect is occurring.
     */
    int         insecure;
    /**
     * HTTP status that caused the redirection (usually 30x, where x is 1 - 10)
     */
    int         httpStatus;
    /**
     * Whether or not the end user has confirmed that POST data should be
     * forwarded on.
     */
    int         postData;
    /**
     * The content type of the source URL, for example "text/html".
     */
    PicselContentType contentType;
}
PicselUserRequest_Redirect;

/* ******************** Confirm Unsecured Child Objects ********************* */

/**
 * @ingroup TgvBrowserEncryption
 *
 * The Picsel Library has noted that the currently loaded page is secure (for
 * example https://www.example.com), but that some of the items (images, for
 * example) to be loaded within the page are not secure (for example
 * http://example-two.com/images/header.png).
 *
 * The risk is that confidential information could be appended to the http
 * requests for the insecure items. For example:
 * http://example-two.com/images/header.png?username=picsel&password=developer
 *
 * The Alien Application, therefore, needs to tell the Picsel Library whether it
 * is ok to request the insecure objects. Picsel suggest that a prompt be
 * displayed to the device user asking them whether the objects should be
 * retrieved or not.
 */

/*
 * @var PicselUserRequest_Type_InsecureObjects
 *
 * Ask the user to confirm that insecure objects can be displayed on a
 * secure page.
 *
 * See @ref PicselUserRequest_InsecureObjects for a description of the
 * struct sent along with this type of request.
 */

/**
 * @ingroup TgvBrowserEncryption
 *
 * This request is made to confirm that it is okay to request the insecure
 * URL's.
 *
 * See @ref PicselUserRequest_Type_InsecureObjects
 */
typedef struct PicselUserRequest_InsecureObjects
{
    /**
     * The URL of the page containing the insecure URL's.
     */
    const char *parentUrl;  /**< [in]  Parent URL */
}
PicselUserRequest_InsecureObjects;

/* ***************************** File Selection ***************************** */

/**
 * @defgroup TgvBrowserFileOpen File Selection
 * @ingroup TgvBrowser
 *
 * The Picsel Library needs a filename to read/write.
 */

/**
 * @var PicselUserRequest_Type_ChooseFileName
 *
 * Ask the user to select a filename.
 *
 * See @ref PicselUserRequest_ChooseFileName for a description of the
 * struct sent along with this type of request.
 */

/**
 * @ingroup TgvBrowserFileOpen
 *
 * This is sent when the Picsel Library needs a filename to read/write from
 * the Alien Application.
 *
 * See @ref PicselUserRequest_ChooseFileName.
 */
typedef struct PicselUserRequest_ChooseFileName
{
    /**
     * [in]  A file name used to initialize the File Name edit control.
     */
    const char *defaultFileName;
    /** [out] File name is reset by the Alien when the user requires a file to
     * be selected, for example when activating a file upload form box in a
     * html document.
     */
    char       *fileName;
}
PicselUserRequest_ChooseFileName;

/**
 * @ingroup TgvBrowserFileOpen
 *
 * Response to PicselUserRequest_ValidateFileName.
 */
typedef enum PicselUserRequest_ValidateResponse
{
    PicselUserRequest_Validate_Accepted = (1<<16), /**< Allow this file to be
                                                        processed */
    PicselUserRequest_Validate_Rejected,           /**< Stop the operation */
    PicselUserRequest_Validate_Ignore              /**< Continue, but do not
                                                        process this file */
}
PicselUserRequest_ValidateResponse;

/**
 * @var PicselUserRequest_Type_ValidateFileName
 *
 * This will be sent to the Alien Application by the Picsel Library when
 * the user has selected a file name to carry out an operation on.
 *
 * See @ref PicselUserRequest_ValidateFileName for a description of the
 * struct sent along with this type of request.
 */

/**
 * @ingroup TgvBrowserFileOpen
 *
 * A filename has been supplied to the Picsel Library (fileName), and should
 * be validated.  The Alien Application should check that the file exists, and
 * that it can be opened.
 *
 * To indicate that the file is valid, the Alien Application should respond by
 * setting:
 *
 * @code PicselUserRequest_Request.result = PicselUserRequest_Result_Accepted;
 * @endcode
 *
 * The connection can be rejected by setting:
 *
 * @code PicselUserRequest_Request.result = PicselUserRequest_Result_Rejected;
 * @endcode
 *
*/
typedef struct PicselUserRequest_ValidateFileName
{
    const char                        *fileName; /**< [in]  The file name to
                                                      be validated */
    PicselUserRequest_ValidateResponse response; /**< [out] Response to the
                                                      validation */
}
PicselUserRequest_ValidateFileName;

/* *********************** Confirm Form AutoComplete ************************ */

/**
 * @defgroup TgvBrowserFormAutoComplete AutoComplete HTML Forms
 * @ingroup TgvBrowser
 *
 * Like many web browsers, the Picsel Library provides the ability to
 * automatically fill forms for the device user.
 *
 * It is not sensible to prompt for the device users preference at each and
 * every form encountered, so Picsel suggests that this form part of the device
 * policy with the user being offered the choice to set the policy once, but
 * with the ability to change the decision at a later date if desired.
 *
 * If this facility is enabled, then the Picsel Library will store information
 * the device user enters into each form, to be used in forms encountered in the
 * future.
 */

/**
 * @var PicselUserRequest_Type_FormAutoComplete
 *
 * Confirm whether it is ok to proceed with autocompletion of an HTML
 * form. Picsel recommends that the users response to this question is
 * stored by the Alien Application.  The answer can be  automatically
 * sent for subsequent requests of this type.
 *
 * See @ref PicselUserRequest_FormAutoComplete for a description of the
 * struct sent along with this type of request.
 */

/**
 * @ingroup TgvBrowserFormAutoComplete
 *
 * To indicate that the form should be pre-filled, and that any data entered by
 * the device user should be remembered , the Alien Application should respond
 * by setting:
 *
 * @code PicselUserRequest_Request.result = PicselUserRequest_Result_Accepted;
 * @endcode
 *
 * The option can be rejected by setting:
 *
 * @code PicselUserRequest_Request.result = PicselUserRequest_Result_Rejected;
 * @endcode
 */
typedef enum PicselUserRequest_FormAutoComplete_Type
{
    PicselUserRequest_FormAutoComplete_Fill = (1<<16),
    PicselUserRequest_FormAutoComplete_Save
}
PicselUserRequest_FormAutoComplete_Type;


/**
 * @ingroup TgvBrowserFormAutoComplete
 *
 * Forms part of @ref PicselUserRequest_Request.requestData, sent with
 * PicselUserRequest_notify().
 */
typedef struct PicselUserRequest_FormAutoComplete
{
    PicselUserRequest_FormAutoComplete_Type type;
}
PicselUserRequest_FormAutoComplete;


/* *************************** Download handling **************************** */

/**
 * @ingroup TgvBrowserFileOpen
 *
 * If the Picsel Library begins to download a document type that was registered
 * with PicselConfig_setString() / @ref PicselConfigBr_downloadableMimeTypes,
 * the Picsel Library will call AlienUserRequest_request() with a type of @ref
 * PicselUserRequest_Type_DownloadConfirm.
 *
 * The Alien Application should decide if it wants to handle the document, or if
 * the Picsel Library should handle it; the Picsel Library will wait for a
 * response before proceeding with the download.
 *
 * To tell the Picsel Library to stream the document to the Alien Application,
 * using @ref PicselUserRequest_DownloadData, set @ref
 * PicselUserRequest_DownloadConfirm.response to @ref
 * PicselUserRequest_DownloadConfirm_AlienHandled
 *
 * To allow the Picsel Library to handle the application, set @ref
 * PicselUserRequest_DownloadConfirm.response to @ref
 * PicselUserRequest_DownloadConfirm_PicselHandled.
 *
 * To cancel the download, set PicselUserRequest_DownloadConfirm.response to
 * @ref PicselUserRequest_DownloadConfirm_CancelDownload.
 *
 * When @ref PicselUserRequest_DownloadConfirm.response has been set appropriately,
 * send the @ref PicselUserRequest_DownloadConfirm struct to the Picsel Library
 * by calling PicselUserRequest_notify().
 */

/**
 * @ingroup TgvBrowserFileOpen
 *
 * Use one of these values as part of the @ref
 * PicselUserRequest_Request.requestData struct indicated by @ref
 * PicselUserRequest_Type_DownloadConfirm, to tell the Picsel Library how the document
 * should be handled.
 */
typedef enum PicselUserRequest_DownloadConfirm_Result
{
    /**
     * Tells the Picsel Library to handle the download internally, if it can.
     * If it cannot be handled, then the download will be cancelled.
     *
     * The Alien Application will receive a @ref
     * PicselUserRequest_Type_DownloadData event with status
     * @ref PicselUserRequest_DownloadData_TerminatedByAlien if the download
     * cannot be handled by the Picsel Library.
     */
    PicselUserRequest_DownloadConfirm_PicselHandled         = (1<<16),

    /**
     * Tells the Picsel Library that the downloaded document will be handled
     * by the Alien Application.
     */
    PicselUserRequest_DownloadConfirm_AlienHandled,

    /**
     * Cancels the download. You should expect to receive a @ref
     * PicselUserRequest_Type_DownloadData event with status
     * @ref PicselUserRequest_DownloadData_TerminatedByAlien after sending
     * this response.
     */
    PicselUserRequest_DownloadConfirm_CancelDownload
}
PicselUserRequest_DownloadConfirm_Result;

/**
 * @ingroup TgvBrowserFileOpen
 *
 * Used in the HTTP method field of @ref PicselUserRequest_DownloadConfirm to
 * inform the alien of the method by which the document is being received.
 */
typedef enum PicselUserRequest_Download_HttpMethod
{
    /** The document is being received as a HTTP POST request. */
    PicselUserRequest_Download_HttpPost = (1<<16),

    /** The document is being received as a HTTP GET request. */
    PicselUserRequest_Download_HttpGet
}
PicselUserRequest_Download_HttpMethod;

/**
 * @var PicselUserRequest_Type_DownloadConfirm
 *
 * Request guidance on how downloading of a document should be handled.
 * The Alien Application will receive this event only for documents with
 * MIME types which it has registered an interest in handling with
 * PicselConfig_setString() / PicselConfigBr_downloadableMimeTypes (if
 * you do not set this property, then you will not receive this event).
 *
 * The Alien Application can choose to have the document handled
 * internally by the Picsel Library (i.e. the default behaviour), to have the
 * document's content sent to your application, or to not have it
 * downloaded at all.  When you receive this event, the document will
 * already be in the process of being downloaded and so you should respond
 * to it promptly to allow the download to continue, in most cases not
 * providing a user prompt.
 *
 * See @ref PicselUserRequest_DownloadConfirm for a description of the
 * struct sent along with this type of request.
 */

/**
 * @ingroup TgvBrowserFileOpen
 *
 * The struct which will be received as part of an AlienUserRequest_Request()
 * event with type @ref PicselUserRequest_Type_DownloadConfirm.
 *
 * The Alien Application should take copies of any fields required to persist.
 * The only fields which may need to be altered are @ref response and, maybe,
 * @ref alienData.
 */
typedef struct PicselUserRequest_DownloadConfirm
{
    /** [in] A unique ID that identifies this request, and any subsequent
             @ref PicselUserRequest_DownloadData requests that follow. */
    Picsel_Download *downloadId;

    /** [in] The URL of the document being downloaded. */
    const char      *url;

    /** [in] The MIME type of the document being downloaded. */
    const char      *mime;

    /** [in] The size of the document in bytes, if known, or -1 if the size is
             unknown. */
    int              contentLength;

    /** [in] Flag whether this is embedded content. */
    int              embedded;

    /** [in] The data at the start of the downloaded file - may be NULL if there
             is no data. */
    const char      *buffer;

    /** [in] The size of the data buffer in bytes. */
    int              bufferLength;

    /** [in] The cookies that were sent with the document request that resulted
             in this document being downloaded.  All cookies are presented in
             a single string, not preceeded by Cookies: or followed by a
             newline.  This value may be NULL. */
    const char      *cookies;

    /** [in] The username that was used in authentication for the URL.  May be
             NULL. */
    const char      *username;

    /** [in] The password that was used in authentication for the URL.  May be
             NULL. */
    const char      *password;

    /** [in] The realm that was used in authentication for the URL.  May be
             NULL. */
    const char      *realm;

    /** [in] The challenge that was used in authentication for the URL.  May be
             NULL. */
    const char      *challenge;

    /** [in] The referer that was sent with the document request that resulted
             in this document being downloaded. */
    const char      *referer;

    /** [in] The HTTP status code from the last read of data from the
             document. */
    int              httpStatusCode;

    /** [in] The HTTP method through which the document is being received. */
    PicselUserRequest_Download_HttpMethod httpMethod;

    /** [out] The Alien Application should complete this field to tell the
     * Picsel Library how the download should be handled.
     */
    PicselUserRequest_DownloadConfirm_Result response;

    /**
     * [out] You may (but are not required to) set this to point at alien
     * data that you want associated with this download, such as a native
     * file handle.
     */
    void            *alienData;

}
PicselUserRequest_DownloadConfirm;


/**
 * @ingroup TgvBrowserFileOpen
 *
 * Used in the @ref PicselUserRequest_DownloadData::lastError "lastError" field
 * of @ref PicselUserRequest_DownloadData to indicate the last error to have
 * occurred.
 */
typedef enum PicselUserRequest_Download_Error
{
    /** No error has occurred (default). */
    PicselUserRequest_Download_NoError = 0,

    /** The download limit has been reached. */
    PicselUserRequest_Download_LimitReached = (1<<16),

    /** A TCP error has occurred. */
    PicselUserRequest_Download_TcpError,

    /** The download of the file has unexpectedly terminated. */
    PicselUserRequest_Download_UnexpectedTermination,

    /** An internal error has occurred. */
    PicselUserRequest_Download_InternalError
}
PicselUserRequest_Download_Error;

/**
 * @ingroup TgvBrowserFileOpen
 *
 * Forms part of a @ref PicselUserRequest_DownloadData struct, to describe what
 * state the download is in.
 */
typedef enum PicselUserRequest_DownloadData_Status
{
    /** The download is still in progress - expect further
        PicselUserRequest_DownloadData events for this download.  For any
        other status, do not expect any more events for this download. */
    PicselUserRequest_DownloadData_Progressing              = (1<<16),

    /** The download has now finished successfully (i.e. this was the last
        data packet). */
    PicselUserRequest_DownloadData_CompletedOk,

    /** An error has occurred during downloading and it cannot complete
        successfully. Further detail of the error may be found in the
        field 'lastError'. */
    PicselUserRequest_DownloadData_CompletedError,

    /** The download has been terminated because you have responded to
        a previous PicselUserRequest_DownloadData with
        PicselUserRequest_Result_Rejected. */
    PicselUserRequest_DownloadData_TerminatedByAlien
}
PicselUserRequest_DownloadData_Status;

/**
 * @var PicselUserRequest_Type_DownloadData
 *
 * If your Alien application chooses to have downloaded data sent to it,
 * then it will be sent in a series of these requests, each of which
 * offers you the opportunity to continue or cancel the download.
 *
 * It is not sensible to produce a user prompt for every event of this
 * type, but you may want to provide a progress dialogue that presents
 * an option to cancel the download.
 *
 * If the user does choose to cancel the download, the Alien Application can
 * respond to this event with PicselUserRequest_Result_Rejected to
 * terminate the download.
 *
 * <EM>Note:</EM> Cancelling the download will not automatically remove any data
 * which has already been retrieved.
 *
 * See @ref PicselUserRequest_DownloadData for a description of the
 * struct sent along with this type of request.
 */

/**
 * @ingroup TgvBrowserFileOpen
 *
 * After telling Picsel how to handle the download of a document, the Alien
 * Application will receive a series these structs in calls to
 * AlienUserRequest_request(), identified by the type @ref
 * PicselUserRequest_Type_DownloadData.
 *
 * To continue the download, The Alien Application should respond by setting:
 * @code PicselUserRequest_Request.result = PicselUserRequest_Result_Accepted; @endcode
 *
 * The download can be cancelled at any time by setting:
 *
 * @code PicselUserRequest_Request.result = PicselUserRequest_Result_Rejected; @endcode
 *
 * The Alien Application is responsible for clearing up any data that was
 * downloaded before @ref PicselUserRequest_Result_Rejected was set.
 */
typedef struct PicselUserRequest_DownloadData
{
    /**
     * [in] The same ID that was sent with the initial @ref
     * PicselUserRequest_DownloadConfirm request.
     */
    Picsel_Download *downloadId;

    /**
     * [in] The user data (if any) that you set in the initial
     *  PicselUserRequest_DownloadConfirm request.
     */
    void            *alienData;

    /**
     * [in] The data being downloaded - may be NULL if there is no data.
     */
    const char      *buffer;

    /**
     * [in] The size of the data in bytes.
     */
    int              bufferLength;

    /**
     * [in] The last error to have occurred while retrieving the data from the
     * document. An error will only be reported here in combination with a
     * status of @ref PicselUserRequest_DownloadData_CompletedError.
     */
    PicselUserRequest_Download_Error lastError;

    /** [in] The current status of the download. */
    PicselUserRequest_DownloadData_Status status;
}
PicselUserRequest_DownloadData;


/* ******************************* OpenFolder ******************************* */

/**
 * @var PicselUserRequest_Type_OpenFolder
 *
 * User has requested to open a folder on the device.
 *
 * See @ref PicselUserRequest_OpenFolder for a description of the
 * struct sent along with this type of request.
 */

/**
 * @ingroup TgvBrowserFileOpen
 *
 * Used in the response field of PicselUserRequest_OpenFolder to tell the
 * Picsel Library application whether the Alien Application or the Picsel
 * Library should open a folder.
 */
typedef enum PicselUserRequest_OpenFolder_Result
{
    PicselUserRequest_OpenFolder_Host       = (1<<16),
    PicselUserRequest_OpenFolder_Picsel
}
PicselUserRequest_OpenFolder_Result;

/**
 * @ingroup TgvBrowserFileOpen
 *
 * This is used for @ref PicselUserRequest_Type_OpenFolder
 */
typedef struct PicselUserRequest_OpenFolder
{
    /** [in] Unique folder identifier */
    unsigned long                           folder;
    /** [out] result of request - who should open the folder. */
    PicselUserRequest_OpenFolder_Result     result;
}
PicselUserRequest_FolderOpen;

/* **************************** SecretModeChange **************************** */
/**
 * @defgroup TgvSecretMode Secret Mode
 * @ingroup TgvDeviceServices
 *
 * Some devices have a "Secret Mode" which allows private information to be
 * stored in an area where a casual user (for instance someone who has
 * borrowed the device to make a call) cannot access it.
 *
 * If the Picsel Library calls AlienUserRequest_request() with type @ref
 * PicselUserRequest_Type_SecretModeChange, then the Alien Application might
 * prompt the device user for a code which, if accepted, will cause "Secret
 * Mode" to be toggled on or off, depending on the current state.
 *
 * If the device the Alien Application will be running on does not support
 * "Secret Mode", please ignore this section of the API.
 */

/**
 * @var PicselUserRequest_Type_SecretModeChange
 *
 * The Picsel Library requests to toggle secrecy mode. The Alien
 * Application should perform some sort of authentication (e.g. check a
 * PIN) and respond appropriately to the Picsel Library. If the response
 * is negative i.e authentication failed, then secrecy mode will remain
 * as it was.
 *
 * See @ref PicselUserRequest_SecretModeChange for a description of the
 * struct sent along with this type of request.
 */

/**
 * @ingroup TgvDeviceServices
 *
 * Used in the response field of PicselUserRequest_SecretMode to request that
 * the Alien Application changes SecretMode.
 *
 * Specifies whether to show or hide secret data.
 */
typedef enum PicselUserRequest_SecretMode
{
    PicselUserRequest_SecretMode_Hide    = (1<<16),
    PicselUserRequest_SecretMode_Show
}
PicselUserRequest_SecretMode;

/**
 *  This is sent as part of a request to change SecretMode.
 */
typedef struct PicselUserRequest_SecretModeChange
{
    /** [in] Secret mode setting requested         */
    PicselUserRequest_SecretMode    request;
    /** [out] 1 if successful, 0 if error occurred */
    int                             result;
}
PicselUserRequest_SecretModeChange;

/* *********************** CreateBookmark ******************************* */
/**
 * @ingroup TgvVisitedLinks
 *
 * Used in the response field of PicselUserRequest_CreateBookmark to
 * request that the Alien Application save a URL as a bookmark. This is sent
 * along with a @ref PicselUserRequest_Type_CreateBookmark type event.
 */
typedef struct PicselUserRequest_CreateBookmark
{
    /** [in] URL as a UTF-8 string. */
    const char*                     url;
    /** [in] A title for the bookmark, as a UTF-8 string */
    const char*                     title;
    /** [out] 1 if successful, 0 otherwise */
    int                             result;
}
PicselUserRequest_CreateBookmark;


/* *********************** Other Requests ******************************* */

/**
 * @var PicselUserRequest_Type_FileSaveContentLoss
 *
 * Warn the user that saving a file may result in some lost data.
 *
 * Lossless file saving requires the original document to be present. If
 * it is not available at the time of saving, the saved document may not
 * contain all the content of the original document. Show a query
 * dialogue box to let the user choose to continue saving the file with
 * possible content loss.
 *
 * See @ref PicselUserRequest_Data for a description of the struct sent
 * along with this request. In this instance, the struct need not contain
 * any information as it is only used to generate a notification to the
 * device user.
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !ALIEN_REQUEST_BROWSER_H */
