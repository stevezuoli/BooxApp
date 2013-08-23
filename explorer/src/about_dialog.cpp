#include "onyx/screen/screen_proxy.h"
#include "onyx/sys/sys.h"
#include "about_dialog.h"
#include "private_conf/conf.h"

namespace ui
{

const QString LABEL_STYLE = "           \
QLabel                                  \
{                                       \
     margin: 5px;                       \
     background: transparent;           \
     font: 19px ;                       \
     color: black;                      \
     border-width: 0px;                 \
     border-color: transparent;         \
}";

const QString WIDGET_STYLE = "          \
QWidget                                 \
{                                       \
    background: #ffffff;                \
    font: 24px ;                        \
    border-width: 1px;                  \
    border-color: black;                \
    border-style: solid;                \
    border-radius: 5;                   \
    padding: 10px;                      \
}";

struct AboutEntry
{
    QString image;
    QString text;
};

AboutEntry entries[] =
{
    {":/images/onyx_logo.png",
    "<p align=justify>The mission of Onyx is to bring exceptional reading experience to the consumers, "
    "to enable new business models for the publishers, and to revolutionize the publishing industry, "
    "by creating state-of-the-art e-readers and distribution platform with the newest display and "
    "communication technologies.</p>"
    "Firmware Version:     %1<br>"
    "Device serial number: %2<br>"
    "Waveform information: %3<br>"
    "Binary fingerprint:   %4<br>"
    "<p></p>"
    "CPU:                  %5<br>"
    "Memory:               %6<br>"
    "Flash:                %7<br>"
    },

    {":/images/adobe_logo.png", "<p>Boox 60 contains Adobe&reg; Reader&reg; Mobile software under license "
    "from Adobe Systems Incorporated, Copyright &copy; 1995-2009 Adobe Systems Incorporated."
    "All rights reserved. Adobe and Reader are trademarks of Adobe Systems Incorporated.</p>"},

    {":/images/onyx_logo.png", "<Center><B>Open-source projects used in the BOOX platform</B></Center><br>"
    "<ul>"
    "<li>RedBoot is a complete bootstrap environment for embedded systems.</li>"
    "<li>Linux kernel is an operating system kernel used by the Linux family of Unix-like operating systems. "
    "It is one of the most prominent examples of free and open source software.</li>"
    "<li>BusyBox combines tiny versions of many common UNIX utilities into a single small executable. </li>"
    "<li>AES Crypt is a file encryption software product available on several operating systems that uses the industry standard Advanced Encryption Standard (AES) to easily and securely encrypt files.</li>"
    "<li>The usbutils package contains a utility used to display information about USB buses in the system and the devices connected to them.</li>"
    "<li>mtdutils provides a generic Linux subsystem for memory devices, especially Flash devices.</li>"
    "<li>dosfstools: Utilities to create and check MS-DOS FAT filesystems Inside of this package there are two utilities to create and to check MS-DOS FAT filesystems on either harddisks or floppies under Linux.</li>"
    "<li>USB_ModeSwitch is (surprise!) a mode switching tool for controlling \"flip flop\" (multiple device) USB gear.</li>"
    "<li>Udev provides a dynamic /dev directory, and hooks userspace into kernel device events.</li>"
    "</ul>"
    },

    {":/images/onyx_logo.png", "<Center><B>Open-source projects used in the BOOX platform</B></Center><br>"
    "<ul>"
    "<li>glibc The GNU C library is used as the C library in the GNU system and most systems with the Linux kernel.</li>"
    "<li>Qt is a cross-platform application and UI framework.</li>"
    "<li>libiconv is for you if your application needs to support multiple character encodings, but that support lacks from your system.</li>"
    "<li>libusb aims to create a library for use by user level applications to access USB devices regardless of OS.</li>"
    "<li>libpng is the official PNG reference library.</li>"
    "<li>zlib is designed to be a free, general-purpose, legally unencumbered -- that is, not covered by any patents -- lossless data-compression library for use on virtually any computer hardware and operating system.</li>"
    "<li>libdbus is a message bus system, a simple way for applications to talk to one another.</li>"
    "<li>libexapt is an XML parser library written in C.</li>"
    "</ul>"
    },

    {":/images/onyx_logo.png", "<Center><B>Open-source projects used in the BOOX platform</B></Center><br>"
    "<ul>"
    "<li>Freetype provides Free, High-Quality, and Portable Font Engine</li>"
    "<li>FriBidi is an implementation of the Unicode Bidirectional Algorithm (bidi).</li>"
    "<li>libcurl A library for transferring files with URL syntax, supporting FTP, FTPS, HTTP, HTTPS, SCP, SFTP, TFTP, TELNET, DICT, LDAP, LDAPS and FILE.</li>"
    "<li>tslib is an abstraction layer for touchscreen panel events, as well as a filter stack for the manipulation of those events.</li>"
    "<li>OpenSSL is a collaborative effort to develop a robust, commercial-grade, full-featured, and Open Source toolkit implementing the Secure Sockets Layer (SSL v2/v3) and Transport Layer Security (TLS v1) protocols as well as a full-strength general purpose cryptography library.</li>"
    "<li>SQLite is a software library that implements a self-contained, serverless, zero-configuration, transactional SQL database engine.</li>"
    "<li>wpa_supplicant is a WPA Supplicant for Linux, BSD, Mac OS X, and Windows with support for WPA and WPA2 (IEEE 802.11i / RSN).</li>"
    "<li>DjVuLibre is an open source (GPL'ed) implementation of DjVu, including viewers, browser plugins, decoders, simple encoders, and utilities.</li>"
    "</ul>"
    },


};
static const int ENTRY_COUNT = sizeof(entries) / sizeof(entries[0]);


AboutItem::AboutItem(QWidget *parent)
: QWidget(parent)
, layout_(this)
, image_(0)
, text_(0)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    createLayout();
}

