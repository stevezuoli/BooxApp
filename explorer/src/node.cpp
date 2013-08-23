#include "node.h"
#include <QApplication>


namespace explorer {

namespace model {

const size_t Node::INVALID_ORDER = 0xffffffff;
const QString Node::DATE_FORMAT  = "yyyy-MM-dd hh:mm";

Node::Node(Node*p)
        : parent_(p)
{
}

Node::~Node()
{
}


QString nodeDisplayName(NodeType type)
{
    switch (type)
    {
    case NODE_TYPE_LIBRARY:
        return QApplication::tr("Library");
    case NODE_TYPE_SD:
        return QApplication::tr("SD Card");
    case NODE_TYPE_DICTIONARY:
        return QApplication::tr("Dictionary");
    case NODE_TYPE_RECENT_DOCS:
        return QApplication::tr("Recent Documents");
    case NODE_TYPE_SHORTCUTS:
        return QApplication::tr("Shortcuts");
    case NODE_TYPE_WRITEPAD_CONTAINER:
        return QApplication::tr("Notes");
    case NODE_TYPE_NEW_WRITEPAD:
        return QApplication::tr("Create Note");
    case NODE_TYPE_WRITEPAD:
        return QApplication::tr("Create Note");
    case NODE_TYPE_SYS_SETTINGS:
        return QApplication::tr("Settings");
    case NODE_TYPE_APPLICATIONS:
        return QApplication::tr("Applications");
    case NODE_TYPE_LOCALE:
        return QApplication::tr("Locale");
    case NODE_TYPE_FONTMANAGEMENT:
        return QApplication::tr("Default Font");
    case NODE_TYPE_DATE:
        return QApplication::tr("Date");
    case NODE_TYPE_CALENDAR:
        return QApplication::tr("Calendar");
    case NODE_TYPE_FULL_SCREEN_CLOCK:
        return QApplication::tr("Clock");
    case NODE_TYPE_PM:
        return QApplication::tr("Power Management");
    case NODE_TYPE_ABOUT:
        return QApplication::tr("About");
    case NODE_TYPE_SSH_SERVER:
        return QApplication::tr("SSH Server");
    case NODE_TYPE_SCREEN_CALIBRATION:
        return QApplication::tr("Stylus Calibration");
    case NODE_TYPE_WEBSITES:
        return QApplication::tr("Web Sites");
    case NODE_TYPE_ONLINESHOPS:
        return QApplication::tr("Online shops");
    case NODE_TYPE_3G_CONNECTION:
        return QApplication::tr("3G Connection");
    case NODE_TYPE_FORMAT_FLASH:
        return QApplication::tr("Format Flash");
    case NODE_TYPE_FORMAT_SD:
        return QApplication::tr("Format SD Card");
    case NODE_TYPE_USER_MANUAL:
        return QApplication::tr("User Manual");
    case NODE_TYPE_TIMEZONE:
        return QApplication::tr("Time Zone");
    case NODE_TYPE_WAVEFORM_SETTINGS:
        return QApplication::tr("Color Settings");
    case NODE_TYPE_NOTE_CONTAINER:
        return QApplication::tr("Scribble");
    case NODE_TYPE_NOTE:
        return QApplication::tr("Scribble");
    case NODE_TYPE_NEW_NOTE:
        return QApplication::tr("Create Scribble");
    case NODE_TYPE_FEEDREADER:
        return QApplication::tr("Feed Reader");
    case NODE_TYPE_GAMES:
        return QApplication::tr("Games");
    case NODE_TYPE_GAME_SUDOKU:
        return QApplication::tr("Sudoku");
    case NODE_TYPE_FILETYPE_SETTINGS:
        return QApplication::tr("Preferred Applications");
    case NODE_TYPE_APPLICATION:
        return QApplication::tr("VCOM");
    case NODE_TYPE_STARTUP:
        return QApplication::tr("Startup Setting");
    case NODE_TYPE_SCREEN_UPATE_SETTING:
            return QApplication::tr("Screen Update");
    }
    return QString();
}

QString nodeName(NodeType type)
{
    switch (type)
    {
    case NODE_TYPE_LIBRARY:
        return "Library";
    case NODE_TYPE_SD:
        return "SD Card";
    case NODE_TYPE_DICTIONARY:
        return "Dictionary";
    case NODE_TYPE_RECENT_DOCS:
        return "Recent Documents";
    case NODE_TYPE_SHORTCUTS:
        return "Shortcuts";
    case NODE_TYPE_WRITEPAD_CONTAINER:
        return "notes";
    case NODE_TYPE_WRITEPAD:
        return "create_note";
    case NODE_TYPE_SYS_SETTINGS:
        return "Settings";
    case NODE_TYPE_APPLICATIONS:
        return "Applications";
    case NODE_TYPE_LOCALE:
        return "Locale";
    case NODE_TYPE_FONTMANAGEMENT:
        return "Default Font";
    case NODE_TYPE_DATE:
        return "Date";
    case NODE_TYPE_CALENDAR:
        return "Calendar";    
    case NODE_TYPE_FULL_SCREEN_CLOCK:
        return "Clock"; 
    case NODE_TYPE_PM:
        return "Power Management";
    case NODE_TYPE_ABOUT:
        return "About";
    case NODE_TYPE_SSH_SERVER:
        return "SSH Server";
    case NODE_TYPE_SCREEN_CALIBRATION:
        return "Stylus Calibration";
    case NODE_TYPE_WEBSITES:
        return "Web Sites";
    case NODE_TYPE_ONLINESHOPS:
        return "Online shops";
    case NODE_TYPE_3G_CONNECTION:
        return "3G Connection";
    case NODE_TYPE_FORMAT_FLASH:
        return "Format Flash";
    case NODE_TYPE_FORMAT_SD:
        return "Format SD Card";
    case NODE_TYPE_USER_MANUAL:
        return "User Manual";
    case NODE_TYPE_TIMEZONE:
        return "Time Zone";
    case NODE_TYPE_WAVEFORM_SETTINGS:
        return "Color Settings";
    case NODE_TYPE_NOTE_CONTAINER:
        return "Scribble";
    case NODE_TYPE_NOTE:
        return "Edit scribble";
    case NODE_TYPE_NEW_NOTE:
        return "Create Scribble";
    case NODE_TYPE_FEEDREADER:
        return "feed";
    case NODE_TYPE_GAMES:
        return "Games";
    case NODE_TYPE_GAME_SUDOKU:
        return "sudoku";
    case NODE_TYPE_APPLICATION:
        return "application";
    case NODE_TYPE_STARTUP:
        return "Startup Setting";
    case NODE_TYPE_SCREEN_UPATE_SETTING:
        return "Screen Update";
    }
    return QString();
}


}  // namespace model

}  // namespace explorer
