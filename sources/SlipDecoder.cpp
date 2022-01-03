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

#include "SlipDecoder.h"

#include <stdexcept>

namespace rcp
{
    static void packet_cb(char* data, size_t data_size, void* user)
    {
        if (user == nullptr)
        {
            return;
        }

        SlipDecoder* decoder = (SlipDecoder*)user;
        decoder->dataOut(data, data_size);
    }

    SlipDecoder::SlipDecoder(int argc, t_atom *argv) :
        m_slip(nullptr)
    {
        AddInAnything();

        FLEXT_ADDMETHOD(0, m_float);
        FLEXT_ADDMETHOD(0, m_list);

        int buffer_size = 1024;

        for (int i=0; i<argc; i++)
        {
            if ((IsInt(argv[i]) || IsFloat(argv[i])))
            {
                buffer_size = GetInt(argv[i]);
            }
        }

        if (buffer_size <= 0)
        {
            throw std::invalid_argument("please provide a valid buffersize");
        }

        // create parser
        m_slip = rcp_slip_create(buffer_size);
        if (m_slip)
        {
            rcp_slip_set_user(m_slip, this);
            rcp_slip_set_packet_cb(m_slip, packet_cb);
        }
        else
        {
            throw std::invalid_argument("could not allocate memory for slip");
        }
    }


    SlipDecoder::~SlipDecoder()
    {
        rcp_slip_free(m_slip);
    }


    void SlipDecoder::m_list(int argc, t_atom *argv)
    {
        for (int i=0; i<argc; i++)
        {
            if (IsFloat(argv[i]))
            {
                int id = (int)GetFloat(argv[i]);
                if (id < 256 && id >= 0)
                {
                    rcp_slip_append(m_slip, (char)id);
                }
            }
            else if (IsInt(argv[i]))
            {
                int id = (int)GetInt(argv[i]);
                if (id < 256 && id >= 0)
                {
                    rcp_slip_append(m_slip, (char)id);
                }
            }
        }
    }


    void SlipDecoder::m_float(float f)
    {
        int data = (int)f;
        if (data < 256 && data >= 0)
        {
            char d = (char)data;
            rcp_slip_append(m_slip, d);
        }
    }


    void SlipDecoder::dataOut(char* data, size_t data_size) const
    {
        t_atom atoms[data_size];

        for (size_t i=0; i<data_size; i++)
        {
            SetInt(atoms[i], (unsigned char)data[i]);
        }

        ToOutList(0, data_size, atoms);
    }

    FLEXT_LIB_V("slipdecoder", SlipDecoder)
}
