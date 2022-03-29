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

#include "WebsocketServerImpl.h"

namespace rcp
{

    WebsocketServerImpl::WebsocketServerImpl(IWebsocketServerListener* listener)
        : websocketServer()
        , m_listener(listener)
    {
    }

    size_t WebsocketServerImpl::connections() const
    {
        return m_connections.size();
    }

    void WebsocketServerImpl::send(char* data, size_t size)
    {
        // send to all connected clients
        for (auto& conn : m_connections)
        {
            m_server.send(conn, data, size, websocketpp::frame::opcode::value::binary);
        }
    }

    // websocketServer
    void WebsocketServerImpl::connected(void* client)
    {
        if (m_listener)
        {
            m_listener->connected(client);
        }
    }

    void WebsocketServerImpl::disconnected(void* client)
    {
        if (m_listener)
        {
            m_listener->disconnected(client);
        }
    }

    void WebsocketServerImpl::received(char* data, size_t size, void* id)
    {
        if (m_listener)
        {
            m_listener->received(data, size, id);
        }
    }

    void WebsocketServerImpl::socketerror(const char* reason)
    {
        if (m_listener)
        {
            m_listener->socketerror(reason);
        }
    }

}

