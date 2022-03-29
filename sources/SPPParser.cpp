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

#include "SPPParser.h"

#include <stdexcept>
#include <vector>

namespace rcp
{

    static void packet_cb(char* data, size_t data_size, void* user)
    {
        if (user == nullptr)
        {
            return;
        }

        SPPParser* parser = (SPPParser*)user;
        parser->dataOut(data, data_size);
    }

    SPPParser::SPPParser(int argc, t_atom *argv) :
        m_parser(nullptr)
    {
        AddInAnything();

        FLEXT_ADDMETHOD(0, m_float);
        FLEXT_ADDMETHOD(0, m_list);
        FLEXT_ADDMETHOD_(0, "reset", m_reset);

        int buffer_size = 1024;

        for (int i=0; i<argc; i++)
        {
            if (CanbeInt(argv[i]))
            {
                buffer_size = GetAInt(argv[i], 1024);
            }
        }

        if (buffer_size <= 0)
        {
            throw std::invalid_argument("please provide a valid buffersize");
        }

        // create parser
        m_parser = rcp_sppp_create(buffer_size, packet_cb, this);
    }

    SPPParser::~SPPParser()
    {
        if (m_parser)
        {
            rcp_sppp_free(m_parser);
        }
    }

    void SPPParser::m_list(int argc, t_atom *argv)
    {
        std::vector<char> data(argc);
        int offset = 0;

        for (int i=0; i<argc; i++)
        {
            if (CanbeInt(argv[i]))
            {
                int id = (int)GetAInt(argv[i], -1);
                if (id >= 0 &&
                        id < 256)
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

        rcp_sppp_data(m_parser, data.data(), argc - offset);
    }

    void SPPParser::m_float(float f)
    {
        int data = int(f);
        if (data < 256 && data >= 0)
        {
            char d = char(data);
            rcp_sppp_data(m_parser, &d, 1);
        }
    }

    void SPPParser::dataOut(char* data, size_t data_size) const
    {
        std::vector<t_atom> atoms(data_size);

        for (size_t i=0; i<data_size; i++)
        {
            SetInt(atoms[i], data[i]);
        }

        ToOutList(0, data_size, atoms.data());
    }

    void SPPParser::m_reset()
    {
        rcp_sppp_reset(m_parser);
    }

    FLEXT_LIB_V("sppp", SPPParser)
}
