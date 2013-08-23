#include "onyx/cms/notes_manager.h"

namespace cms
{


NoteInfo::NoteInfo()
{
}

NoteInfo::NoteInfo(const NoteInfo & right)
: name_(right.name())
, image_(right.thumbnail())
{
}

NoteInfo::~NoteInfo()
{
}

NoteInfo & NoteInfo::operator = (const NoteInfo & right)
{
    if (this != &right)
    {
        name_ = right.name();
        image_ = right.thumbnail();
    }
    return *this;
}

void NoteInfo::thumbnail(QByteArray & ba) const
{
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    image_.save(&buffer, "PNG");
}

NotesManager::NotesManager(void)
{
}

NotesManager::~NotesManager(void)
{
}

QString NotesManager::suggestedName(QSqlDatabase & database)
{
    const QString NOTE_NAME = "scribble_";
    const QString curr_time = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh:mm");
    int curr_time_len = curr_time.length();
    int max = 1;
    Notes notes;
    all(database, notes);
    foreach(NoteInfo n, notes)
    {
        if (n.name().startsWith(NOTE_NAME))
        {
            QString str = n.name();
            str = str.remove(0, NOTE_NAME.length());
            str = str.remove(str.length() - curr_time_len - 1, curr_time_len + 1);
            int value = str.toInt();
            if (max <= value)
            {
                max = value + 1;
            }
        }
    }

    QString name("%1%2%3%4");
    name = name.arg(NOTE_NAME).arg(max).arg("_").arg(curr_time);
    return name;
}

bool NotesManager::makeSureTableExist(QSqlDatabase &database)
{
    QSqlQuery query(database);
    return query.exec( "create table if not exists notes ("
                       "id integer primary key, "
                       "name text, "
                       "thumbnail blob)");

    // May also need to create a index.
}

bool NotesManager::removeTable(QSqlDatabase &database)
{
    QSqlQuery query(database);
    return query.exec( "drop table notes" );
}

int  NotesManager::all(QSqlDatabase &database, Notes &notes)
{
    QSqlQuery query(database);
    query.prepare( "select name, thumbnail from notes ");

    if (!query.exec())
    {
        return false;
    }

    NoteInfo note;
    while (query.next())
    {
        note.mutable_name() = query.value(0).toString();
        note.mutable_thumbnail().loadFromData(query.value(1).toByteArray());
        notes.push_back(note);
    }
    return true;
}

/// Add a new note index or replace existing note index.
bool NotesManager::addIndex(QSqlDatabase &database,
                            const NoteInfo & note)
{
    // Remove the existing note if any.
    removeIndex(database, note.name());

    QSqlQuery query(database);
    query.prepare( "INSERT OR REPLACE into notes (name, thumbnail) values(?, ?)");
    query.addBindValue(note.name());

    QByteArray ba;
    note.thumbnail(ba);
    query.addBindValue(ba);
    return query.exec();
}

bool NotesManager::removeIndex(QSqlDatabase &database,
                               const QString & name)
{
    QSqlQuery query(database);
    query.prepare( "delete from notes where name = ?");
    query.addBindValue(name);
    return query.exec();
}

bool NotesManager::removeAll(QSqlDatabase &database)
{
    QSqlQuery query(database);
    query.prepare( "delete from notes");
    if (!query.exec())
    {
        qDebug() << query.lastError().text();
        return false;
    }
    return true;
}

}

