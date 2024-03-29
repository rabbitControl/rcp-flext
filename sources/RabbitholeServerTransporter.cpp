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


// callbacks
static void _pd_rabbithole_server_transporter_sendToOne(rcp_server_transporter* transporter, char* data, size_t data_size, void* /*id*/)
{
	if (transporter &&
             transporter->user)
	{
		((rcp::RabbitHoleServerTransporter*)transporter->user)->send(data, data_size);
	}
}

static void _pd_rabbithole_server_transporter_sendToAll(rcp_server_transporter* transporter, char* data, size_t data_size, void* /*excludeId*/)
{
	if (transporter &&
            transporter->user)
	{
		((rcp::RabbitHoleServerTransporter*)transporter->user)->send(data, data_size);
	}
}


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
        m_transporter = (rcp_server_transporter*)RCP_CALLOC(1, sizeof(rcp_server_transporter));

        if (m_transporter)
        {
            rcp_server_transporter_setup(m_transporter,
                                         _pd_rabbithole_server_transporter_sendToOne,
                                         _pd_rabbithole_server_transporter_sendToAll);

            rcp_server_add_transporter(m_rcpServer, m_transporter);

            m_transporter->user = this;
        }

        m_tryConnectTimer.SetCallback(timerCb);
    }

    RabbitHoleServerTransporter::~RabbitHoleServerTransporter()
    {
        m_doTryConnect = false;
        m_oneTimeError = false;
        m_tryConnectTimer.Reset();

        if (m_transporter)
        {
            rcp_server_remove_transporter(m_rcpServer, m_transporter);

            RCP_FREE(m_transporter);
            m_transporter = nullptr;
        }
    }

    //-------------------------
    // IPdServerTransporter
    rcp_server_transporter* RabbitHoleServerTransporter::transporter() const
    {
        return m_transporter;
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
        m_oneTimeError = true;
    }

    void RabbitHoleServerTransporter::failed(uint16_t code)
    {
        /*
         rabbithole response codes:
         400: BAD_REQUEST: no tunnel name
         412: PRECONDITION_FAILED: tunnel name too short
         423: LOCKED: tunnel name already taken (another server is already connected)
        */

        if (code != 200 &&
                m_oneTimeError)
        {
            m_oneTimeError = false;
            switch (code)
            {
            case 400:
                error("Rabbithole: no tunnel name provided");
                break;
            case 412:
                error("Rabbithole: tunnel name too short");
                break;
            case 423:
                error("Rabbithole: tunnel already in use - please use a different public tunnel or consider using a private tunnel.");
                break;
            default:
                std::cout << "unhandled fail code: " << code << std::endl;
                break;
            }
        }

        if (m_doTryConnect)
        {
            tryConnect();
        }
    }

    void RabbitHoleServerTransporter::disconnected(uint16_t code)
    {
        /*
         rabbithole close codes:
         4500: SESSION_NOT_RELIABLE: public tunnel are considered not reliable - sessions close after a certain time
        */

        if (code == 4500 &&
                m_uri.find("/public/rcpserver/connect") != std::string::npos)
        {
            post("Rabbithole: public tunnel closed - please reconnect your client or use a private tunnel");
        }

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
            if (m_transporter->received)
            {
                m_transporter->received(m_transporter->server,
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
            m_oneTimeError = true;
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
