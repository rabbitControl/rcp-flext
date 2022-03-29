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

#ifndef RABBITHOLESERVERTRANSPORTER_H
#define RABBITHOLESERVERTRANSPORTER_H

#include <flext.h>

#include <rcp_server_transporter.h>

#include "IServerTransporter.h"
#include "websocketClient.h"

typedef struct _pd_rabbithole_server_transporter pd_rabbithole_server_transporter;

namespace rcp
{
    class RabbitHoleServerTransporter
            : public IServerTransporter
            , public websocketClient
    {

    public:
        RabbitHoleServerTransporter(rcp_server* server);
        ~RabbitHoleServerTransporter();

        void setInterval(const int i);
        int interval() const { return m_connectInterval; }
        void tryConnectTimerTimeout();
        std::string uri() const { return m_uri; }

    public:
        // IServerTransporter
        rcp_server_transporter* transporter() const override;
        void bind(uint16_t port) override;
        void unbind() override;
        uint16_t port() const override { return 0; }
        bool isListening() const override { return true; }

    public:
        // websocketClient
        void connected() override;
        void disconnected() override;
        void received(char* data, size_t size) override;
        void received(const std::string& /*msg*/) override {}

        // websocketClient overrides
        void connect(const std::string& uri, const std::string& subprotocol = "") override;


    private:
        void tryConnect();

        rcp_server* m_rcpServer;
        pd_rabbithole_server_transporter* m_transporter;
        std::string m_uri;

        flext::Timer m_tryConnectTimer;
        int m_connectInterval;
        std::atomic<bool> m_doTryConnect{false};
    };

}

#ifdef __cplusplus
extern "C"{
#endif


typedef struct _pd_rabbithole_server_transporter {
    rcp_server_transporter transporter;
	rcp::RabbitHoleServerTransporter* pdST;
} pd_rabbithole_server_transporter;

// server transporter interface
void pd_rabbithole_server_transporter_sendToOne(rcp_server_transporter* transporter, char* data, size_t data_size, void* id);
void pd_rabbithole_server_transporter_sendToAll(rcp_server_transporter* transporter, char* data, size_t data_size, void* excludeId);


#ifdef __cplusplus
} // extern "C"
#endif

#endif // RABBITHOLESERVERTRANSPORTER_H
