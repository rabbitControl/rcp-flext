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

#include "ParameterServerClientBase.h"

#include <string>
#include <vector>

#include <rcp_parameter.h>
#include <rcp_typedefinition.h>
#include <rcp_logging.h>
#include <rcp_manager.h>


namespace rcp
{
    static void postAtoms(int argc, t_atom* argv)
    {
        for (int i=0; i<argc; i++)
        {
            if (flext::IsFloat(argv[i]))
            {
                post("float: %f", flext::GetFloat(argv[i]));
            }
            else if (flext::IsInt(argv[i]))
            {
                post("int: %d", flext::GetInt(argv[i]));
            }
            else if (flext::IsString(argv[i]))
            {
                const char* s = flext::GetString(argv[i]);
                post("sym: %s", s);
            }
            else
            {
                post("other");
            }
        }
    }

    static bool setAtomValue(rcp_parameter* param, t_atom& atom)
    {
        if (param == nullptr) return false;

        rcp_datatype type = rcp_typedefinition_get_type_id(rcp_parameter_get_typedefinition(param));
        bool was_set = false;

        if (rcp_parameter_is_value(param))
        {
            if (type == DATATYPE_BOOLEAN)
            {
                if (flext::CanbeInt(atom))
                {
                    rcp_parameter_set_value_bool(RCP_VALUE_PARAMETER(param), flext::GetAInt(atom) > 0);
                    was_set = true;
                }
            }
            else if (type == DATATYPE_INT32)
            {
                if (flext::CanbeInt(atom))
                {
                    rcp_parameter_set_value_int32(RCP_VALUE_PARAMETER(param), flext::GetAInt(atom));
                    was_set = true;
                }
            }
            else if (type == DATATYPE_FLOAT32)
            {
                if (flext::IsFloat(atom))
                {
                    rcp_parameter_set_value_float(RCP_VALUE_PARAMETER(param), flext::GetFloat(atom));
                    was_set = true;
                }
            }
            else if (type == DATATYPE_STRING)
            {
                if (flext::IsString(atom))
                {
                    rcp_parameter_set_value_string(RCP_VALUE_PARAMETER(param), flext::GetString(atom));
                    was_set = true;
                }
            }
        }
        else if (type == DATATYPE_BANG)
        {
        }

        return was_set;
    }


    static std::string typeToString(rcp_datatype type)
    {
        switch(type)
        {
        case DATATYPE_FLOAT32:
            return "float";
        case DATATYPE_INT32:
            return "int";
        case DATATYPE_BOOLEAN:
            return "toggle";
        case DATATYPE_BANG:
            return "bang";
        case DATATYPE_STRING:
            return "string";
        case DATATYPE_GROUP:
            return "group";
        }

        return "unknown";
    }


    ParameterServerClientBase::ParameterServerClientBase() :
        m_manager(nullptr)
    {
        AddInAnything();
        FLEXT_ADDMETHOD(0, m_list);
        FLEXT_ADDMETHOD(0, m_any);
        FLEXT_ADDBANG(0, m_bang);

        FLEXT_ADDMETHOD_(0, "info", parameterInfo);
        FLEXT_ADDMETHOD_(0, "id", parameterId);
        FLEXT_ADDMETHOD_(0, "type", parameterType);
        FLEXT_ADDMETHOD_(0, "readonly", parameterReadonly);
        FLEXT_ADDMETHOD_(0, "order", parameterOrder);
        FLEXT_ADDMETHOD_(0, "value", parameterValue);
        FLEXT_ADDMETHOD_(0, "min", parameterMin);
        FLEXT_ADDMETHOD_(0, "max", parameterMax);

        // parameter outlet
        AddOutList(0);
        // parameter id
        AddOutInt();
        // outlet connected clients (server) or connected/disconnected status (client)
        AddOutInt();
        // add info outlet
        AddOutAnything();
    }


