#ifndef NODE_TYPES_H
#define NODE_TYPES_H

namespace explorer {

namespace model {

/// Define node type. Used by node type identification.
enum NodeType
{
    NODE_TYPE_NULL = -1,        ///< Null node type.
    NODE_TYPE_ROOT,             ///< The unique root node.
    NODE_TYPE_RECENT_DOCS,      ///< Recent document list.
    NODE_TYPE_SHORTCUTS,        ///< All shortcuts.
    NODE_TYPE_NOTE_CONTAINER,   ///< All notes.
    NODE_TYPE_NEW_NOTE,         ///< Create a new note.
    NODE_TYPE_NOTE,             ///< Edit an existing note.
    NODE_TYPE_WRITEPAD_CONTAINER,   ///< Write pad.
    NODE_TYPE_NEW_WRITEPAD,     ///< Create a new write pad.
    NODE_TYPE_WRITEPAD,         ///< Write pad.
    NODE_TYPE_SYS_SETTINGS,     ///< Device system settings.
    NODE_TYPE_APPLICATIONS,     ///< applicatins as calendar, clock
    NODE_TYPE_EXTERNAL_STORAGE, ///< External storage.
    NODE_TYPE_LIBRARY,          ///< Library node.
    NODE_TYPE_DICTIONARY,       ///< Dictionaries.
    NODE_TYPE_MUSICS,           ///< All music items parent node.
    NODE_TYPE_IMAGES,           ///< All image items parent node.
    NODE_TYPE_USB,              ///< USB device node. Multiple instances.
    NODE_TYPE_CF,               ///< CF device node.
    NODE_TYPE_SD,               ///< SD device node.
    NODE_TYPE_DOWNLOAD,         ///< Download directory.
    NODE_TYPE_LOCALE,           ///< Locale setting.
    NODE_TYPE_DATE,             ///< Date setting.
    NODE_TYPE_CALENDAR,         ///< Calendar show.
    NODE_TYPE_FULL_SCREEN_CLOCK,            ///< Clock show.
    NODE_TYPE_TIMEZONE,         ///< Time zone setting.
    NODE_TYPE_PM,               ///< Power management setting.
    NODE_TYPE_SCREEN_CALIBRATION,   ///< Screen calibration.
    NODE_TYPE_FORMAT_FLASH,     ///< Format internal flash.
    NODE_TYPE_FORMAT_SD,        ///< Format sd card.
    NODE_TYPE_ABOUT,            ///< About information.
    NODE_TYPE_NETWORK_PROFILE,  ///< Network profile.
    NODE_TYPE_DIRECTORY,        ///< Directory.
    NODE_TYPE_FILE,             ///< File content or unknown file format.
    NODE_TYPE_VFS,              ///< Virtual file system.
    NODE_TYPE_USER_MANUAL,      ///< User manual.
    NODE_TYPE_WEBSITES,         ///< All web sites.
    NODE_TYPE_WEBSITE,          ///< A single web site.
    NODE_TYPE_ONLINESHOPS,      ///< All online shops.
    NODE_TYPE_ONLINESHOP,       ///< A single online shop.
    NODE_TYPE_3G_CONNECTION,    ///< Setup 3G connection.
    NODE_TYPE_SSH_SERVER,       ///< Start ssh server for our partners and developers.
    NODE_TYPE_WAVEFORM_SETTINGS,///< Change the screen grayscale.
    NODE_TYPE_REMOVE_ACCOUNT_INFO,   ///< Remove all of the account information.
    NODE_TYPE_GAMES,            ///< Game container.
    NODE_TYPE_GAME_SUDOKU,      ///< Game.
    NODE_TYPE_FEEDREADER,       ///< Feed reader.
    NODE_TYPE_FONTMANAGEMENT,   ///< Font management
    NODE_TYPE_FILETYPE_SETTINGS,///< filet type settings
    NODE_TYPE_APPLICATION,      ///< Application.
    NODE_TYPE_STARTUP,          ///< Startup setting.
    NODE_TYPE_SCREEN_UPATE_SETTING, ///< Screen update setting.
    NODE_TYPE_COUNT             ///< The last type, do not use it.
    
};

/// Define content types. Make sure it does not conflict with
/// node type.
enum ContentType
{
    CONTENT_START = NODE_TYPE_COUNT,
    CONTENT_BMP,
    CONTENT_CHM,
    CONTENT_DIRECTORY,
    CONTENT_EPUB,
    CONTENT_GIF,
    CONTENT_HTML,
    CONTENT_JPEG,
    CONTENT_MOBI,
    CONTENT_PDB,
    CONTENT_PDF,
    CONTENT_PNG,
    CONTENT_RTF,
    CONTENT_TIFF,
    CONTENT_TEXT,
    CONTENT_FB2,
    CONTENT_ZIP,
    CONTENT_MUSIC,
    CONTENT_DJVU,
    CONTENT_ABF,
    CONTENT_DOC,
    CONTENT_XLS,
    CONTENT_PPT,
    CONTENT_RUNNABLE,
    CONTENT_UNKNOWN,
    CONTENT_END
};

enum Field
{
    NONE = 0,                ///< Do not sort.
    NAME,
    TITLE,
    DESCRIPTION,
    SIZE,
    RATING,
    LAST_ACCESS_TIME,
    READ_COUNT,
    CREATE_TIME,
    NODE_TYPE,
};

enum SortOrder
{
    NO_ORDER = 0,
    ASCENDING,
    DESCENDING
};


}  // namespace model

}  // namespace explorer

#endif

