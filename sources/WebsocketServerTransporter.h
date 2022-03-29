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

#ifndef WEBSOCKETSERVERTRANSPORTER_H
#define WEBSOCKETSERVERTRANSPORTER_H

#include <rcp_server_transporter.h>

#include "IServerTransporter.h"
#include "websocketServer.h"

typedef struct _pd_websocket_server_transporter pd_websocket_server_transporter;

namespace rcp
{
    class ParameterServer;

    class WebsocketServerTransporter
            : public IServerTransporter
            , public websocketServer
    {
    public:
        WebsocketServerTransporter(rcp_server* server, IWebsocketServerListener* listener);
        ~WebsocketServerTransporter();

    public:
        // IServerTransporter
        rcp_server_transporter* transporter() const override;
        void bind(uint16_t port) override;
        void unbind() override;
        uint16_t port() const override;
        bool isListening() const override;

        void sendToOne(char* data, size_t size, void* id);
        void sendToAll(char* data, size_t size, void* excludeId);

    public:
        // IWebsocketServerListener
        void connected(void* client) override;
        void disconnected(void* client) override;
        void received(char* data, size_t size, void* id) override;
        void socketerror(const char* reason) override;

    private:
        rcp_server* m_rcpServer;
        pd_websocket_server_transporter* m_transporter;
        IWebsocketServerListener* m_listener;
    };

}

#ifdef __cplusplus
extern "C"{
#endif


typedef struct _pd_websocket_server_transporter {
    rcp_server_transporter transporter;
	rcp::WebsocketServerTransporter* pdST;
} pd_websocket_server_transporter;

// server transporter interface
void pd_websocket_server_transporter_sendToOne(rcp_server_transporter* transporter, char* data, size_t data_size, void* id);
void pd_websocket_server_transporter_sendToAll(rcp_server_transporter* transporter, char* data, size_t data_size, void* excludeId);


#ifdef __cplusplus
} // extern "C"
#endif

#endif // WEBSOCKETSERVERTRANSPORTER_H
