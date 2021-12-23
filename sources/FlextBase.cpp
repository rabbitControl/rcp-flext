/*
********************************************************************
* rabbitcontrol - a protocol and data-format for remote control.
*
* https://rabbitcontrol.cc
* https://github.com/rabbitcontrol/rcp-flext
*
* This file is part of rabbitcontrol for Pd and Max.
*
* Written by Ingo Randolf, 2021
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, version 3.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
* OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
* HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*********************************************************************
*/

#include "FlextBase.h"

#include <algorithm>
#include <cstring>

#include <rcp.h>

#include "version.h"

#include "ParameterServer.h"
#include "ParameterClient.h"
#include "PdWebsocketServer.h"
#include "PdWebsocketClient.h"
#include "RcpDebug.h"
#include "RcpFormat.h"
#include "RcpParse.h"

#include "SizePrefixer.h"
#include "SPPParser.h"
#include "SlipDecoder.h"
#include "SlipEncoder.h"

#ifndef LIBRARY_NAME
#define LIBRARY_NAME rcp
#endif


namespace rcp
{

    bool FlextBase::debugLogging = false;

    static void rcp_main_setup()
    {
        FlextBase::postRabbitcontrolInit();

        // call the objects' setup routines
        FLEXT_SETUP(ParameterServer);
        FLEXT_SETUP(ParameterClient);
        FLEXT_SETUP(RcpDebug);
        FLEXT_SETUP(RcpFormat);
        FLEXT_SETUP(RcpParse);

        FLEXT_SETUP(PdWebsocketServer);
        FLEXT_SETUP(PdWebsocketClient);

        FLEXT_SETUP(SizePrefixer);
        FLEXT_SETUP(SPPParser);
        FLEXT_SETUP(SlipDecoder);
        FLEXT_SETUP(SlipEncoder);
    }

    void FlextBase::postRabbitcontrolInit()
    {
        post("");
    #if FLEXT_SYS == FLEXT_SYS_MAX
        FlextBase::rabbitPost("RabbitControl for Max");
    #elif FLEXT_SYS == FLEXT_SYS_PD
        FlextBase::rabbitPost("RabbitControl for Pd");
    #else
        FlextBase::rabbitPost("RabbitControl");
    #endif
    }

    void FlextBase::postVersion()
    {
        post("RCP Pd version: %s", RCP_PD_VERSION);
        post("RCP version: %s", RCP_VERSION);
    }

    void FlextBase::rabbitPost(const char* msg)
    {
        post("()()");
        (msg != NULL && strlen(msg) > 0 ) ? post(" oO    %s", msg) : post(" oO");
        post("  x");
    }

    void FlextBase::rabbitPostOneline(const char* msg)
    {
        (msg != NULL && strlen(msg) > 0 ) ? post("()()    %s", msg) : post("()()");
    }

    FLEXT_LIB_SETUP(LIBRARY_NAME, rcp::rcp_main_setup);

}

