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

#ifndef PDWEBSOCKETCLIENT_H
#define PDWEBSOCKETCLIENT_H

#include <flext.h>

#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 401)
#error You need at least flext version 0.4.1
#endif

#include "WebsocketClientImpl.h"

namespace rcp
{

    class PdWebsocketClient : public flext_base, public IWebsocketClientListener
    {
        FLEXT_HEADER_S(PdWebsocketClient, flext_base, setup)

    public:
        PdWebsocketClient();

    public:
        // websocketClient
        void connected() override;
        void disconnected() override;
        void received(char* data, size_t size) override;
        void received(const std::string& msg) override;

    protected:
        static void setup(t_classid c)
        {
            FLEXT_CADDMETHOD_(c, 0, "open", m_open);
            FLEXT_CADDMETHOD_(c, 0, "close", m_close);
        }

        void m_list(int argc, t_atom* argv);
        void m_open(const t_symbol *d);
        void m_close();


    private:
        FLEXT_CALLBACK_V(m_list)
        FLEXT_CALLBACK_S(m_open)
        FLEXT_CALLBACK(m_close)

    private:
        std::shared_ptr<WebsocketClientImpl> m_client;
    };

}

#endif // PDWEBSOCKETCLIENT_H
