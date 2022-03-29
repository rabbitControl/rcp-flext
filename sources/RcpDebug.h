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

#ifndef RCPDEBUG_H
#define RCPDEBUG_H

#include <flext.h>

namespace rcp
{
    class RcpDebug : public flext_base
    {
        FLEXT_HEADER_S(RcpDebug, flext_base, setup)

    public:
        RcpDebug();

    protected:
        static void setup(t_classid c)
        {
            FLEXT_CADDBANG(c, 0, m_bang);
            FLEXT_CADDMETHOD_B(c, 0, "debug", m_debug);
            FLEXT_CADDMETHOD_(c, 0, "version", m_version);
            FLEXT_CADDMETHOD_(c, 0, "memory", m_memory);
        }

        void m_bang();
        void m_debug(bool b);
        void m_version();
        void m_memory();

    private:
        FLEXT_CALLBACK(m_bang)
        FLEXT_CALLBACK_B(m_debug)
        FLEXT_CALLBACK(m_version)
        FLEXT_CALLBACK(m_memory)
    };

}

#endif // RCPDEBUG_H
