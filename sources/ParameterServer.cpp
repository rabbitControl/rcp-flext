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

#include "ParameterServer.h"

#include <string>
#include <vector>

#include <rcp_manager.h>
#include <rcp_parameter.h>
#include <rcp_typedefinition.h>

#include "RabbitholeServerTransporter.h"
#include "WebsocketServerTransporter.h"
#include "PdServerTransporter.h"
#include "Optional.h"

namespace rcp
{
    static void parameterValueUpdatedCb(rcp_value_parameter* parameter, void* user)
    {
        if (user)
        {
            ParameterServer* x = static_cast<ParameterServer*>(user);
            x->parameterUpdate(RCP_PARAMETER(parameter));
        }
    }
    static void bangCb(rcp_bang_parameter* param, void* user)
    {
        if (user)
        {
            ParameterServer* x = static_cast<ParameterServer*>(user);
            x->parameterUpdate(RCP_PARAMETER(param));
        }
    }

	ParameterServer::ParameterServer(int argc, t_atom *argv)
		: ParameterServerClientBase()
        , m_server(nullptr)
		, m_transporter(nullptr)
        , m_rabbitholeTransporter(nullptr)
        , m_raw(false)
        , m_clientCount(0)
	{
        // [rcp.server] - server without name and default port 10000
        // [rcp.server symbol] - server with name "symbol" and default port 10000
        //
        // option symbol: -raw - creates a raw-transporter (sending must be done in pd)
        //      per default internal websocket server is used

        for (int i=0; i<argc; i++)
        {
            if (IsString(argv[i]))
            {
                if (std::string(GetString(argv[i])) == "-raw")
                {
                    m_raw = true;
                    continue;
                }
            }
        }

        // ok to create that server

        m_server = rcp_server_create(NULL);

        if (m_server == nullptr)
        {
            throw std::runtime_error("could not create rcp server");
        }

        if (m_raw)
		{
            AddOutAnything();
            AddInAnything("raw data input", 1);
            FLEXT_ADDMETHOD(1, data_list);

            m_transporter = std::make_shared<PdServerTransporter>(this);

            if (m_transporter)
            {
                rcp_server_add_transporter(m_server, m_transporter->transporter());
            }
		}

        m_manager = rcp_server_get_manager(m_server);

        // set application id
        rcp_server_set_id(m_server, "pd rcp server ");
	}

	ParameterServer::~ParameterServer()
	{
		// free resources
		if (m_transporter)
		{
			m_transporter->unbind();
			m_transporter.reset();
		}

        if (m_rabbitholeTransporter)
        {
            m_rabbitholeTransporter.reset();
        }

        rcp_server_free(m_server);
	}

    void ParameterServer::connected(void* /*client*/)
    {
        m_clientCount++;
        ToOutInt(2, m_clientCount);
    }

    void ParameterServer::disconnected(void* /*client*/)
    {
        m_clientCount--;
        if (m_clientCount < 0)
        {
            error("client count < 0!");
            m_clientCount = 0;
        }

        ToOutInt(2, m_clientCount);
    }

	void ParameterServer::data_list(int argc, t_atom* argv)
	{
		if (m_transporter)
		{
            std::vector<char> data(argc);
			int offset = 0;

			for (int i=0; i<argc; i++)
			{
				if (IsFloat(argv[i]))
				{
					data[i-offset] = GetFloat(argv[i]);
				}
				else if (IsInt(argv[i]))
				{
					data[i-offset] = GetInt(argv[i]);
				}
				else
				{
					offset++;
				}
			}

			m_transporter->pushData(data.data(), argc - offset);
		}
        else
        {
            post("no transporter");
        }
	}

	void ParameterServer::dataOut(char* data, size_t data_size) const
	{
        std::vector<t_atom> atoms(data_size);
		
		for (size_t i=0; i<data_size; i++)
		{
			SetInt(atoms[i], data[i]);
		}
		
		ToOutList(0, data_size, atoms.data());
	}

    void ParameterServer::getPort(int& p)
    {
        if (m_transporter)
        {
            p = m_transporter->port();
        }
        else
        {
            p = 0;
        }
    }

