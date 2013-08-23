#ifndef FILE_ACTIONS_H_
#define FILE_ACTIONS_H_

#include "onyx/base/base.h"
#include "onyx/ui/context_dialog_base.h"

namespace ui
{

enum FileActionType
{
    INVALID_FILE_ACTION     = 0x0,
    FILE_CUT                = 0x1,
    FILE_COPY               = 0x2,
    FILE_PASTE              = 0x4,
    FILE_DELETE             = 0x8,
    FILE_SEARCH             = 0x10,
    FILE_CREATE_SHORTCUT    = 0x20,
    FILE_CLEAR_ALL_NOTES    = 0x40,
    FILE_DRM_RETURNABLE     = 0x80,
    FILE_CLEAR_ALL_WRITEPADS= 0x100,
    FILE_RENAME             = 0x200,
    FILE_PASTE_TO_DIR        = 0x400,
};

Q_DECLARE_FLAGS(FileActionTypes, FileActionType)
Q_DECLARE_OPERATORS_FOR_FLAGS(FileActionTypes)

class FileActions : public BaseActions
{
public:
    FileActions(void);
    ~FileActions(void);

public:
    /// Generate or re-generate the edit actions.
    void generateActions(FileActionTypes types = FILE_DELETE|FILE_SEARCH|FILE_CREATE_SHORTCUT);
    QAction * action(const FileActionType type);
    FileActionType selected();

};

}

#endif //  FILE_ACTIONS_H_
