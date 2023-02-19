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

#ifndef PARAMETERSERVER_H
#define PARAMETERSERVER_H

#include <vector>

#include <rcp_server.h>

#include "ParameterServerClientBase.h"
#include "PdServerTransporter.h"
#include "IServerTransporter.h"
#include "websocketServer.h"

#define RCP_WS_DEFAULT_PORT 10000

namespace rcp
{

    class ParameterBase;
    class RabbitHoleServerTransporter;


    class ParameterServer : public ParameterServerClientBase, public IWebsocketServerListener
    {
        // obligatory flext header (class name,base class name)
        FLEXT_HEADER_S(ParameterServer, flext_base, setup)

    public:
        ParameterServer(int argc, t_atom *argv);
        ~ParameterServer();

        rcp_server* server() const { return m_server; }

    public:
        // IWebsocketServerListener
        void connected(void* client) override;
        void disconnected(void* client) override;
        void received(char* /*data*/, size_t /*size*/, void* /*id*/) override {}
        void socketerror(const char* /*reason*/) override {}

    protected:
        static void setup(t_classid c)
        {
            // server
            FLEXT_CADDATTR_GET(c, "port", getPort);
            FLEXT_CADDMETHOD_I(c, 0, "listen", listen);
            // rabbithole
            FLEXT_CADDATTR_VAR(c, "rabbithole", getRabbithole, setRabbithole);
            FLEXT_CADDATTR_VAR(c, "rabbithole_interval", getRabbitholeInterval, setRabbitholeInterval);

            // parameter
            FLEXT_CADDMETHOD_(c, 0, "expose", exposeParameter);
            FLEXT_CADDMETHOD_(c, 0, "remove", removeParameterList);
            FLEXT_CADDMETHOD_I(c, 0, "remove", removeParameter);
            // parameter options
            FLEXT_CADDMETHOD_(c, 0, "setreadonly", parameterSetReadonly);
            FLEXT_CADDMETHOD_(c, 0, "setorder", parameterSetOrder);
            // min max
            FLEXT_CADDMETHOD_(c, 0, "setmin", parameterSetMin);
            FLEXT_CADDMETHOD_(c, 0, "setmax", parameterSetMax);
            FLEXT_CADDMETHOD_(c, 0, "setminmax", parameterSetMinMax);
        }

        void handle_raw_data(char* /*data*/, size_t /*size*/) override;

        // port
        void getPort(int& p);
        void listen(int& p);
        // rabbithole
        void setRabbithole(const t_symbol *&d);
        void getRabbithole(const t_symbol *&d);
        void setRabbitholeInterval(const int &i);
        void getRabbitholeInterval(int &i);
        // parameter
        void exposeParameter(int argc, t_atom* argv);
        void removeParameter(int id);
        void removeParameterList(int argc, t_atom* argv);
        // parameter options
        void parameterSetReadonly(int argc, t_atom* argv);
        void parameterSetOrder(int argc, t_atom* argv);
        // min max
        void parameterSetMin(int argc, t_atom* argv);
        void parameterSetMax(int argc, t_atom* argv);
        void parameterSetMinMax(int argc, t_atom* argv);

    private:
        rcp_group_parameter* createGroups(int argc, t_atom* argv, std::string& outLabel);
        void setupValueParameter(rcp_value_parameter* parameter);

    private:
        // port
        FLEXT_CALLGET_I(getPort)
        FLEXT_CALLBACK_I(listen)
        // rabbithole
        FLEXT_CALLSET_S(setRabbithole)
        FLEXT_CALLGET_S(getRabbithole)
        FLEXT_CALLSET_I(setRabbitholeInterval)
        FLEXT_CALLGET_I(getRabbitholeInterval)
        // parameter
        FLEXT_CALLBACK_V(exposeParameter)
        FLEXT_CALLBACK_I(removeParameter)
        FLEXT_CALLBACK_V(removeParameterList)
        // parameter options
        FLEXT_CALLBACK_V(parameterSetReadonly)
        FLEXT_CALLBACK_V(parameterSetOrder)
        // min max
        FLEXT_CALLBACK_V(parameterSetMin)
        FLEXT_CALLBACK_V(parameterSetMax)
        FLEXT_CALLBACK_V(parameterSetMinMax)

    private:
        rcp_server* m_server{nullptr};
        std::shared_ptr<IServerTransporter> m_transporter;
        std::shared_ptr<RabbitHoleServerTransporter> m_rabbitholeTransporter;

        bool m_raw;
        int m_clientCount;
    };

}


#endif // PARAMETERSERVER_H