    void ParameterServerClientBase::_input(rcp_parameter* parameter, int argc, t_atom* argv)
    {
        if (parameter)
        {
            if (rcp_parameter_is_group(parameter))
            {
                if (argc > 1)
                {
                    // look for parameter in group
                    parameter = getParameter(argc-1, argv, RCP_GROUP_PARAMETER(parameter));
                    if (parameter == NULL)
                    {
                        post("parameter not found");
                        return;
                    }

                    if (rcp_parameter_is_group(parameter))
                    {
                        post("can not set value for group parameter");
                        return;
                    }
                }
                else
                {
                    // can not set value for group parameter
                    post("can not set value for group parameter");
                    return;
                }
            }
            else if (rcp_parameter_is_type(parameter, DATATYPE_BANG))
            {
                rcp_manager_set_dirty(m_manager, parameter);
                rcp_manager_update(m_manager);
                return;
            }

            // set value
            if (setAtomValue(parameter, argv[argc-1]))
            {
                rcp_manager_update(m_manager);
            }
        }
    }

    void ParameterServerClientBase::m_list(int argc, t_atom* argv)
    {
        // <id> <value>
        // <group> <group> ... <label> <value>

//        postAtoms(argc, argv);

        int16_t id = 0;
        if (flext::CanbeInt(argv[0]))
        {
            id = flext::GetAInt(argv[0]);
        }

        if (id != 0)
        {
            rcp_parameter* p = rcp_manager_get_parameter(m_manager, id);
            // TODO: check for group and bang parameter

            if (setAtomValue(p, argv[argc-1]))
            {
                rcp_manager_update(m_manager);
                return;
            }
        }



        if (flext::IsString(argv[0]))
        {
            rcp_parameter* param = rcp_manager_find_parameter(m_manager, GetString(argv[0]), NULL);
            _input(param, argc-1, argv+1);
        }
    }

    void ParameterServerClientBase::m_any(t_symbol* sym, int argc, t_atom* argv)
    {
        rcp_parameter* param = rcp_manager_find_parameter(m_manager, sym->s_name, NULL);
        _input(param, argc, argv);
    }


    void ParameterServerClientBase::m_bang()
    {
        rcp_parameter_list* list = rcp_manager_get_paramter_list(m_manager);
        post("---- parameter ----");
        while (list != NULL)
        {
            rcp_datatype type = RCP_TYPE_ID(list->parameter);
            std::string ts = typeToString(type);
            const char* label = rcp_parameter_get_label(list->parameter);
            post("%s\tid: %d\ttype: %s", label, rcp_parameter_get_id(list->parameter), ts.c_str());
            list = list->next;
        }
    }


    void ParameterServerClientBase::_outputInfo(rcp_parameter* parameter, int argc, t_atom* argv)
    {
        if (parameter)
        {
            rcp_typedefinition* td = rcp_parameter_get_typedefinition(parameter);

            int16_t id = rcp_parameter_get_id(parameter);
            rcp_datatype type = RCP_TYPE_ID(parameter);
            std::string ts = typeToString(type);

            // info <group-label-list> <value> <min> <max> <id> <type>

            int len = 3 + argc;
            bool has_min = false;
            bool has_max = false;

            if (rcp_parameter_is_value(parameter))
            {
                len++;

                if (type == DATATYPE_FLOAT32 || type == DATATYPE_INT32)
                {
                    // check if we have min and max
                    if (rcp_typedefinition_has_option(td, NUMBER_OPTIONS_MINIMUM))
                    {
                        has_min = true;
                        len++;
                    }

                    if (rcp_typedefinition_has_option(td, NUMBER_OPTIONS_MAXIMUM))
                    {
                        has_max = true;
                        len++;
                    }
                }
            }


            std::vector<t_atom> list(len);
            int i=0;


            SetSymbol(list[i], MakeSymbol("info"));
            i++;

            for (int j=0; j<argc; j++, i++) {
                CopyAtom(&list[i], &argv[j]);
            }

            // set value
            if (type == DATATYPE_BOOLEAN)
            {
                SetBool(list[i], rcp_parameter_get_value_bool(RCP_VALUE_PARAMETER(parameter)));
                i++;
            }
            else if (type == DATATYPE_INT32)
            {
                SetInt(list[i], rcp_parameter_get_value_int32(RCP_VALUE_PARAMETER(parameter)));
                i++;

                if (has_min)
                {
                    // add min
                    SetInt(list[i], rcp_typedefinition_get_option_i32(td, NUMBER_OPTIONS_MINIMUM, 0));
                    i++;
                }

                if (has_max)
                {
                    // add max
                    SetInt(list[i], rcp_typedefinition_get_option_i32(td, NUMBER_OPTIONS_MAXIMUM, 0));
                    i++;
                }
            }
            else if (type == DATATYPE_FLOAT32)
            {
                SetFloat(list[i], rcp_parameter_get_value_float(RCP_VALUE_PARAMETER(parameter)));
                i++;

                if (has_min)
                {
                    // add min
                    SetFloat(list[i], rcp_typedefinition_get_option_f32(td, NUMBER_OPTIONS_MINIMUM, 0));
                    i++;
                }

                if (has_max)
                {
                    // add max
                    SetFloat(list[i], rcp_typedefinition_get_option_f32(td, NUMBER_OPTIONS_MAXIMUM, 0));
                    i++;
                }
            }
            else if (type == DATATYPE_STRING)
            {
                SetString(list[i], rcp_parameter_get_value_string(RCP_VALUE_PARAMETER(parameter)));
                i++;
            }

            SetInt(list[i], id);
            i++;

            SetString(list[i], ts.c_str());
            i++;

            ToOutList(3, i, list.data());
        }
    }

