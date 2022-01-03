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

#ifndef RCPFORMAT_H
#define RCPFORMAT_H

#include <string>
#include <vector>

#include <flext.h>

#include <rcp.h>

namespace rcp
{

    class RcpFormat : public flext_base
    {
        FLEXT_HEADER_S(RcpFormat, flext_base, setup)

    public:
        RcpFormat(int argc, t_atom *argv);

    protected:
        static void setup(t_classid c)
        {
            //
            FLEXT_CADDMETHOD_I(c, 0, "set", m_setId);
            FLEXT_CADDATTR_VAR(c, "type", getType, setType);
            FLEXT_CADDATTR_VAR(c, "label", getLabel, setLabel);
            FLEXT_CADDMETHOD_(c, 0, "clearlabel", clearLabel);
        }

        void m_bang();
        void m_int(int& v);
        void m_float(float& v);
        void m_symbol(t_symbol*& v);

        void m_setId(int& v);

        // type
        void setType(const t_symbol*& p);
        void getType(const t_symbol*& p);
        // label
        void setLabel(const t_symbol*& p);
        void getLabel(const t_symbol*& p);
        void clearLabel();

    private:
        FLEXT_CALLBACK(m_bang)
        FLEXT_CALLBACK_I(m_int)
        FLEXT_CALLBACK_F(m_float)
        FLEXT_CALLBACK_S(m_symbol)

        FLEXT_CALLBACK_I(m_setId)

        FLEXT_CALLSET_S(setType)
        FLEXT_CALLGET_S(getType)
        FLEXT_CALLSET_S(setLabel)
        FLEXT_CALLGET_S(getLabel)
        FLEXT_CALLBACK(clearLabel)

    private:
        int16_t m_id;
        rcp_datatype m_type;
        std::string m_label;

        // update start
        // 4 18 id id t 0 32 value 33 any len label 0 0 0

        std::vector<char> labelOption;
    };

}

#endif // RCPFORMAT_H
