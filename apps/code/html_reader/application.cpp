#include "onyx/data/configuration.h"
#include "application.h"
#include "onyx/ui/font_family_actions.h"

using namespace ui;
namespace reader
{


ReaderApplication::ReaderApplication(int &argc, char **argv)
    : QApplication(argc, argv)
    , main_window_(0)
{
    initTheme();
    loadExternalFonts();
}

ReaderApplication::~ReaderApplication(void)
{
}

/// Initialize RenderThemeQt for webkit Qt backend. See
/// 3rdparty\webkit\WebCore\platform\qt\RenderThemeQt.cpp for details.
void ReaderApplication::initTheme()
{
    // We change the active selection background color so that
    // all viewers have the same look and feel.
    // Have to use the 254 as alpha value, as the webkit itself does blend
    // with white color. When using 16 grey level color, the result is same
    // as complete black.
    QPalette p(QApplication::palette());
    QBrush b(QColor(0, 0, 0, 254));
    p.setBrush(QPalette::Active, QPalette::Highlight, b);
    p.setBrush(QPalette::Inactive, QPalette::Highlight, b);

    b.setColor(QColor(255, 255, 255, 254));
    p.setBrush(QPalette::Active, QPalette::HighlightedText, b);
    p.setBrush(QPalette::Inactive, QPalette::HighlightedText, b);

    QApplication::setPalette(p);
}

void ReaderApplication::loadExternalFonts()
{
    // Before we open the document, make sure the external fonts
    // have been installed. It's necessary as user may use external
    // fonts, but by default, these fonts are not loaded.
    FontFamilyActions::loadExternalFonts();
}

bool ReaderApplication::open(const QString & path_name)
{
    main_window_.resize(qApp->desktop()->screenGeometry().size());
    main_window_.showMaximized();
    return main_window_.open(path_name);
}

bool ReaderApplication::close(const QString & path)
{
    main_window_.close();
    return true;
}

void ReaderApplication::suspend()
{
    main_window_.onAboutToSuspend();
}

} // namespace reader


