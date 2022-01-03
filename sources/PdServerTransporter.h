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

#ifndef PDSERVERTRANSPORTER_H
#define PDSERVERTRANSPORTER_H

#include <rcp_server_transporter.h>

#include "IServerTransporter.h"

typedef struct _pd_server_transporter pd_server_transporter;

namespace rcp
{

    class ParameterServer;

    class PdServerTransporter : public IServerTransporter
    {
    public:
        PdServerTransporter(ParameterServer* pdServer);
        ~PdServerTransporter();

        ParameterServer* pdServer() const;

    public:
        // implement IPdServerTransporter
        rcp_server_transporter* transporter() const override;
        virtual void bind(uint16_t /*port*/) override {}
        virtual void unbind() override {}
        virtual void pushData(char* data, size_t data_size) const override;
        uint16_t port() const override { return 0; }
        bool isListening() const override { return true; }



    private:
        ParameterServer* m_pdServer;
        pd_server_transporter* m_transporter;
    };

}


#ifdef __cplusplus
extern "C"{
#endif


typedef struct _pd_server_transporter {
    rcp_server_transporter transporter;
	rcp::PdServerTransporter* pdST;
} pd_server_transporter;


void pd_server_transporter_push_data(pd_server_transporter* transporter, char* data, size_t data_size);

// server transporter interface
void pd_server_transporter_sendToOne(rcp_server_transporter* transporter, char* data, size_t data_size, void* id);
void pd_server_transporter_sendToAll(rcp_server_transporter* transporter, char* data, size_t data_size, void* excludeId);

#ifdef __cplusplus
} // extern "C"
#endif



#endif // PDSERVERTRANSPORTER_H
