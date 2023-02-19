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
        void failed(uint16_t code) override;
        void disconnected(uint16_t code) override;
        void received(char* data, size_t size) override;
        void received(const std::string& /*msg*/) override {}

        // websocketClient overrides
        void connect(const std::string& uri, const std::string& subprotocol = "") override;


    private:
        void tryConnect();

        rcp_server* m_rcpServer{nullptr};
        rcp_server_transporter* m_transporter{nullptr};
        std::string m_uri;
        bool m_oneTimeError{true};

        flext::Timer m_tryConnectTimer;
        int m_connectInterval;
        std::atomic<bool> m_doTryConnect{false};
    };
}

#endif // RABBITHOLESERVERTRANSPORTER_H
