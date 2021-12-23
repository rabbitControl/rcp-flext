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

#ifndef WEBSOCKETCLIENTTRANSPORTER_H
#define WEBSOCKETCLIENTTRANSPORTER_H

#include <rcp_client_transporter.h>

#include "IClientTransporter.h"
#include "websocketClient.h"

typedef struct _pd_websocket_client_transporter pd_websocket_client_transporter;

namespace rcp
{

    class ParameterClient;

    class WebsocketClientTransporter : public IClientTransporter, public websocketClient
    {
    public:
        WebsocketClientTransporter(IWebsocketClientListener* listener);
        ~WebsocketClientTransporter();

        // implement IClientTransporter
        rcp_client_transporter* transporter() const override;
        void open(const std::string& address) override;
        void close() override;
        void pushData(char* /*data*/, size_t /*size*/) const override {}

    public:
        // websocketClient
        void connected() override;
        void disconnected() override;
        void received(char* data, size_t size) override;
        void received(const std::string& /*msg*/) override {}

    private:
        pd_websocket_client_transporter* m_transporter;
        IWebsocketClientListener* m_listener;
    };

}


#ifdef __cplusplus
extern "C"{
#endif


typedef struct _pd_websocket_client_transporter {
    rcp_client_transporter transporter;
	rcp::WebsocketClientTransporter* pdST;
} pd_websocket_client_transporter;

// client transporter interface
void pd_websocket_client_transporter_send(rcp_client_transporter* transporter, char* data, size_t data_size);

#ifdef __cplusplus
} // extern "C"
#endif


#endif // WEBSOCKETCLIENTTRANSPORTER_H
