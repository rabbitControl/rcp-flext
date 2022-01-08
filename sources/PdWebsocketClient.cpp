/*
********************************************************************
* rabbitcontrol - a protocol and data-format for remote control.
*
* https://rabbitcontrol.cc
* https://github.com/rabbitControl/rcp-flext
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

#include "PdWebsocketClient.h"

#include <vector>

#include <rcp_packet.h>
#include <rcp_parameter.h>
#include <rcp_typedefinition.h>
#include <rcp_memory.h>
#include <rcp_logging.h>

namespace rcp
{

    PdWebsocketClient::PdWebsocketClient()
    {
        AddInList(0);
        FLEXT_ADDMETHOD(0, m_list);

        // binary output
        AddOutList(0);
        // message output
        AddOutSymbol(1);
        // connected / disconnected outlet
        AddOutInt(1);

        // create client
        m_client = std::make_shared<WebsocketClientImpl>(this);
    }

    // websocketClient
    void PdWebsocketClient::connected()
    {
        ToOutInt(2, 1);
    }

    void PdWebsocketClient::disconnected()
    {
        ToOutInt(2, 0);
    }

    void PdWebsocketClient::received(char* data, size_t size)
    {
        std::vector<t_atom> list(size);
        for (size_t i=0; i<size; i++)
        {
            SetInt(list[i], data[i]);
        }

        ToOutList(0, size, list.data());
    }

    void PdWebsocketClient::received(const std::string& msg)
    {
        ToOutSymbol(1, MakeSymbol(msg.c_str()));
    }

    void PdWebsocketClient::m_list(int argc, t_atom* argv)
    {
        if (!m_client) return;

        if (!m_client->isOpen())
        {
            post("websocket client is not connected");
            return;
        }

        std::vector<char> data(argc);

        for (int i=0; i<argc; i++)
        {
            if (CanbeInt(argv[i]))
            {
                unsigned int di = GetInt(argv[i]);
                if (di > 255)
                {
                    error("invalid data in packet");
                    return;
                }

                data[i] = (char)di;
            }
            else
            {
                error("malformed data");
                return;
            }
        }

        m_client->send(data.data(), argc);
    }

    void PdWebsocketClient::m_open(const t_symbol *d)
    {
        m_client->connect(std::string(GetString(d)));
    }

    void PdWebsocketClient::m_close()
    {
        m_client->disconnect();
    }

    FLEXT_LIB("ws.client", PdWebsocketClient);
}
