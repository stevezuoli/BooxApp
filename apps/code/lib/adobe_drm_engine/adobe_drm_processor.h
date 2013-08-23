#ifndef ADOBE_DRM_PROCESSOR_H_
#define ADOBE_DRM_PROCESSOR_H_

#include "adobe_drm_utils.h"

namespace adobe_drm
{

class AdobeDRMHandler;
class AdobeDRMProcessorClientPrivate;
class AdobeDRMProcessorClient
{
public:
    AdobeDRMProcessorClient(AdobeDRMHandler *host);
    ~AdobeDRMProcessorClient();
    AdobeDRMProcessorClientPrivate * data();

private:
    shared_ptr<AdobeDRMProcessorClientPrivate> data_;
};

};
#endif
