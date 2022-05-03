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

#ifndef PARAMETERCLIENT_H
#define PARAMETERCLIENT_H

#include <rcp_client.h>

#include "ParameterServerClientBase.h"
#include "IClientTransporter.h"
#include "websocketClient.h"

namespace rcp
{

    class ParameterClient : public ParameterServerClientBase, public IWebsocketClientListener
    {
        FLEXT_HEADER_S(ParameterClient, flext_base, setup)

    public:
        ParameterClient();
        ~ParameterClient();

        void parameterAdded(rcp_parameter* parameter);
        void parameterRemoved(rcp_parameter* parameter);        

    public:
        // IWebsocketClientListener
        void connected() override;
        void failed(uint16_t code) override;
        void disconnected(uint16_t code) override;
        void received(char* /*data*/, size_t /*size*/) override {}
        void received(const std::string& /*msg*/) override {}

    protected:
        static void setup(t_classid c)
        {            
            FLEXT_CADDMETHOD_(c, 0, "open", m_open);
            FLEXT_CADDMETHOD_(c, 0, "close", m_close);
        }

        void m_open(const t_symbol *d);
        void m_close();

    private:        
        FLEXT_CALLBACK_S(m_open)
        FLEXT_CALLBACK(m_close)        

    private:
        rcp_client* m_client;
        IClientTransporter* m_transporter;
    };

}

#endif // PARAMETERCLIENT_H
