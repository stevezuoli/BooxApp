#ifndef ONYX_ALIEN_H_
#define ONYX_ALIEN_H_

#include "alien-types.h"
#include "alien-context.h"
#include "alien-event.h"
#include "alien-error.h"
#include "alien-notify.h"
#include "alien-pointer.h"
#include "alien-screen.h"
#include "alien-request.h"
#include "alien-timer.h"
#include "alien-filesys-data.h"
#include "alien-memory.h"
#include "alien-debug.h"
#include "alien-config.h"
#include "preferences.h"
#include "touchscreen.h"
#include "picsel-entrypoint.h"
#include "picsel-config.h"
#include "picsel-config-fileviewer.h"
#include "picsel-agent.h"
#include "picsel-pixelblock.h"
#include "picsel-fileviewer.h"
#include "picsel-pointer.h"
#include "picsel-flowmode.h"
#include "picsel-debug.h"
#include "linux-alien.h"
#include "linux-alien-config-fv.h"
#include "picsel-version.h"
#include "picsel-focus.h"
#include "picsel-app.h"
#include "picsel-version.h"
#include "picsel-language.h"


/// Dummy alien for Linux platform using Qt backend.
/// Not necessary to store any data here, we can access
/// global instance directly.
struct AlienLinuxMain {
};

namespace onyx {

class OnyxAlien {

public:
    OnyxAlien();
    ~OnyxAlien();

public:
    void reportFatalError();
    void reportError (PicselError error, PicselErrorData *errorData);
    void reportPageChanged (int current, int total);
    void updateImage (void *, PicselScreenFormat ,
                      unsigned int        width,
                      unsigned int        height,
                      unsigned int        widthBytes,
                      unsigned int        updateX,
                      unsigned int        updateY,
                      unsigned int        updateWidth,
                      unsigned int        updateHeight,
                      unsigned int        xTopLeft,
                      unsigned int        yTopLeft);

    void onInsufficientMemory();
    void informationEvent (AlienInformation_Event event, void  *data);
    void userRequest (void *data);
    void timerRequest (unsigned long  *reference, unsigned long ms);
    void cancelTimer (unsigned long  *reference);
    void configReady();
    void screenConfiguration (void *d);
    Picsel_Context* getPicselContext();
};

};      // namespace onyx

#endif  // ONYX_ALIEN_H_