    void ParameterServerClientBase::parameterInfo(int argc, t_atom* argv)
    {
        if (argc == 0)
        {
            // output all
            rcp_parameter_list* list = rcp_manager_get_paramter_list(m_manager);
            while (list != NULL)
            {
                std::vector<std::string> groups = getParents(list->parameter);

                std::vector<t_atom> groups_a(groups.size());

                int i = 0;
                for(std::vector<std::string>::reverse_iterator rit = groups.rbegin();
                    rit != groups.rend(); ++rit)
                {
                    SetString(groups_a[i], rit->c_str());
                    i++;
                }

                _outputInfo(list->parameter, groups.size(), groups_a.data());

                list = list->next;
            }
        }
        else
        {
            rcp_parameter* parameter = getParameter(argc, argv);
            _outputInfo(parameter, argc, argv);
        }
    }

    void ParameterServerClientBase::parameterId(int argc, t_atom* argv)
    {
        rcp_parameter* parameter = getParameter(argc, argv);
        if (parameter)
        {
            int16_t id = rcp_parameter_get_id(parameter);

            // id <group-label-list> <id>
            int len = 2 + argc;
            std::vector<t_atom> list(len);
            int i=0;

            SetSymbol(list[i], MakeSymbol("id"));
            i++;

            for (int j=0; j<argc; j++, i++) {
                CopyAtom(&list[i], &argv[j]);
            }

            SetInt(list[i], id);
            i++;

            ToOutList(3, i, list.data());
        }
    }

    void ParameterServerClientBase::parameterType(int argc, t_atom* argv)
    {
        rcp_parameter* parameter = getParameter(argc, argv);
        if (parameter)
        {
            rcp_typedefinition* td = rcp_parameter_get_typedefinition(parameter);
            rcp_datatype type = RCP_TYPE_ID(parameter);
            std::string ts = typeToString(type);


            // id <group-label-list> <type>
            int len = 2 + argc;
            std::vector<t_atom> list(len);
            int i=0;

            SetSymbol(list[i], MakeSymbol("type"));
            i++;

            for (int j=0; j<argc; j++, i++) {
                CopyAtom(&list[i], &argv[j]);
            }

            SetString(list[i], ts.c_str());
            i++;

            ToOutList(3, i, list.data());
        }
    }

    void ParameterServerClientBase::parameterReadonly(int argc, t_atom* argv)
    {
        rcp_parameter* parameter = getParameter(argc, argv);
        if (parameter)
        {
            bool ro = rcp_parameter_get_readonly(parameter);

            // readonly <group-label-list> <ro>
            int len = 2 + argc;
            std::vector<t_atom> list(len);
            int i=0;

            SetSymbol(list[i], MakeSymbol("readonly"));
            i++;

            for (int j=0; j<argc; j++, i++) {
                CopyAtom(&list[i], &argv[j]);
            }

            SetInt(list[i], ro ? 1 : 0);
            i++;

            ToOutList(3, i, list.data());
        }
    }

