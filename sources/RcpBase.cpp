#include "RcpBase.h"

#include <algorithm>
#include <cstring>

#include <flext.h>

#include <rcp.h>

#include "version.h"

namespace rcp
{
    bool RcpBase::debugLogging = false;

    void RcpBase::postRabbitcontrolInit()
    {
        post("");
    #if FLEXT_SYS == FLEXT_SYS_MAX
        RcpBase::rabbitPost("RabbitControl for Max");
    #elif FLEXT_SYS == FLEXT_SYS_PD
        RcpBase::rabbitPost("RabbitControl for Pd");
    #else
        RcpBase::rabbitPost("RabbitControl");
    #endif
    }

    void RcpBase::postVersion()
    {
    #if FLEXT_SYS == FLEXT_SYS_MAX
        post("RCP Max version: %s", RCP_PD_VERSION);
    #elif FLEXT_SYS == FLEXT_SYS_PD
        post("RCP Pd version: %s", RCP_PD_VERSION);
    #else
        post("RCP Pd/Max version: %s", RCP_PD_VERSION);
    #endif

        post("RCP version: %s", RCP_VERSION);
    }

    void RcpBase::rabbitPost(const char* msg)
    {
        post("()()");
        (msg != NULL && strlen(msg) > 0 ) ? post(" oO    %s", msg) : post(" oO");
        post("  x");
    }

    void RcpBase::rabbitPostOneline(const char* msg)
    {
        (msg != NULL && strlen(msg) > 0 ) ? post("()()    %s", msg) : post("()()");
    }

} // namespace rcp
