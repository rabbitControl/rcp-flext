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

#include "RcpFormat.h"

#include <stdint.h>
#include <cstring>

#include <rcp_endian.h>
#include <rcp_option.h>

namespace rcp
{

    #define STORE_ID_TO_LIST(m_id, list, o) do {           \
        unsigned char id[2]; _rcp_store16(id, m_id);             \
        SetInt(list[o], id[0]); SetInt(list[o+1], id[1]); \
    } while(0)

    #define STORE_32_TO_LIST(v, list, o) do {           \
        unsigned char _v[4]; _rcp_store32(_v, v);             \
        SetInt(list[o], _v[0]); \
        SetInt(list[o+1], _v[1]); \
        SetInt(list[o+2], _v[2]); \
        SetInt(list[o+3], _v[3]); \
    } while(0)

    #define SETUP_UPDATE_PACKET(id, list, type) do {\
        SetInt(list[0], COMMAND_UPDATE);\
        SetInt(list[1], PACKET_OPTIONS_DATA);\
        STORE_ID_TO_LIST(id, list, 2);\
        SetInt(list[4], type);\
        SetInt(list[5], RCP_TERMINATOR);\
        SetInt(list[6], PARAMETER_OPTIONS_VALUE);\
    } while(0)

    union _int_float_union{
        int32_t i;
        float f;
    };


    RcpFormat::RcpFormat(int argc, t_atom *argv)
        : m_type(DATATYPE_INVALID)
    {
        int id = 1;

        if (argc > 0)
        {
            if (!CanbeInt(argv[0]))
            {
                throw std::runtime_error("id needs to be integer");
            }

            id = GetInt(argv[0]);
        }

        // check limits
        if (id == 0)
        {
            throw std::runtime_error("id can not be 0");
        }
        if (id > INT16_MAX)
        {
            throw std::runtime_error("id is too large");
        }
        if (id < INT16_MIN)
        {
            throw std::runtime_error("id is too small");
        }

        // all good
        m_id = id;


        AddInAnything(0);
        AddInBang(0);
        AddInFloat(0);
        AddInInt(0);
        AddInSymbol(0);


        FLEXT_ADDBANG(0, m_bang);
        FLEXT_ADDMETHOD(0, m_float);
        FLEXT_ADDMETHOD(0, m_int);
        FLEXT_ADDMETHOD(0, m_symbol);
//        FLEXT_ADDMETHOD(0, m_list);

        // output
        AddOutList(0);
        AddOutAnything(1);
    }

    void RcpFormat::m_bang()
    {
        if (m_type == DATATYPE_INVALID ||
                m_type == DATATYPE_BANG)
        {
            if (m_label.empty())
            {
                // send valueupdate
                // 06 id id type
                t_atom list[4];
                SetInt(list[0], COMMAND_UPDATEVALUE);
                STORE_ID_TO_LIST(m_id, list, 1);
                SetInt(list[3], DATATYPE_BANG);

                ToOutList(0, 4, list);
            }
            else
            {
                // send update with label

                // 4 18 id id t 0 33 any len label 0 0 0
                t_atom list[6 + labelOption.size()];
                SETUP_UPDATE_PACKET(m_id, list, DATATYPE_BANG);

                // add option label
                for (size_t i=0; i<labelOption.size(); i++)
                {
                    SetInt(list[6+i], labelOption[i]);
                }

                ToOutList(0, 6 + labelOption.size(), list);
            }
        }
    }

    void RcpFormat::m_int(int& v)
    {
        if (m_type == DATATYPE_INVALID ||
                m_type == DATATYPE_INT32)
        {
            if (m_label.empty())
            {
                // send valueupdate
                // 06 id id type v v v v
                t_atom list[8];
                SetInt(list[0], COMMAND_UPDATEVALUE);
                STORE_ID_TO_LIST(m_id, list, 1);
                SetInt(list[3], DATATYPE_INT32);

                STORE_32_TO_LIST(v, list, 4);

                ToOutList(0, 8, list);
            }
            else
            {
                // send update with label

                // 4 18 id id t 0 32 v v v v 33 any len label 0 0 0
                t_atom list[11 + labelOption.size()];
                SETUP_UPDATE_PACKET(m_id, list, DATATYPE_INT32);
                STORE_32_TO_LIST(v, list, 7);

                // add option label
                for (size_t i=0; i<labelOption.size(); i++)
                {
                    SetInt(list[11+i], labelOption[i]);
                }

                ToOutList(0, 11 + labelOption.size(), list);
            }
        }
        else if (m_type == DATATYPE_BOOLEAN)
        {
            if (m_label.empty())
            {
                // send valueupdate
                // 06 id id type v
                t_atom list[5];
                SetInt(list[0], COMMAND_UPDATEVALUE);
                STORE_ID_TO_LIST(m_id, list, 1);
                SetInt(list[3], DATATYPE_BOOLEAN);

                SetInt(list[4], v > 0 ? 1 : 0);

                ToOutList(0, 5, list);
            }
            else
            {
                // send update with label

                // 4 18 id id t 0 32 v 33 any len label 0 0 0
                t_atom list[8 + labelOption.size()];
                SETUP_UPDATE_PACKET(m_id, list, DATATYPE_BOOLEAN);

                SetInt(list[7], v > 0 ? 1 : 0);

                // add option label
                for (size_t i=0; i<labelOption.size(); i++)
                {
                    SetInt(list[8+i], labelOption[i]);
                }

                ToOutList(0, 8 + labelOption.size(), list);
            }
        }
        else if (m_type == DATATYPE_FLOAT32)
        {
            float vf = v;
            m_float(vf);
        }
        else if (m_type == DATATYPE_STRING)
        {
            std::string s = std::to_string(v);
            t_symbol* sym = const_cast<t_symbol*>(MakeSymbol(s.c_str()));
            m_symbol(sym);
        }
    }

    void RcpFormat::m_float(float& v)
    {
        if (m_type == DATATYPE_INVALID ||
                m_type == DATATYPE_FLOAT32)
        {
            union _int_float_union uu;
            uu.f = v;

            if (m_label.empty())
            {
                // send valueupdate
                // 06 id id type v v v v
                t_atom list[8];
                SetInt(list[0], COMMAND_UPDATEVALUE);
                STORE_ID_TO_LIST(m_id, list, 1);
                SetInt(list[3], DATATYPE_FLOAT32);

                STORE_32_TO_LIST((uint32_t)uu.i, list, 4);

                ToOutList(0, 8, list);
            }
            else
            {
                // send update with label

                // 4 18 id id t 0 32 v v v v 33 any len label 0 0 0
                t_atom list[11 + labelOption.size()];
                SETUP_UPDATE_PACKET(m_id, list, DATATYPE_FLOAT32);

                STORE_32_TO_LIST((uint32_t)uu.i, list, 7);

                // add option label
                for (size_t i=0; i<labelOption.size(); i++)
                {
                    SetInt(list[11+i], labelOption[i]);
                }

                ToOutList(0, 11 + labelOption.size(), list);
            }
        }
        else if (m_type == DATATYPE_INT32 ||
                 m_type == DATATYPE_BOOLEAN)
        {
            int vi = v;
            m_int(vi);
        }
        else if (m_type == DATATYPE_STRING)
        {
            std::string s = std::to_string(v);
            t_symbol* sym = const_cast<t_symbol*>(MakeSymbol(s.c_str()));
            m_symbol(sym);
        }
    }

    void RcpFormat::m_symbol(t_symbol*& v)
    {
        if (m_type == DATATYPE_INVALID ||
                m_type == DATATYPE_STRING)
        {
            const char* str = GetString(v);
            int str_len = strlen(str);

            if (m_label.empty())
            {
                // send valueupdate
                // 06 id id type s s s s v v v v
                t_atom list[8 + str_len];
                SetInt(list[0], COMMAND_UPDATEVALUE);
                STORE_ID_TO_LIST(m_id, list, 1);
                SetInt(list[3], DATATYPE_STRING);

                // set size prefix - long string
                STORE_32_TO_LIST(str_len, list, 4);

                for (int i=0; i<str_len; i++)
                {
                    SetInt(list[8+i], str[i]);
                }

                ToOutList(0, 8 + str_len, list);
            }
            else
            {
                // send update with label

                // 4 18 id id t 0 32 s s s s string-data 33 any len label 0 0 0
                t_atom list[11 + str_len + labelOption.size()];
                SETUP_UPDATE_PACKET(m_id, list, DATATYPE_STRING);

                // set size prefix - long string
                STORE_32_TO_LIST(str_len, list, 7);

                for (int i=0; i<str_len; i++)
                {
                    SetInt(list[11+i], str[i]);
                }

                // add option label
                for (size_t i=0; i<labelOption.size(); i++)
                {
                    SetInt(list[11 + str_len + i], labelOption[i]);
                }

                ToOutList(0, 11 + str_len + labelOption.size(), list);
            }
        }
    }

