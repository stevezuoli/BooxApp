#include "onyx/ui/languages.h"
#include "text_application.h"

namespace text_editor
{

TextApplication::TextApplication(int &argc, char **argv)
    : QApplication(argc, argv)
{
    ui::loadTranslator(QLocale::system().name());
    frame_.reset(new TextFrame(this));
    if (argc > 1)
    {
        current_path_ = QString::fromLocal8Bit(argv[1]);
    }
}

TextApplication::~TextApplication(void)
{
    close( current_path_ );
}

bool TextApplication::open(const QString &path_name)
{
    frame_->load(path_name);
    frame_->show();
    return true;
}

bool TextApplication::close(const QString &path_name)
{
    return false;
}

bool TextApplication::isOpened()
{
    return true;
}

bool TextApplication::errorFound()
{
    return false;
}

bool TextApplication::suspend()
{
    return true;
}

void TextApplication::onWakeUp()
{
}

void TextApplication::onUSBSignal(bool inserted)
{
}

void TextApplication::onSDChangedSignal(bool inserted)
{
}

void TextApplication::onMountTreeSignal(bool inserted, const QString &mount_point)
{
}

void TextApplication::onConnectToPCSignal(bool connected)
{
    qDebug("\n\nPC %s\n\n", connected ? "connected" : "disconnected");
    if ( connected )
    {
        qApp->exit();
    }
}

void TextApplication::onBatterySignal(const int, const int, bool)
{
}

void TextApplication::onSystemIdleSignal()
{
}

void TextApplication::onAboutToShutDown()
{
}

void TextApplication::onRotateScreen()
{
}

void TextApplication::onScreenSizeChanged(int)
{
}

}
