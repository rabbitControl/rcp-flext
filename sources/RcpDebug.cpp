/*
********************************************************************
* rabbitcontrol - a protocol and data-format for remote control.
*
* https://rabbitcontrol.cc
* https://github.com/rabbitcontrol/rcp-flext
*
* This file is part of rabbitcontrol for Pd and Max.
*
* Written by Ingo Randolf, 2021 - 2022
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

#include "RcpDebug.h"

#include <rcp_memory.h>

#include "RcpBase.h"

#ifdef __cplusplus
extern "C"{
#endif

static int mallocCount = 0;
static int freeCount = 0;

void rcp_malloc_cb(void* /*ptr*/)
{
    mallocCount++;
}

void rcp_free_cb(void* /*ptr*/)
{
    freeCount++;
}

#ifdef __cplusplus
} // extern "C"
#endif

namespace rcp
{

    RcpDebug::RcpDebug()
    {
        AddInAnything("bang, debug, version, memory");
    }

    void RcpDebug::m_bang()
    {
        RcpBase::postRabbitcontrolInit();
        m_version();
    }

    void RcpDebug::m_debug(bool b)
    {
        RcpBase::debugLogging = b;
    }

    void RcpDebug::m_version()
    {
        RcpBase::postVersion();
    }

    void RcpDebug::m_memory()
    {
        post("malloc count: %d", mallocCount);
        post("free count: %d", freeCount);
    }

    FLEXT_LIB("rcp.debug", RcpDebug);
}