    void ParameterServer::listen(int& p)
	{
        if (m_raw)
        {
            // setting port does not apply for raw servers
            return;
        }

        if (m_transporter &&
                m_transporter->isListening() &&
                p == m_transporter->port())
        {
            // port not changed
            return;
        }

        // check upper limit
        if (p > (int)UINT16_MAX)
        {
            error("invalid port: %d", p);
            return;
        }

        // check lower limit
        if (p > 0)
        {
            // create new transporter
            std::shared_ptr<IServerTransporter> new_transporter = std::make_shared<WebsocketServerTransporter>(m_server, this);

            if (new_transporter)
            {
                // set new transporter
                m_transporter = new_transporter;
                rcp_server_add_transporter(m_server, m_transporter->transporter());

                m_transporter->bind(p);

                // reset connected clients
                m_clientCount = 0;
                ToOutInt(2, m_clientCount);
            }
            else
            {
                error("could not create transporter!");
            }
        }
        else if (m_transporter)
        {
            // just remove this transporter
            // NOTE: transporter gets removed from rcp_server in desctructor
            m_transporter.reset();
        }
    }

    // rabbithole

    void ParameterServer::setRabbithole(const t_symbol*& uri)
    {
        if (m_server &&
                !m_rabbitholeTransporter)
        {
            m_rabbitholeTransporter = std::make_shared<RabbitHoleServerTransporter>(m_server);

            if (m_rabbitholeTransporter)
            {
                rcp_server_add_transporter(m_server, m_rabbitholeTransporter->transporter());
            }
        }

        if (m_rabbitholeTransporter)
        {
            m_rabbitholeTransporter->connect(std::string(GetAString(uri)));
        }
    }
    void ParameterServer::getRabbithole(const t_symbol*& uri)
    {
        if (m_rabbitholeTransporter)
        {
            uri = MakeSymbol(m_rabbitholeTransporter->uri().c_str());
        }
    }

    void ParameterServer::setRabbitholeInterval(const int& i)
    {
        if (m_rabbitholeTransporter)
        {
            m_rabbitholeTransporter->setInterval(i);
        }
    }
    void ParameterServer::getRabbitholeInterval(int& i)
    {
        if (m_rabbitholeTransporter)
        {
            i = m_rabbitholeTransporter->interval();
        }
    }


