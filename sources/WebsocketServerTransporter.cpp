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

#include "WebsocketServerTransporter.h"

#include <rcp_memory.h>

#include "ParameterServer.h"

namespace rcp
{
    WebsocketServerTransporter::WebsocketServerTransporter(rcp_server* server, IWebsocketServerListener* listener)
        : websocketServer()
        , m_rcpServer(server)
        , m_transporter(nullptr)
        , m_listener(listener)
    {
        m_transporter = (pd_websocket_server_transporter*)RCP_CALLOC(1, sizeof (pd_websocket_server_transporter));

        if (m_transporter)
        {
            m_transporter->pdST = this;

            rcp_server_transporter_setup(RCP_TRANSPORTER(m_transporter),
                                     pd_websocket_server_transporter_sendToOne,
                                     pd_websocket_server_transporter_sendToAll);

            rcp_server_add_transporter(m_rcpServer, RCP_TRANSPORTER(m_transporter));
        }
    }

    WebsocketServerTransporter::~WebsocketServerTransporter()
    {
        if (m_transporter)
        {
            // remove transporter from rcp_Server
            rcp_server_remove_transporter(m_rcpServer, RCP_TRANSPORTER(m_transporter));

            RCP_FREE(m_transporter);
            m_transporter = nullptr;
        }
    }


    // IPdServerTransporter
    rcp_server_transporter* WebsocketServerTransporter::transporter() const
    {
        if (m_transporter)
        {
            return &m_transporter->transporter;
        }

        return nullptr;
    }

    void WebsocketServerTransporter::bind(uint16_t port)
    {
        run(port);
    }

    void WebsocketServerTransporter::unbind()
    {
        stop();
    }

    uint16_t WebsocketServerTransporter::port() const
    {
        return websocketServer::port();
    }

    bool WebsocketServerTransporter::isListening() const
    {
        return m_server.is_listening();
    }


    // wesocketpp
    void WebsocketServerTransporter::connected(void* client)
    {
        if (m_listener)
        {
            m_listener->connected(client);
        }
    }

    void WebsocketServerTransporter::disconnected(void* client)
    {
        if (m_listener)
        {
            m_listener->disconnected(client);
        }
    }

    void WebsocketServerTransporter::received(char* data, size_t size, void* client)
    {
        if (m_transporter &&
                data  &&
                size > 0)
        {
            if (m_transporter->transporter.received)
            {
                m_transporter->transporter.received(m_transporter->transporter.server,
                                                    data,
                                                    size,
                                                    client);
            }
        }
    }

    void WebsocketServerTransporter::socketerror(const char* reason)
    {
        error("websocketserver(%d): %s", websocketServer::port(), reason);
    }

    void WebsocketServerTransporter::sendToOne(char* data, size_t size, void* id)
    {
        if (!m_server.is_listening()) {
            return;
        }

        if (id == nullptr) {
            return;
        }

        for (auto& conn : m_connections)
        {
            if (auto p = conn.lock())
            {
                if (id == p.get())
                {
                    m_server.send(conn, data, size, websocketpp::frame::opcode::value::binary);
                }
            }
            else
            {
                // TODO: handle this?
            }
        }
    }

    void WebsocketServerTransporter::sendToAll(char* data, size_t size, void* excludeId)
    {
        if (!m_server.is_listening()) {
            return;
        }

        for (auto& conn : m_connections)
        {
            if (auto p = conn.lock())
            {
                if (excludeId == p.get())
                {
                    continue;
                }
                m_server.send(conn, data, size, websocketpp::frame::opcode::value::binary);
            }
        }
    }

} // namespace rcp


//
void pd_websocket_server_transporter_sendToOne(rcp_server_transporter* transporter, char* data, size_t data_size, void* id)
{
	if (transporter)
	{
		((pd_websocket_server_transporter*)transporter)->pdST->sendToOne(data, data_size, id);
	}
}

void pd_websocket_server_transporter_sendToAll(rcp_server_transporter* transporter, char* data, size_t data_size, void* excludeId)
{
	if (transporter)
	{
		((pd_websocket_server_transporter*)transporter)->pdST->sendToAll(data, data_size, excludeId);
	}
}