    void ParameterServerClientBase::parameterOrder(int argc, t_atom* argv)
    {
        rcp_parameter* parameter = getParameter(argc, argv);
        if (parameter)
        {
            int32_t order = rcp_parameter_get_order(parameter);

            // order <group-label-list> <order>
            int len = 2 + argc;
            std::vector<t_atom> list(len);
            int i=0;

            SetSymbol(list[i], MakeSymbol("order"));
            i++;

            for (int j=0; j<argc; j++, i++) {
                CopyAtom(&list[i], &argv[j]);
            }

            SetInt(list[i], order);
            i++;

            ToOutList(3, i, list.data());
        }
    }

    void ParameterServerClientBase::parameterValue(int argc, t_atom* argv)
    {
        rcp_parameter* parameter = getParameter(argc, argv);
        if (parameter)
        {
            rcp_datatype type = RCP_TYPE_ID(parameter);

            // value <group-label-list> <value>
            int len = 1 + argc + (rcp_parameter_is_value(parameter) ? 1 : 0);
            std::vector<t_atom> list(len);
            int i=0;

            SetSymbol(list[i], MakeSymbol("value"));
            i++;

            for (int j=0; j<argc; j++, i++) {
                CopyAtom(&list[i], &argv[j]);
            }

            if (type == DATATYPE_BOOLEAN)
            {
                SetBool(list[i], rcp_parameter_get_value_bool(RCP_VALUE_PARAMETER(parameter)));
                i++;
            }
            else if (type == DATATYPE_INT32)
            {
                SetInt(list[i], rcp_parameter_get_value_int32(RCP_VALUE_PARAMETER(parameter)));
                i++;
            }
            else if (type == DATATYPE_FLOAT32)
            {
                SetFloat(list[i], rcp_parameter_get_value_float(RCP_VALUE_PARAMETER(parameter)));
                i++;
            }
            else if (type == DATATYPE_STRING)
            {
                SetString(list[i], rcp_parameter_get_value_string(RCP_VALUE_PARAMETER(parameter)));
                i++;
            }

            ToOutList(3, i, list.data());
        }
    }

    void ParameterServerClientBase::parameterMin(int argc, t_atom* argv)
    {
        rcp_parameter* parameter = getParameter(argc, argv);
        if (parameter)
        {
            rcp_typedefinition* td = rcp_parameter_get_typedefinition(parameter);
            rcp_datatype type = RCP_TYPE_ID(parameter);

            if (rcp_typedefinition_has_option(td, NUMBER_OPTIONS_MINIMUM))
            {
                // min <group-label-list> <min>
                int len = 1 + argc + 1;
                std::vector<t_atom> list(len);
                int i=0;

                SetSymbol(list[i], MakeSymbol("min"));
                i++;

                for (int j=0; j<argc; j++, i++) {
                    CopyAtom(&list[i], &argv[j]);
                }

                if (type == DATATYPE_INT32)
                {
                    SetInt(list[i], rcp_parameter_get_min_int32(RCP_VALUE_PARAMETER(parameter)));
                    i++;
                }
                else if (type == DATATYPE_FLOAT32)
                {
                    SetFloat(list[i], rcp_parameter_get_min_float(RCP_VALUE_PARAMETER(parameter)));
                    i++;
                }

                ToOutList(3, i, list.data());
            }
        }
    }

