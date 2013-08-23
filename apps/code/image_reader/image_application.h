#ifndef IMAGE_APPLICATION_H_
#define IMAGE_APPLICATION_H_

#include "image_utils.h"
#include "image_model.h"

using namespace vbf;

namespace image
{

class NotesView;
class NotesDocumentManager;
class ImageApplication: public QApplication
{
    Q_OBJECT
public:
    ImageApplication(int argc, char** argv);
    virtual ~ImageApplication();

public Q_SLOTS:
    bool open(const QString & path);
    bool close();
    bool suspend();

    void onWakeUp();
    void onCreateView(int type, MainWindow* main_window, QWidget*& result);
    void onAttachView(int type, QWidget* view, MainWindow* main_window);
    void onDeattachView(int type, QWidget* view, MainWindow* main_window);

    void onRotateScreen();
    void onScreenSizeChanged(int);

    void onUSBSignal(bool inserted);
    void onSDCardChangedSignal(bool inserted);
    void onMountTreeSignal(bool inserted, const QString &mount_point);
    void onConnectToPCSignal(bool connected);
    void onBatterySignal(const int, const int, bool);
    void onSystemIdleSignal();
    void onAboutToShutDown();

private:
    BaseView* activateNotes();
    BaseView* activateImage();

private:
    MainWindow      main_window_;  // main window
    ImageModel      model_;        // image model instance
    QString         path_;         // path of the document
    ApplicationType app_type_;     // type of the application: image or notes

    NO_COPY_AND_ASSIGN(ImageApplication);
};

class ImageApplicationAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT;
    Q_CLASSINFO("D-Bus Interface", "com.onyx.interface.images");

public:
    ImageApplicationAdaptor(ImageApplication *application)
        : QDBusAbstractAdaptor(application)
        , app_(application)
    {
        QDBusConnection::systemBus().
            registerService("com.onyx.service.images");
        QDBusConnection::systemBus().
            registerObject("/com/onyx/object/images", app_);
    }

public Q_SLOTS:
    bool open(const QString & path) { return app_->open(path); }
    bool close(const QString & path) { return app_->close(); }

private:
    ImageApplication *app_;
    NO_COPY_AND_ASSIGN(ImageApplicationAdaptor);
};


};
#endif
