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

#include "RabbitholeServerTransporter.h"

#include <cstdlib>

#include <rcp_memory.h>

#include "ParameterServer.h"


namespace rcp
{

    static void timerCb(void* userdata)
    {
        if (userdata != NULL)
        {
            RabbitHoleServerTransporter* rhl_transporter = static_cast<RabbitHoleServerTransporter*>(userdata);
            rhl_transporter->tryConnectTimerTimeout();
        }
    }

    RabbitHoleServerTransporter::RabbitHoleServerTransporter(rcp_server* server)
        : websocketClient()
        , m_rcpServer(server)
        , m_transporter(nullptr)
        , m_connectInterval(2)
    {
        m_transporter = (pd_rabbithole_server_transporter*)RCP_CALLOC(1, sizeof (pd_rabbithole_server_transporter));

        if (m_transporter)
        {
            m_transporter->pdST = this;

            rcp_server_transporter_setup(RCP_TRANSPORTER(m_transporter),
                                     pd_rabbithole_server_transporter_sendToOne,
                                     pd_rabbithole_server_transporter_sendToAll);

            rcp_server_add_transporter(m_rcpServer, RCP_TRANSPORTER(m_transporter));
        }


        m_tryConnectTimer.SetCallback(timerCb);
    }

    RabbitHoleServerTransporter::~RabbitHoleServerTransporter()
    {
        // unbind
        m_doTryConnect = false;
        m_tryConnectTimer.Reset();

        if (m_transporter)
        {
            rcp_server_remove_transporter(m_rcpServer, RCP_TRANSPORTER(m_transporter));

            RCP_FREE(m_transporter);
            m_transporter = nullptr;
        }
    }

    //-------------------------
    // IPdServerTransporter
    rcp_server_transporter* RabbitHoleServerTransporter::transporter() const
    {
        if (m_transporter)
        {
            return &m_transporter->transporter;
        }

        return nullptr;
    }

    void RabbitHoleServerTransporter::bind(uint16_t /*port*/)
    {
    }

    void RabbitHoleServerTransporter::unbind()
    {
        m_doTryConnect = false;

        m_tryConnectTimer.Reset();

        disconnect();
    }

    //-------------------------
    // websocketClient
    void RabbitHoleServerTransporter::connected()
    {
    }

    void RabbitHoleServerTransporter::disconnected()
    {
        if (m_doTryConnect)
        {
            tryConnect();
        }
    }

    void RabbitHoleServerTransporter::received(char* data, size_t size)
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
                                                    NULL);
            }
        }
    }


    // websocketClient
    void RabbitHoleServerTransporter::connect(const std::string& uri, const std::string& subprotocol)
    {
        m_uri = uri;

        if (m_uri.find("https", 0) == 0)
        {
            m_uri = m_uri.replace(0, 5, "wss");
        }
        else if (m_uri.find("http", 0) == 0)
        {
            m_uri = m_uri.replace(0, 4, "ws");
        }

        if (m_uri.find("ws", 0) == 0)
        {
            m_doTryConnect = true;
            websocketClient::connect(m_uri, subprotocol);
        }
    }

    void RabbitHoleServerTransporter::setInterval(const int i)
    {;
        bool start_after = m_connectInterval > 0;

        m_connectInterval = i;

        if (start_after)
        {
            tryConnect();
        }

        if (m_connectInterval <= 0)
        {
            disconnect();
        }
    }

    // timer function
    void RabbitHoleServerTransporter::tryConnectTimerTimeout()
    {
        if (!m_uri.empty())
        {
            websocketClient::connect(m_uri);
        }
    }

    void RabbitHoleServerTransporter::tryConnect()
    {
        if (m_connectInterval > 0
                && !m_uri.empty())
        {
            m_tryConnectTimer.Delay(m_connectInterval, this);
        }
    }

} // namespace rcp


void pd_rabbithole_server_transporter_sendToOne(rcp_server_transporter* transporter, char* data, size_t data_size, void* /*id*/)
{
	if (transporter)
	{
		((pd_rabbithole_server_transporter*)transporter)->pdST->send(data, data_size);
	}
}

void pd_rabbithole_server_transporter_sendToAll(rcp_server_transporter* transporter, char* data, size_t data_size, void* /*excludeId*/)
{
	if (transporter)
	{
		((pd_rabbithole_server_transporter*)transporter)->pdST->send(data, data_size);
	}
}
