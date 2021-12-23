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

#include "PdServerTransporter.h"

#include <rcp_memory.h>

#include "ParameterServer.h"

namespace rcp
{

    PdServerTransporter::PdServerTransporter(ParameterServer* pdServer)
        : m_pdServer(pdServer)
    {
        m_transporter = (pd_server_transporter*)RCP_CALLOC(1, sizeof (pd_server_transporter));

        if (m_transporter)
        {
            m_transporter->pdST = this;

            rcp_server_transporter_setup(RCP_TRANSPORTER(m_transporter),
                                     pd_server_transporter_sendToOne,
                                     pd_server_transporter_sendToAll);
        }
    }

    PdServerTransporter::~PdServerTransporter()
    {
        if (m_transporter)
        {
            RCP_FREE(m_transporter);
        }
    }

    rcp_server_transporter* PdServerTransporter::transporter() const
    {
        if (m_transporter)
        {
            return &m_transporter->transporter;
        }

        return nullptr;
    }

    ParameterServer* PdServerTransporter::pdServer() const
    {
        return m_pdServer;
    }

    void PdServerTransporter::pushData(char* data, size_t data_size) const
    {
        pd_server_transporter_push_data(m_transporter, data, data_size);
    }

} // namespace rcp



//
void pd_server_transporter_push_data(pd_server_transporter* m_transporter, char* data, size_t data_size)
{
    if (m_transporter &&
            data  &&
            data_size > 0)
    {
        if (m_transporter->transporter.received)
        {
            m_transporter->transporter.received(m_transporter->transporter.server,
                                              data,
                                              data_size,
                                              NULL);
        }
    }
}

void pd_server_transporter_sendToOne(rcp_server_transporter* transporter, char* data, size_t data_size, void* id)
{
    if (transporter)
    {
        ((pd_server_transporter*)transporter)->pdST->pdServer()->dataOut(data, data_size);
    }
}

void pd_server_transporter_sendToAll(rcp_server_transporter* transporter, char* data, size_t data_size, void* excludeId)
{
    if (transporter)
    {
        ((pd_server_transporter*)transporter)->pdST->pdServer()->dataOut(data, data_size);
    }
}
