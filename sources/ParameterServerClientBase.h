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

#ifndef PARAMETERSERVERCLIENTBASE_H
#define PARAMETERSERVERCLIENTBASE_H

#include <vector>
#include <string>

#include <flext.h>

#include <rcp_parameter_type.h>
#include <rcp_manager_type.h>

namespace rcp
{
    class ParameterServerClientBase : public flext_base
    {
        FLEXT_HEADER(ParameterServerClientBase, flext_base)

    public:
        ParameterServerClientBase();

        void parameterUpdate(rcp_parameter* parameter);

    protected:
        void m_any(t_symbol* sym, int argc, t_atom* argv);
        void m_list(int argc, t_atom* argv);
        void m_bang();
        void parameterInfo(int argc, t_atom* argv);
        void parameterId(int argc, t_atom* argv);
        void parameterType(int argc, t_atom* argv);
        void parameterReadonly(int argc, t_atom* argv);
        void parameterOrder(int argc, t_atom* argv);
        void parameterValue(int argc, t_atom* argv);
        void parameterMin(int argc, t_atom* argv);
        void parameterMax(int argc, t_atom* argv);

    protected:
        std::string GetAsString(const t_atom &a);
        rcp_parameter* getParameter(int argc, t_atom* argv, rcp_group_parameter* group = NULL);
        std::vector<std::string> getParents(rcp_parameter* parameter);
        rcp_manager* m_manager;

    private:
        FLEXT_CALLBACK_A(m_any)
        FLEXT_CALLBACK_V(m_list)
        FLEXT_CALLBACK(m_bang)
        FLEXT_CALLBACK_V(parameterInfo)
        FLEXT_CALLBACK_V(parameterId)
        FLEXT_CALLBACK_V(parameterType)
        FLEXT_CALLBACK_V(parameterReadonly)
        FLEXT_CALLBACK_V(parameterOrder)
        FLEXT_CALLBACK_V(parameterValue)
        FLEXT_CALLBACK_V(parameterMin)
        FLEXT_CALLBACK_V(parameterMax)

    private:         
        void _outputInfo(rcp_parameter* parameter, int argc, t_atom* argv);
        void _input(rcp_parameter* parameter, int argc, t_atom* argv);

    };

}

#endif // PARAMETERSERVERCLIENTBASE_H
