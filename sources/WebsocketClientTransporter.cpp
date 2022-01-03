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

#include "WebsocketClientTransporter.h"

#include <rcp_memory.h>
#include <rcp_logging.h>

namespace rcp
{

    WebsocketClientTransporter::WebsocketClientTransporter(IWebsocketClientListener* listener)
        : websocketClient()
        , m_transporter(nullptr)
        , m_listener(listener)
    {
        m_transporter = (pd_websocket_client_transporter*)RCP_CALLOC(1, sizeof (pd_websocket_client_transporter));

        if (m_transporter)
        {
            m_transporter->pdST = this;
            rcp_client_transporter_setup(RCP_TRANSPORTER(m_transporter),
                                     pd_websocket_client_transporter_send);
        }
    }

    WebsocketClientTransporter::~WebsocketClientTransporter()
    {
        if (m_transporter)
        {
            RCP_FREE(m_transporter);
            m_transporter = nullptr;
        }
    }


    // implement IClientTransporter
    rcp_client_transporter* WebsocketClientTransporter::transporter() const
    {
        if (m_transporter)
        {
            return &m_transporter->transporter;
        }

        return nullptr;
    }

    void WebsocketClientTransporter::open(const std::string& address)
    {
        websocketClient::connect(address);
    }

    void WebsocketClientTransporter::close()
    {
        websocketClient::disconnect();
    }

    // websocketClient
    void WebsocketClientTransporter::connected()
    {
        if (m_transporter)
        {
            rcp_client_transporter_call_connected_cb(RCP_TRANSPORTER(m_transporter));
        }

        if (m_listener)
        {
            m_listener->connected();
        }
    }

    void WebsocketClientTransporter::disconnected()
    {
        if (m_transporter)
        {
            // NOTE: this re-creates the client manager
            rcp_client_transporter_call_disconnected_cb(RCP_TRANSPORTER(m_transporter));
        }

        if (m_listener)
        {
            m_listener->disconnected();
        }
    }

    void WebsocketClientTransporter::received(char* data, size_t size)
    {
        if (m_transporter)
        {
            rcp_client_transporter_call_recv_cb(RCP_TRANSPORTER(m_transporter), data, size);
        }
    }

} // namespace rcp


//
void pd_websocket_client_transporter_send(rcp_client_transporter* transporter, char* data, size_t size)
{
	if (transporter)
	{
		((pd_websocket_client_transporter*)transporter)->pdST->send(data, size);
	}
}
