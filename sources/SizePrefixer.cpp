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

#include "SizePrefixer.h"

#include <cstring>

#include <rcp_endian.h>

namespace rcp
{

    SizePrefixer::SizePrefixer()
    {
        AddInList();

        AddOutList();

        FLEXT_ADDMETHOD(0, m_list);
    }

    void SizePrefixer::m_list(int argc, t_atom *argv)
    {
        t_atom size_a[4];
        char size_c[4];
        uint32_t n = htonl(argc);

        memcpy(size_c, &n, sizeof(uint32_t));

        for (int i=0; i<4; i++)
        {
            SetInt(size_a[i], size_c[i]);
        }

        ToOutList(1, 4, size_a);
        ToOutList(0, argc, argv);
    }


    FLEXT_LIB("sizeprefix", SizePrefixer)

}
