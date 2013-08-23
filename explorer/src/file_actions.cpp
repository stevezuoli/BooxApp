#include "file_actions.h"

namespace ui
{

FileActions::FileActions()
: BaseActions()
{
    category()->setIcon(QIcon(QPixmap(":/images/edit.png")));
}

FileActions::~FileActions(void)
{
}

void FileActions::generateActions(FileActionTypes types)
{
    category()->setText(QApplication::tr("File"));
    actions_.clear();

    // Adjust the order if necessary.
    const FileActionType part1[] =
    {
        FILE_SEARCH, FILE_RENAME, FILE_CREATE_SHORTCUT,
        FILE_CLEAR_ALL_NOTES, FILE_DRM_RETURNABLE, FILE_CLEAR_ALL_WRITEPADS
    };
    int size = sizeof(part1)/sizeof(part1[0]);
    for(int i = 0; i < size; ++i)
    {
        if (types.testFlag(part1[i]))
        {
            shared_ptr<QAction> action(new QAction(exclusiveGroup()));
            switch (part1[i])
            {
            case FILE_DELETE:
                action->setCheckable(true);
                action->setText(QApplication::tr("Delete"));
                action->setIcon(QIcon(QPixmap(":/images/delete.png")));
                action->setData(FILE_DELETE);
                actions_.push_back(action);
                break;

            case FILE_SEARCH:
                action->setCheckable(true);
                action->setText(QApplication::tr("Search"));
                action->setIcon(QIcon(QPixmap(":/images/file_search.png")));
                action->setData(FILE_SEARCH);
                actions_.push_back(action);
                break;

            case FILE_RENAME:
                action->setCheckable(true);
                action->setText(QApplication::tr("Rename"));
                action->setIcon(QIcon(QPixmap(":/images/file_search.png")));
                action->setData(FILE_RENAME);
                actions_.push_back(action);
                break;

            case FILE_CREATE_SHORTCUT:
                action->setCheckable(true);
                action->setText(QApplication::tr("Create shortcut"));
                action->setIcon(QIcon(QPixmap(":/images/create_shortcut.png")));
                action->setData(FILE_CREATE_SHORTCUT);
                actions_.push_back(action);
                break;

            case FILE_CLEAR_ALL_NOTES:
                action->setCheckable(true);
                action->setText(QApplication::tr("Clear All Notes"));
                action->setIcon(QIcon(QPixmap(":/images/clear_all_notes.png")));
                action->setData(FILE_CLEAR_ALL_NOTES);
                actions_.push_back(action);
                break;

            case FILE_CLEAR_ALL_WRITEPADS:
                action->setCheckable(true);
                action->setText(QApplication::tr("Clear All Notes"));
                action->setIcon(QIcon(QPixmap(":/images/clear_all_notes.png")));
                action->setData(FILE_CLEAR_ALL_WRITEPADS);
                actions_.push_back(action);
                break;

            case FILE_DRM_RETURNABLE:
                action->setCheckable(true);
                action->setText(QApplication::tr("Return Book"));
                action->setIcon(QIcon(QPixmap(":/images/loan_return.png")));
                action->setData(FILE_DRM_RETURNABLE);
                actions_.push_back(action);
                break;

            default:
                break;
            }
        }
    } 
    // separator.
    shared_ptr<QAction> separator(new QAction(exclusiveGroup()));
    separator->setSeparator(true);
    actions_.push_back(separator);

    // Adjust the order if necessary.
    const FileActionType part2[] =
    {
        FILE_COPY,FILE_CUT, FILE_PASTE, FILE_PASTE_TO_DIR,FILE_DELETE
    };
    size = sizeof(part2)/sizeof(part2[0]);
    for(int i = 0; i < size; ++i)
    {
        if (types.testFlag(part2[i]))
        {
            shared_ptr<QAction> action(new QAction(exclusiveGroup()));
            switch (part2[i])
            {
            case FILE_CUT:
                action->setCheckable(true);
                action->setText(QApplication::tr("Cut"));
                action->setIcon(QIcon(QPixmap(":/images/file_cut.png")));
                action->setData(FILE_CUT);
                actions_.push_back(action);
                break;
            case FILE_COPY:
                action->setCheckable(true);
                action->setText(QApplication::tr("Copy"));
                action->setIcon(QIcon(QPixmap(":/images/file_copy.png")));
                action->setData(FILE_COPY);
                actions_.push_back(action);
                break;
            case FILE_PASTE:
                action->setCheckable(true);
                action->setText(QApplication::tr("Paste"));
                action->setIcon(QIcon(QPixmap(":/images/file_paste.png")));
                action->setData(FILE_PASTE);
                actions_.push_back(action);
                break;
            case FILE_PASTE_TO_DIR:
                action->setCheckable(true);
                action->setText(QApplication::tr("Paste to Selected"));
                action->setIcon(QIcon(QPixmap(":/images/file_paste_to_dir.png")));
                action->setData(FILE_PASTE_TO_DIR);
                actions_.push_back(action);
                break;
            case FILE_DELETE:
                action->setCheckable(true);
                action->setText(QApplication::tr("Delete"));
                action->setIcon(QIcon(QPixmap(":/images/delete.png")));
                action->setData(FILE_DELETE);
                actions_.push_back(action);
                break;

            default:
                break;
            }
        }
    } 
}

QAction * FileActions::action(const FileActionType type)
{
    for(int i = 0; i < static_cast<int>(actions_.size()); ++i)
    {
        if (actions_.at(i)->data().toInt() == type)
        {
            return actions_.at(i).get();
        }
    }
    return 0;
}

FileActionType FileActions::selected()
{
    // Search for the changed actions.
    QAction * act = exclusiveGroup()->checkedAction();
    if (act)
    {
        return static_cast<FileActionType>(act->data().toInt());
    }
    return INVALID_FILE_ACTION;
}


}