AboutItem::~AboutItem()
{
}

void AboutItem::createLayout()
{
    QRect desk = qApp->desktop()->screenGeometry();

    layout_.setContentsMargins(0, 0, 0, 0);
    layout_.addWidget(&image_, desk.height() / 7);
    layout_.addWidget(&text_, desk.height() * 6 / 7);

    image_.setStyleSheet(LABEL_STYLE);
    image_.setAlignment(static_cast<Qt::Alignment>(Qt::AlignCenter|Qt::AlignTop));

    text_.setStyleSheet(LABEL_STYLE);
    text_.setTextFormat(Qt::RichText);
    text_.setAlignment(static_cast<Qt::Alignment>(Qt::AlignLeft|Qt::AlignTop));
    text_.setWordWrap(true);
}

void AboutItem::setInformation(const QString & image_path,
                               const QString & text)
{
    image_.setPixmap(QPixmap(image_path));
    text_.setText(text);
}

AboutDialog::AboutDialog(QWidget *parent)
    : OnyxDialog(parent)
    , ver_layout_(&content_widget_)
    , item_(0)
    , status_bar_(0, ui::PROGRESS|MESSAGE|BATTERY)
    , page_number_(1)
{
    updateEntries();

    setModal(true);
    createLayout();
}

AboutDialog::~AboutDialog(void)
{
}

void AboutDialog::updateEntries()
{
    int index = 0;
    sys::SystemConfig c;
    sys::PrivateConfig pc;
    entries[index].text =
        entries[index].text.arg(c.version()).arg(c.deviceId()).arg(pc.waveformInfo()).arg(pc.binaryFingerprint()).arg(c.cpuInfo() ).arg(c.memInfo()).arg(c.flashInfo());
}

int AboutDialog::exec()
{
    shadows_.show(true);
    showMaximized();

    onyx::screen::instance().flush();
    onyx::screen::instance().updateWidgetRegion(0, outbounding(parentWidget()), onyx::screen::ScreenProxy::GC, false, onyx::screen::ScreenCommand::WAIT_ALL);
    return QDialog::exec();
}

void AboutDialog::keyPressEvent(QKeyEvent *ke)
{
    ke->accept();
}

void AboutDialog::keyReleaseEvent(QKeyEvent *ke)
{
    // Check the current selected type.
    ke->accept();
    switch (ke->key())
    {
    case Qt::Key_Up:
        focusPreviousChild();
        break;
    case Qt::Key_Down:
        focusNextChild();
        break;
    case Qt::Key_Return:
        break;
    case Qt::Key_Escape:
        reject();
        break;
    case Qt::Key_Right:
    case Qt::Key_PageDown:
        nextPage();
        break;
    case Qt::Key_Left:
    case Qt::Key_PageUp:
        prevPage();
        break;
    }
}

void AboutDialog::createLayout()
{
    // Retrieve the values from system status.
    updateTitle(QApplication::tr("About"));
    updateTitleIcon(QPixmap(":/images/about_dialog_icon.png"));

    content_widget_.setBackgroundRole(QPalette::Base);

    // The big layout.
    ver_layout_.setContentsMargins(0, 0, 0, 0);

    // Content.
    ver_layout_.addWidget(&item_);
    item_.setStyleSheet(WIDGET_STYLE);
    ver_layout_.addSpacing(0);

    // Status bar.
    ver_layout_.addWidget(&status_bar_);
    status_bar_.setProgress(page_number_, ENTRY_COUNT);

    // Show current page.
    updatePage();

    // Setup connection.
    connect(&status_bar_,
            SIGNAL(progressClicked(const int, const int)),
            this,
            SLOT(onStatusItemClicked(const int, const int)));

}

void AboutDialog::onStatusItemClicked(const int percentage, const int value)
{
    page_number_ = value;
    updatePage();
}

void AboutDialog::updatePage()
{
    if (page_number_ >= 1 && page_number_ <= ENTRY_COUNT)
    {
        int index = page_number_ - 1;
        status_bar_.setProgress(page_number_, ENTRY_COUNT);
        item_.setInformation(entries[index].image, entries[index].text);
    }
}

bool AboutDialog::prevPage()
{
    if (page_number_ > 1)
    {
        --page_number_;
        updatePage();
        onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GC);
        return true;
    }
    return false;
}

bool AboutDialog::nextPage()
{
    if (page_number_ < ENTRY_COUNT)
    {
        ++page_number_;
        updatePage();
        onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GC);
        return true;
    }
    return false;
}


bool AboutDialog::event(QEvent* qe)
{
    bool ret = OnyxDialog::event(qe);
    if (qe->type() == QEvent::UpdateRequest)
    {
        static int count = 0;
        qDebug("update count %d", count++);
        onyx::screen::instance().sync(&shadows_.hor_shadow());
        onyx::screen::instance().sync(&shadows_.ver_shadow());
        onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::GC);
    }
    return ret;
}


}