    // parameter
    void ParameterServer::exposeParameter(int argc, t_atom* argv)
    {
        // <type> <group> <group> ... <label>
        // options: @min @max @readonly @order

        if (argc < 2)
        {
            post("not enough arguments to expose parameter");
            return;
        }

        // check type
        rcp_datatype datatype = DATATYPE_INVALID;
        if (!IsString(argv[0]))
        {
            post("please provide a valid type (f, i, t, b, s)");
            return;
        }

        //
        const char* type_str = GetString(argv[0]);

        if (*type_str == 'f' || *type_str == 'F')
        {
            // TODO check pd/max float size!
            datatype = DATATYPE_FLOAT32;
        }
        else if (*type_str == 'i' || *type_str == 'I')
        {
            datatype = DATATYPE_INT32;
        }
        else if (*type_str == 't' || *type_str == 'T')
        {
            datatype = DATATYPE_BOOLEAN;
        }
        else if (*type_str == 'b' || *type_str == 'B')
        {
            datatype = DATATYPE_BANG;
        }
        else if (*type_str == 's' || *type_str == 'S')
        {
            datatype = DATATYPE_STRING;
        }


        // check datatype
        if (datatype == DATATYPE_INVALID)
        {
            post("unknown datatype: %s", type_str);
            return;
        }


        // arguments
        Optional<float> min;
        Optional<float> max;
        Optional<bool> readonly;
        Optional<int> order;

        // get options - look for first atom starting with @
        int args_index = argc;
        for (int i=2; i<argc; i++)
        {
            if (IsString(argv[i]))
            {
                std::string t = GetString(argv[i]);
                if (t[0] == '@')
                {
                    // set first arguments index
                    if (args_index == argc) args_index = i;

                    if (t == "@min")
                    {
                        i++;
                        if (i >= argc) break;

                        if (CanbeFloat(argv[i]))
                        {
                            min.set(GetFloat(argv[i]));
                        }
                        else
                        {
                            // error
                            error("can not set argument min!");
                        }
                    }
                    else if (t == "@max")
                    {
                        i++;
                        if (i >= argc) break;

                        if (CanbeFloat(argv[i]))
                        {
                            max.set(GetFloat(argv[i]));
                        }
                        else
                        {
                            // error
                            error("can not set argument max!");
                        }
                    }
                    else if (t == "@order")
                    {
                        i++;
                        if (i >= argc) break;

                        if (CanbeInt(argv[i]))
                        {
                            order.set(GetInt(argv[i]));
                        }
                        else
                        {
                            // error
                            error("can not set argument order!");
                        }
                    }
                    else if (t == "@readonly")
                    {
                        readonly.set(true);
                    }
                }
            }
        }


        // create necessary groups
        std::string label;
        rcp_group_parameter* group = createGroups(args_index-1, argv+1, label);

        if (label.empty())
        {
            // no label - can not create parameter
            error("please provide a label to expose a parameter");
            return;
        }

        // check if this label already exists in group
        rcp_parameter* param = rcp_manager_find_parameter(m_manager, label.c_str(), group);
        if (param != NULL)
        {
            error("parameter '%s' already exists", label.c_str());
            return;
        }


        // expose parameter
        switch (datatype)
        {
        case DATATYPE_FLOAT32:
        {
            rcp_value_parameter* p = rcp_server_expose_f32(m_server, label.c_str(), group);
            setupValueParameter(p);
            if (p != NULL)
            {
                if (min.isSet()) rcp_parameter_set_min_float(p, min.get());
                if (max.isSet()) rcp_parameter_set_max_float(p, max.get());
                if (order.isSet()) rcp_parameter_set_order(RCP_PARAMETER(p), order.get());
                if (readonly.isSet()) rcp_parameter_set_readonly(RCP_PARAMETER(p), readonly.get() > 0);

                // set default value
                rcp_parameter_set_value_float(p, 0);
            }
            else
            {
                post("could not create parameter");
            }
            break;
        }
        case DATATYPE_INT32:
        {
            rcp_value_parameter* p = rcp_server_expose_i32(m_server, label.c_str(), group);
            setupValueParameter(p);
            if (p != NULL)
            {
                if (min.isSet()) rcp_parameter_set_min_int32(p, (int32_t)min.get());
                if (max.isSet()) rcp_parameter_set_max_int32(p, (int32_t)max.get());
                if (order.isSet()) rcp_parameter_set_order(RCP_PARAMETER(p), order.get());
                if (readonly.isSet()) rcp_parameter_set_readonly(RCP_PARAMETER(p), readonly.get() > 0);

                // set default value
                rcp_parameter_set_value_int32(p, 0);
            }
            else
            {
                post("could not create parameter");
            }
            break;
        }
        case DATATYPE_BOOLEAN:
        {
            rcp_value_parameter* p = rcp_server_expose_bool(m_server, label.c_str(), group);
            setupValueParameter(p);
            if (p != NULL)
            {
                if (order.isSet()) rcp_parameter_set_order(RCP_PARAMETER(p), order.get());
                if (readonly.isSet()) rcp_parameter_set_readonly(RCP_PARAMETER(p), readonly.get() > 0);

                // set default value
                rcp_parameter_set_value_bool(p, false);
            }
            else
            {
                post("could not create parameter");
            }
            break;
        }
        case DATATYPE_STRING:
        {
            rcp_value_parameter* p = rcp_server_expose_string(m_server, label.c_str(), group);
            setupValueParameter(p);
            if (p != NULL)
            {
                if (order.isSet()) rcp_parameter_set_order(RCP_PARAMETER(p), order.get());
                if (readonly.isSet()) rcp_parameter_set_readonly(RCP_PARAMETER(p), readonly.get() > 0);

                // set default value
                rcp_parameter_set_value_string(p, "");
            }
            else
            {
                post("could not create parameter");
            }
            break;
        }
        case DATATYPE_BANG:
        {
            rcp_bang_parameter* p = rcp_server_expose_bang(m_server, label.c_str(), group);
            if (p != NULL)
            {
                if (order.isSet()) rcp_parameter_set_order(RCP_PARAMETER(p), order.get());
                if (readonly.isSet()) rcp_parameter_set_readonly(RCP_PARAMETER(p), readonly.get() > 0);

                rcp_parameter_set_user(RCP_PARAMETER(p), this);
                rcp_bang_parameter_set_bang_cb(p, bangCb);
            }
            else
            {
                post("could not create parameter");
            }
            break;
        }
        }

        rcp_server_update(m_server);
    }


    rcp_group_parameter* ParameterServer::createGroups(int argc, t_atom* argv, std::string& outLabel)
    {
        rcp_group_parameter* lastGroup = nullptr;
        for (int i=0; i<argc-1; i++)
        {
            if (!IsString(argv[i]))
            {
                //
                post("can not create group with non-string name");
                outLabel = "";
                return nullptr;
            }

            std::string group_name = GetAsString(argv[i]);            
            rcp_group_parameter* group = rcp_server_find_group(m_server, group_name.c_str(), lastGroup);

            if (group == nullptr)
            {
                // create group
                group = rcp_server_create_group(m_server, group_name.c_str(), lastGroup);
            }

            lastGroup = group;
        }

        outLabel = GetAsString(argv[argc-1]);
        return lastGroup;
    }

