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

#include "PdWebsocketServer.h"

#include <cstring>
#include <cstdlib>

#include <rcp_packet.h>
#include <rcp_parameter.h>
#include <rcp_typedefinition.h>
#include <rcp_memory.h>
#include <rcp_logging.h>

namespace rcp
{

    PdWebsocketServer::PdWebsocketServer(int argc, t_atom *argv)
    {
        FLEXT_ADDMETHOD(0, m_list);

        AddOutList(0);
        AddOutInt(1);

        uint16_t port = 0;
        if (argc > 0)
        {
            if (CanbeInt(argv[0]))
            {
                int p = GetInt(argv[0]);

                if (p > 0 &&
                        p <= (int)UINT16_MAX)
                {
                    port = p;
                }
            }
        }

        if (port > 0)
        {
            m_server = std::make_shared<WebsocketServerImpl>(this);
            m_server->run(port);
        }
    }


    // IWebsocketServerListener
    void PdWebsocketServer::connected(void* /*client*/)
    {
        ToOutInt(1, m_server->connections());
    }

    void PdWebsocketServer::disconnected(void* /*client*/)
    {
        ToOutInt(1, m_server->connections());
    }

    void PdWebsocketServer::received(char* data, size_t size, void* /*client*/)
    {
        t_atom list[size];
        for (size_t i=0; i<size; i++)
        {
            SetInt(list[i], data[i]);
        }

        ToOutList(0, size, list);
    }

    void PdWebsocketServer::socketerror(const char* reason)
    {
        error("Could not bind to port %d: %s", m_server->port(), reason);
    }



    void PdWebsocketServer::m_list(int argc, t_atom* argv)
    {
        char data[argc];

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

        m_server->send(data, argc);
    }

    void PdWebsocketServer::m_listen(int& port)
    {
        int p = port;
        if (p > (int)UINT16_MAX)
        {
            return;
        }

        if (m_server && p == m_server->port())
        {
            return;
        }

        if (p > 0)
        {
            m_server = std::make_shared<WebsocketServerImpl>(this);
            m_server->run(p);
        }
        else
        {
            // dispose server
            m_server.reset();
        }
    }

    FLEXT_LIB_V("ws.server", PdWebsocketServer);
}