    void ParameterServerClientBase::parameterMax(int argc, t_atom* argv)
    {
        rcp_parameter* parameter = getParameter(argc, argv);
        if (parameter)
        {
            rcp_typedefinition* td = rcp_parameter_get_typedefinition(parameter);
            rcp_datatype type = RCP_TYPE_ID(parameter);

            if (rcp_typedefinition_has_option(td, NUMBER_OPTIONS_MAXIMUM))
            {
                // max <group-label-list> <max>
                int len = 1 + argc + 1;
                std::vector<t_atom> list(len);
                int i=0;

                SetSymbol(list[i], MakeSymbol("max"));
                i++;

                for (int j=0; j<argc; j++, i++) {
                    CopyAtom(&list[i], &argv[j]);
                }

                if (type == DATATYPE_INT32)
                {
                    SetInt(list[i], rcp_parameter_get_max_int32(RCP_VALUE_PARAMETER(parameter)));
                    i++;
                }
                else if (type == DATATYPE_FLOAT32)
                {
                    SetFloat(list[i], rcp_parameter_get_max_float(RCP_VALUE_PARAMETER(parameter)));
                    i++;
                }

                ToOutList(3, i, list.data());
            }
        }
    }



    std::vector<std::string> ParameterServerClientBase::getParents(rcp_parameter* parameter)
    {
        std::vector<std::string> groups;

        rcp_group_parameter* last_group = rcp_parameter_get_parent(RCP_PARAMETER(parameter));
        while (last_group != NULL)
        {
            const char* label = rcp_parameter_get_label(RCP_PARAMETER(last_group));
            groups.push_back(label != NULL ? label : "null");

            last_group = rcp_parameter_get_parent(RCP_PARAMETER(last_group));
        }

        return groups;
    }

    void ParameterServerClientBase::parameterUpdate(rcp_parameter* parameter)
    {
        const char* label = rcp_parameter_get_label(parameter);
        int16_t id = rcp_parameter_get_id(parameter);
        rcp_datatype type = rcp_typedefinition_get_type_id(rcp_parameter_get_typedefinition(parameter));

        // get the parents
        std::vector<std::string> groups = getParents(parameter);


        // output [list]
        // update group1 groupN... label value

        int len = 3 + groups.size();
        std::vector<t_atom> list(len);

        int i=0;
        SetSymbol(list[i], MakeSymbol("update"));
        i++;

        for (std::vector<std::string>::reverse_iterator rit = groups.rbegin();
            rit != groups.rend(); ++rit)
        {
            SetString(list[i], rit->c_str());
            i++;
        }

        SetSymbol(list[i], MakeSymbol(label != NULL ? label : "<nolabel>"));
        i++;

        if (type == DATATYPE_BOOLEAN)
        {
            SetBool(list[i], rcp_parameter_get_value_bool(RCP_VALUE_PARAMETER(parameter)));
            i++;
        }
        else if (type == DATATYPE_INT32)
        {
            SetInt(list[i], rcp_parameter_get_value_int32(RCP_VALUE_PARAMETER(parameter)));
            i++;
        }
        else if (type == DATATYPE_FLOAT32)
        {
            SetFloat(list[i], rcp_parameter_get_value_float(RCP_VALUE_PARAMETER(parameter)));
            i++;
        }
        else if (type == DATATYPE_STRING)
        {
            SetString(list[i], rcp_parameter_get_value_string(RCP_VALUE_PARAMETER(parameter)));
            i++;
        }

        ToOutInt(1, id);
        ToOutList(0, i, list.data());
    }

    std::string ParameterServerClientBase::GetAsString(const t_atom &a)
    {
        if (IsString(a))
        {
            return GetString(a);
        }

        if (IsInt(a) || CanbeInt(a))
        {
            int i = GetInt(a);
            return std::to_string(i);
        }

        if (IsFloat(a))
        {
            float f = GetFloat(a);
            return std::to_string(f);
        }

        return "";
    }

    rcp_parameter* ParameterServerClientBase::getParameter(int argc, t_atom* argv, rcp_group_parameter* group)
    {
        rcp_parameter* param = NULL;
        rcp_group_parameter* last_group = group;

        for (int i=0; i<argc; i++)
        {
            if (!IsString(argv[i]))
            {
                // not a string!
                return NULL;
            }

            param = rcp_manager_find_parameter(m_manager, GetString(argv[i]), last_group);
            if (param == NULL)
            {
                // not found
                return NULL;
            }

            if (rcp_parameter_is_group(param))
            {
                last_group = RCP_GROUP_PARAMETER(param);
            }
        }

        return param;
    }

}
