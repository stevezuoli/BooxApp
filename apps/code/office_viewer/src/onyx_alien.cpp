#include "assert.h"
#include "picsel-language.h"
#include "picsel-locale.h"
#include "picsel-encoding.h"

#include "onyx_alien.h"
#include "onyx_office.h"
#include "onyx/sys/sys_utils.h"

#include <QtCore/QDebug>


static int memoryThreshold()
{
    static int threshold = 0;
    if (threshold <= 0)
    {
        threshold = qgetenv("OFFICE_MEMORY_THRESHOLD").toInt();
    }
    if (threshold <= 0)
    {
        threshold = 512 * 1024;
    }
    return threshold;
}

namespace onyx {

// Forward all messages to onyx::OfficeReader
OnyxAlien::OnyxAlien() {
}

OnyxAlien::~OnyxAlien() {
}

void OnyxAlien::reportFatalError() {
    qDebug ("Fatal error happend.");
}

void OnyxAlien::reportError (PicselError error, PicselErrorData *errorData) {
    switch (error) {
        case PicselDocumentError_AgentMatchFailed:
            qDebug() << "Alien error:  AgentMatchFailed";
            break;
        case PicselDocumentError_DocumentTranslationFailed:
            qDebug() << "Alien error:  DocumentTranslationFailed";
            break;
        case PicselDocumentError_DocumentPasswordProtected:
            qDebug() << "Alien error:  DocumentPasswordProtected";
            break;
        case PicselDocumentError_InternalError:
            qDebug() << "Alien error:  InternalError";
            break;
        case PicselError_OutOfMemory:
            qDebug() << "Alien error:  OutOfMemory";
            break;
        default:
            qDebug() << "Alien error:  Unknown";
            break;
    }
}

void OnyxAlien::reportPageChanged (int current, int total) {
    OfficeReader::instance().onPageChanged (current, total);
}

void OnyxAlien::onInsufficientMemory()
{
   OfficeReader::instance().onInsufficientMemory();
}

void OnyxAlien::updateImage (void               *buffer,
                             PicselScreenFormat  format,
                             unsigned int        width,
                             unsigned int        height,
                             unsigned int        widthBytes,
                             unsigned int        updateX,
                             unsigned int        updateY,
                             unsigned int        updateWidth,
                             unsigned int        updateHeight,
                             unsigned int        xTopLeft,
                             unsigned int        yTopLeft) {
    //format is not necessary
    OfficeReader::instance().updateImage (buffer, width, height, widthBytes,
                                          updateX, updateY, updateWidth, updateHeight, xTopLeft, yTopLeft);
}

void OnyxAlien::informationEvent (AlienInformation_Event  event, void  *data) {
    OfficeReader::instance().informationEvent (event, data);
}

void OnyxAlien::userRequest (void *data) {
    OfficeReader::instance().userRequest (data);
}

void OnyxAlien::timerRequest (unsigned long  *reference, unsigned long ms) {
    OfficeReader::instance().timerRequest (reference, ms);
}

void OnyxAlien::cancelTimer (long unsigned int* reference) {
    OfficeReader::instance().cancelTimer (reference);
}

void OnyxAlien::configReady() {
    OfficeReader::instance().configReady();
}

void OnyxAlien::screenConfiguration (void *d) {
    OfficeReader::instance().screenConfiguration (d);
}

Picsel_Context* OnyxAlien::getPicselContext() {
    return (Picsel_Context*)OfficeReader::instance().backendContext();
}

};   // namespace onyx

static onyx::OnyxAlien onyx_alien;


void AlienError_fatal (Alien_Context *alienContext) {
    onyx_alien.reportFatalError();
}

void AlienError_error (Alien_Context *alienContext, PicselError error, PicselErrorData *errorData) {
    onyx_alien.reportError (error, errorData);
}

void AlienConfig_ready (Alien_Context *ac) {
    onyx_alien.configReady();
}

void AlienUserRequest_request (Alien_Context *alienContext,
                               PicselUserRequest_Request  *request) {
    onyx_alien.userRequest (request);
}

void AlienTimer_request (Alien_Context  *alienContext, unsigned long  *reference, unsigned long   ms) {
    onyx_alien.timerRequest (reference, ms);
}

int AlienTimer_cancel (Alien_Context *ac, unsigned long *reference) {
    onyx_alien.cancelTimer (reference);
    return 1;
}

void *AlienMemory_malloc (size_t *size) {
    size_t available = sys::systemFreeMemory();
    if (*size > available || available <= static_cast<size_t>(memoryThreshold()))
    {
        qWarning("Out of memory.");
        onyx_alien.onInsufficientMemory();
        *size = 0;
        return 0;
    }
    return malloc(*size);
}

void AlienMemory_free (void *mem) {
    free (mem);
}

void AlienMemory_freeStack (void *p) {
    free (p);
}

void AlienDebug_output (const char *string) {
    printf ("%s\n", string);
}

void AlienScreen_pagesChanged (Alien_Context *alienContext, int currentPage, int numPages) {
    onyx_alien.reportPageChanged (currentPage, numPages);
}

void AlienScreen_getConfiguration (Alien_Context            *ac,
                                   AlienScreenConfiguration *config) {
    onyx_alien.screenConfiguration (config);
}

void AlienScreen_update (Alien_Context      *ac,
                         void               *buffer,
                         PicselScreenFormat  format,
                         unsigned int        width,
                         unsigned int        height,
                         unsigned int        widthBytes,
                         unsigned int        updateX,
                         unsigned int        updateY,
                         unsigned int        updateWidth,
                         unsigned int        updateHeight,
                         unsigned int        xTopLeft,
                         unsigned int        yTopLeft) {
    onyx_alien.updateImage (buffer, format, width, height, widthBytes,
                            updateX, updateY, updateWidth, updateHeight,
                            xTopLeft, yTopLeft);
}

void AlienEvent_information (Alien_Context *ac, AlienInformation_Event  event, void  *data) {
    onyx_alien.informationEvent (event, data);
}

Picsel_Context *AlienMemory_getPicselContext (void) {
    return  onyx_alien.getPicselContext();
}
