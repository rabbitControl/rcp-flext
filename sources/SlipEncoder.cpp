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

#include "SlipEncoder.h"

#include <rcp_slip.h>

namespace rcp
{

    static void data_out(char data, void* user)
    {
        if (user)
        {
            SlipEncoder* encoder = (SlipEncoder*)user;
            encoder->appendData(data);
        }
    }

    SlipEncoder::SlipEncoder()
    {
        AddInAnything();

        FLEXT_ADDMETHOD(0, m_list);
    }

    void SlipEncoder::m_list(int argc, t_atom *argv)
    {
        char data[argc];
        int offset = 0;

        for (int i=0; i<argc; i++)
        {
            if (IsFloat(argv[i]))
            {
                int id = (int)GetFloat(argv[i]);
                if (id < 256 && id >= 0)
                {
                    data[i-offset] = id;
                }
                else
                {
                    offset++;
                }
            }
            else if (IsInt(argv[i]))
            {
                int id = (int)GetInt(argv[i]);
                if (id < 256 && id >= 0)
                {
                    data[i-offset] = id;
                }
                else
                {
                    offset++;
                }
            }
            else
            {
                offset++;
            }
        }

        m_data.clear();
        rcp_slip_encode(data, argc - offset, data_out, this);

        // output data
        t_atom atoms[m_data.size()];
        for (size_t i=0; i<m_data.size(); i++)
        {
            SetInt(atoms[i], (unsigned char)m_data[i]);
        }

        ToOutList(0, m_data.size(), atoms);
    }


    void SlipEncoder::appendData(char data)
    {
        m_data.push_back(data);
    }

    FLEXT_LIB("slipencoder", SlipEncoder)

}
