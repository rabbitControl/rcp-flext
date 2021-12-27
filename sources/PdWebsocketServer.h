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

#ifndef PDWEBSOCKETSERVER_H
#define PDWEBSOCKETSERVER_H

#include <flext.h>

#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 401)
#error You need at least flext version 0.4.1
#endif

#include "WebsocketServerImpl.h"

namespace rcp
{

    class PdWebsocketServer : public flext_base, public IWebsocketServerListener
    {
        FLEXT_HEADER_S(PdWebsocketServer, flext_base, setup)

    public:
        PdWebsocketServer(int argc, t_atom *argv);

    public:
        // IWebsocketServerListener
        void connected(void* client) override;
        void disconnected(void* client) override;
        void received(char* data, size_t size, void* id) override;
        void socketerror(const char* reason) override;

    protected:
        static void setup(t_classid c)
        {
            FLEXT_CADDMETHOD_(c, 0, "listen", m_listen);
        }

        void m_list(int argc, t_atom* argv);
        void m_listen(int& port);

    private:
        FLEXT_CALLBACK_V(m_list)
        FLEXT_CALLBACK_I(m_listen)

    private:
        std::shared_ptr<WebsocketServerImpl> m_server;

    };

}

#endif // PDWEBSOCKETSERVER_H