//    void RcpFormat::m_list(int argc, t_atom* argv)
//    {
//        std::stringstream ss;

//        for(int i=0; i<argc; i++)
//        {
//            if (IsString(argv[i]))
//            {
//                ss << GetString(argv[i]);
//            }
//            else if (IsFloat(argv[i]))
//            {
//                ss << GetFloat(argv[i]);
//            }
//            else if (IsInt(argv[i]))
//            {
//                ss << GetInt(argv[i]);
//            }

//            if (i < argc-1) ss << " ";
//        }

//        auto str = ss.str();
//    }

//    void RcpFormat::m_any(t_symbol* sym, int argc, t_atom* argv)
//    {
//        std::stringstream ss;
//        ss << sym->s_name;
//        if (argc > 0)
//        {
//            ss << " ";
//        }

//        for(int i=0; i<argc; i++)
//        {
//            if (IsString(argv[i]))
//            {
//                ss << GetString(argv[i]);
//            }
//            else if (IsFloat(argv[i]))
//            {
//                ss << GetFloat(argv[i]);
//            }
//            else if (IsInt(argv[i]))
//            {
//                ss << GetInt(argv[i]);
//            }

//            if (i < argc-1) ss << " ";
//        }


//        auto str = ss.str();
//    }

    void RcpFormat::m_setId(int& id)
    {
        // check limits
        if (id == 0)
        {
            throw std::runtime_error("id can not be 0");
        }
        if (id > INT16_MAX)
        {
            throw std::runtime_error("id is too big");
        }
        if (id < INT16_MIN)
        {
            throw std::runtime_error("id is too small");
        }

        m_id = id;
    }

    // type
    void RcpFormat::setType(const t_symbol*& sym)
    {
        const char* t = GetString(sym);

        if (*t == 'f' || *t == 'F')
        {
            m_type = DATATYPE_FLOAT32;
        }
        else if (*t == 'i' || *t == 'I')
        {
            m_type = DATATYPE_INT32;
        }
        else if (*t == 't' || *t == 'T')
        {
            m_type = DATATYPE_BOOLEAN;
        }
        else if (*t == 's' || *t == 'S')
        {
            m_type = DATATYPE_STRING;
        }
        else if (*t == 'b' || *t == 'B')
        {
            m_type = DATATYPE_BANG;
        }
        else
        {
            m_type = DATATYPE_INVALID;
        }
    }
    void RcpFormat::getType(const t_symbol*& p)
    {
        switch(m_type)
        {
        case DATATYPE_FLOAT32:
            p = MakeSymbol("f");
            break;
        case DATATYPE_INT32:
            p = MakeSymbol("i");
            break;
        case DATATYPE_BOOLEAN:
            p = MakeSymbol("t");
            break;
        case DATATYPE_BANG:
            p = MakeSymbol("b");
            break;
        case DATATYPE_STRING:
            p = MakeSymbol("s");
            break;
        default:
            p = MakeSymbol("auto");
            break;
        }
    }

    // label
    void RcpFormat::setLabel(const t_symbol*& p)
    {
        m_label = GetString(p);

        rcp_option* opt = rcp_option_create(PARAMETER_OPTIONS_LABEL);
        rcp_option_copy_any_language(opt, m_label.c_str(), TINY_STRING);

        size_t size = rcp_option_get_size(opt, true);

        labelOption.resize(size + 2); // + 2 TERMINATOR

        if (rcp_option_write(opt, labelOption.data(), size, true) > 0)
        {
            labelOption[size] = RCP_TERMINATOR;
            labelOption[size+1] = RCP_TERMINATOR;
        }
        else
        {
            error("could not write label");
            labelOption.clear();
        }
    }

    void RcpFormat::getLabel(const t_symbol*& p)
    {
        p = MakeSymbol(m_label.c_str());
    }

    void RcpFormat::clearLabel()
    {
        m_label.clear();
    }

    FLEXT_LIB_V("rcp.format", RcpFormat);

}