    void ParameterServer::setupValueParameter(rcp_value_parameter* parameter)
    {
        if (parameter)
        {
            rcp_parameter_set_user(RCP_PARAMETER(parameter), this);
            rcp_parameter_set_value_updated_cb(parameter, parameterValueUpdatedCb);
        }
    }

    void ParameterServer::removeParameter(int id)
    {
        if (rcp_server_remove_parameter_id(m_server, id))
        {
            rcp_server_update(m_server);
        }
    }

    void ParameterServer::removeParameterList(int argc, t_atom* argv)
    {
        rcp_parameter* param = getParameter(argc, argv);
        if (param)
        {
            removeParameter(rcp_parameter_get_id(param));
        }
    }

    void ParameterServer::parameterSetReadonly(int argc, t_atom* argv)
    {
        if (!CanbeInt(argv[argc-1]))
        {
            error("rcp set readonly - invalid data");
            return;
        }

        rcp_parameter* parameter = getParameter(argc-1, argv);
        if (parameter)
        {
            rcp_parameter_set_readonly(parameter, GetInt(argv[argc-1]) > 0);
            rcp_server_update(m_server);
        }
    }

    void ParameterServer::parameterSetOrder(int argc, t_atom* argv)
    {
        if (!CanbeInt(argv[argc-1]))
        {
            error("rcp set order - invalid data");
            return;
        }

        rcp_parameter* parameter = getParameter(argc-1, argv);
        if (parameter)
        {
            rcp_parameter_set_order(parameter, GetInt(argv[argc-1]));
            rcp_server_update(m_server);
        }
    }

    void ParameterServer::parameterSetMin(int argc, t_atom* argv)
    {
        rcp_parameter* parameter = getParameter(argc-1, argv);
        if (parameter)
        {
            rcp_datatype type = RCP_TYPE_ID(parameter);
            if (type == DATATYPE_FLOAT32)
            {
                if (CanbeFloat(argv[argc-1]))
                {
                    rcp_parameter_set_min_float(RCP_VALUE_PARAMETER(parameter), GetFloat(argv[argc-1]));
                    rcp_server_update(m_server);
                }
            }
            else if (type == DATATYPE_INT32)
            {
                if (CanbeInt(argv[argc-1]))
                {
                    rcp_parameter_set_min_int32(RCP_VALUE_PARAMETER(parameter), GetInt(argv[argc-1]));
                    rcp_server_update(m_server);
                }
            }
        }
    }

    void ParameterServer::parameterSetMax(int argc, t_atom* argv)
    {
        rcp_parameter* parameter = getParameter(argc-1, argv);
        if (parameter)
        {
            rcp_datatype type = RCP_TYPE_ID(parameter);
            if (type == DATATYPE_FLOAT32)
            {
                if (CanbeFloat(argv[argc-1]))
                {
                    rcp_parameter_set_max_float(RCP_VALUE_PARAMETER(parameter), GetFloat(argv[argc-1]));
                    rcp_server_update(m_server);
                }
            }
            else if (type == DATATYPE_INT32)
            {
                if (CanbeInt(argv[argc-1]))
                {
                    rcp_parameter_set_max_int32(RCP_VALUE_PARAMETER(parameter), GetInt(argv[argc-1]));
                    rcp_server_update(m_server);
                }
            }
        }
    }

    void ParameterServer::parameterSetMinMax(int argc, t_atom* argv)
    {
        if (argc < 3 ||
                !CanbeFloat(argv[argc-2]) ||
                !CanbeFloat(argv[argc-1]))
        {
            error("invalid data for setting min and max");
            return;
        }

        rcp_parameter* parameter = getParameter(argc-2, argv);
        if (parameter)
        {
            rcp_datatype type = RCP_TYPE_ID(parameter);
            if (type == DATATYPE_FLOAT32)
            {
                rcp_parameter_set_min_float(RCP_VALUE_PARAMETER(parameter), GetFloat(argv[argc-2]));
                rcp_parameter_set_max_float(RCP_VALUE_PARAMETER(parameter), GetFloat(argv[argc-1]));
                rcp_server_update(m_server);
            }
            else if (type == DATATYPE_INT32)
            {
                rcp_parameter_set_min_int32(RCP_VALUE_PARAMETER(parameter), GetInt(argv[argc-2]));
                rcp_parameter_set_max_int32(RCP_VALUE_PARAMETER(parameter), GetInt(argv[argc-1]));
                rcp_server_update(m_server);
            }
        }
    }

    FLEXT_LIB_V("rcp.server", ParameterServer);
}
