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

#include "ParameterClient.h"

#include <vector>

#include <rcp_parameter.h>
#include <rcp_typedefinition.h>
#include <rcp_logging.h>

#include "WebsocketClientTransporter.h"


namespace rcp
{

    static void client_parameter_added_cb(rcp_parameter* parameter, void* user)
    {
        if (user)
        {
            ParameterClient* client = (ParameterClient*)user;
            client->parameterAdded(parameter);
        }
    }

    static void client_parameter_removed_cb(rcp_parameter* parameter, void* user)
    {
        if (user)
        {
            ParameterClient* client = (ParameterClient*)user;
            client->parameterRemoved(parameter);
        }
    }

    static void parameterValueUpdatedCb(rcp_value_parameter* parameter, void* user)
    {
        if (user)
        {
            RCP_DEBUG("parameterValueUpdatedCb: %p\n", user);
            ParameterClient* pd_client = (ParameterClient*)user;
            pd_client->parameterUpdate(RCP_PARAMETER(parameter));
        }
    }

    static void bangParameterBangCb(rcp_bang_parameter* parameter, void* user)
    {
        if (user)
        {
            ParameterClient* pd_client = (ParameterClient*)user;
            pd_client->parameterUpdate(RCP_PARAMETER(parameter));
        }
    }



    ParameterClient::ParameterClient()
        : ParameterServerClientBase()
        , m_client(nullptr)
        , m_transporter(nullptr)
    {
        m_transporter = new WebsocketClientTransporter(this);

        if (m_transporter)
        {
            m_client = rcp_client_create(m_transporter->transporter());

            if (m_client)
            {
                m_manager = rcp_client_get_manager(m_client);
                rcp_client_set_id(m_client, "pd rcp client");

                rcp_client_set_user(m_client, this);
                rcp_client_set_parameter_added_cb(m_client, client_parameter_added_cb);
                rcp_client_set_parameter_removed_cb(m_client, client_parameter_removed_cb);
            }
        }
    }

    ParameterClient::~ParameterClient()
    {
        if (m_client)
        {
            rcp_client_free(m_client);
            m_client = nullptr;
        }

        if (m_transporter)
        {
            delete m_transporter;
            m_transporter = nullptr;
        }
    }

    void ParameterClient::m_open(const t_symbol *d)
    {
        if (m_transporter)
        {
            m_transporter->open(std::string(GetString(d)));
        }
    }

    void ParameterClient::m_close()
    {
        if (m_transporter)
        {
            m_transporter->close();
        }
    }



    void ParameterClient::parameterAdded(rcp_parameter* parameter)
    {
        const char* label = rcp_parameter_get_label(parameter);
        uint16_t id = rcp_parameter_get_id(parameter);

        // get the parents
        std::vector<std::string> groups = getParents(parameter);

        RCP_DEBUG("add - parents: %d\n", groups.size());

        // output [list]
        // add group1 groupN... label value

        // TODO: append userid?

        int len = 3 + groups.size();
        std::vector<t_atom> list(len);

        int i=0;
        SetSymbol(list[i], MakeSymbol("add"));
        i++;

        for (std::vector<std::string>::reverse_iterator rit = groups.rbegin();
            rit != groups.rend(); ++rit)
        {
            SetString(list[i], rit->c_str());
            i++;
        }

        SetSymbol(list[i], MakeSymbol(label != NULL ? label : "<nolabel>"));
        i++;


        // set this as user
        rcp_parameter_set_user(parameter, this);
        // get type
        rcp_datatype type = rcp_typedefinition_get_type_id(rcp_parameter_get_typedefinition(parameter));

        if (rcp_parameter_is_value(parameter))
        {
            //
            RCP_DEBUG("---- set value parameter: %d - this: %p\n", id, this);
            rcp_parameter_set_value_updated_cb(RCP_VALUE_PARAMETER(parameter), parameterValueUpdatedCb);

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
                const char* value = rcp_parameter_get_value_string(RCP_VALUE_PARAMETER(parameter));
                SetString(list[i], (value != NULL ? value : ""));
                i++;
            }
        }
        else if (type == DATATYPE_BANG)
        {
            rcp_bang_parameter_set_bang_cb(RCP_BANG_PARAMETER(parameter), bangParameterBangCb);

            //
            SetSymbol(list[i], MakeSymbol("[bang]"));
            i++;
        }
        else if (type == DATATYPE_GROUP)
        {
            //
            SetSymbol(list[i], MakeSymbol("[group]"));
            i++;
        }

        ToOutInt(1, id);
        ToOutList(0, i, list.data());
    }

    void ParameterClient::parameterRemoved(rcp_parameter* parameter)
    {
        const char* label = rcp_parameter_get_label(parameter);
        uint16_t id = rcp_parameter_get_id(parameter);

        // get the parents
        std::vector<std::string> groups = getParents(parameter);

        // output [list]
        // remove group1 groupN... label

        int len = 2 + groups.size();
        std::vector<t_atom> list(len);

        int i=0;
        SetSymbol(list[i], MakeSymbol("remove"));
        i++;

        for (std::vector<std::string>::reverse_iterator rit = groups.rbegin();
            rit != groups.rend(); ++rit)
        {
            SetString(list[i], rit->c_str());
            i++;
        }

        SetSymbol(list[i], MakeSymbol(label != NULL ? label : "<nolabel>"));
        i++;

        ToOutInt(1, id);
        ToOutList(0, i, list.data());
    }


    // IWebsocketClientListener
    void ParameterClient::connected()
    {
        ToOutInt(2, 1);
    }

    void ParameterClient::disconnected()
    {
        // NOTE: this should be called after client_transporter_call_disconnected_cb
        // client manager was re-created: get the new one
//        m_manager = client_get_manager(m_client);

        ToOutInt(2, 0);
    }

    FLEXT_LIB("rcp.client", ParameterClient);
}
