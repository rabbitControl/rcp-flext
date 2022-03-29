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

#ifndef RABBITCONTROL_WEBSOCKET_SERVER_H
#define RABBITCONTROL_WEBSOCKET_SERVER_H

#include <set>
#include <iostream>

// NOTE: needed to figure out ASIO_STANDALONE before including websocketpp
#include <asio.hpp>

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/common/thread.hpp>

// rcp
#include <rcp_server.h>

typedef websocketpp::server<websocketpp::config::asio> server;

using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

using websocketpp::lib::thread;
using websocketpp::lib::mutex;
using websocketpp::lib::lock_guard;
using websocketpp::lib::unique_lock;
using websocketpp::lib::condition_variable;

/* on_open insert connection_hdl into channel
 * on_close remove connection_hdl from channel
 * on_message queue send to all channels
 */

namespace rcp
{

    enum action_type {
        SUBSCRIBE,
        UNSUBSCRIBE,
        MESSAGE
    };

    struct action {
        action(action_type t, connection_hdl h) : type(t), hdl(h) {}
        action(action_type t, connection_hdl h, server::message_ptr m)
          : type(t), hdl(h), msg(m) {}

        action_type type;
        websocketpp::connection_hdl hdl;
        server::message_ptr msg;
    };

    class IWebsocketServerListener
    {
    public:
        virtual void connected(void* client) = 0;
        virtual void disconnected(void* client) = 0;
        virtual void received(char* data, size_t size, void* id) = 0;
        virtual void socketerror(const char* /*reason*/) {};
    };


    class websocketServer : public IWebsocketServerListener
    {
    public:
        websocketServer()
            : m_port(0)
            , ws_thread(nullptr)
            , server_thread(nullptr)
            , m_run(false)
        {
            // Initialize Asio Transport
            m_server.init_asio();
            m_server.set_reuse_addr(true);

            m_server.clear_access_channels(websocketpp::log::alevel::all);
            m_server.clear_error_channels(websocketpp::log::alevel::all);

            // Register handler callbacks
            m_server.set_open_handler(bind(&websocketServer::on_open,this,::_1));
            m_server.set_close_handler(bind(&websocketServer::on_close,this,::_1));
            m_server.set_message_handler(bind(&websocketServer::on_message,this,::_1,::_2));

            // start service thread
            ws_thread = new websocketpp::lib::thread(std::bind(&websocketServer::process_messages, this));
        }

        virtual ~websocketServer()
        {
            stop();
        }

        uint16_t port() const {
            return m_port;
        }

        void run(uint16_t port)
        {
            m_port = port;

            server_thread = new thread(bind(&websocketServer::_do_run, this));

            if (!ws_thread)
            {
                // start service thread
                ws_thread = new websocketpp::lib::thread(std::bind(&websocketServer::process_messages, this));
            }
        }

        void _do_run()
        {
            try
            {
                // listen on specified port
                m_server.listen(m_port);

                // Start the server accept loop
                m_server.start_accept();

                // Start the ASIO io_service run loop
                m_server.run();
            } catch (const std::exception & e) {
                const char* r = e.what();
                std::cout << r << std::endl;
                socketerror(r);
            }
        }

        void stop()
        {
            if (!m_server.stopped())
            {
                m_server.stop();
            }

            if (m_server.is_listening())
            {
                m_server.stop_listening();
            }

            // close all connections
            m_connections.clear();

            if (server_thread)
            {
                server_thread->join();

                delete server_thread;
                server_thread = nullptr;
            }

            if (ws_thread)
            {
                {
                    lock_guard<websocketpp::lib::mutex> guard(m_action_lock);
                    m_run = false;
                }

                m_action_cond.notify_one();
                ws_thread->join();

                delete ws_thread;
                ws_thread = nullptr;
            }
        }

        void on_open(connection_hdl hdl)
        {
            {
                lock_guard<mutex> guard(m_action_lock);
                //std::cout << "on_open" << std::endl;
                m_actions.push(action(SUBSCRIBE,hdl));
            }
            m_action_cond.notify_one();
        }

        void on_close(connection_hdl hdl)
        {
            {
                lock_guard<mutex> guard(m_action_lock);
                //std::cout << "on_close" << std::endl;
                m_actions.push(action(UNSUBSCRIBE,hdl));
            }
            m_action_cond.notify_one();
        }

        void on_message(connection_hdl hdl, server::message_ptr msg)
        {
            // queue message up for sending by processing thread
            {
                lock_guard<mutex> guard(m_action_lock);
                //std::cout << "on_message" << std::endl;
                m_actions.push(action(MESSAGE,hdl,msg));
            }
            m_action_cond.notify_one();
        }

        void process_messages()
        {
            m_run = true;

            while(m_run)
            {
                unique_lock<mutex> lock(m_action_lock);

                while(m_run
                      && m_actions.empty())
                {
                    m_action_cond.wait(lock);
                }

                if (!m_run)
                {
                    break;
                }

                action a = m_actions.front();
                m_actions.pop();

                lock.unlock();

                if (a.type == SUBSCRIBE)
                {
                    lock_guard<mutex> guard(m_connection_lock);
                    m_connections.insert(a.hdl);

                    connected(nullptr);
                }
                else if (a.type == UNSUBSCRIBE)
                {
                    lock_guard<mutex> guard(m_connection_lock);
                    m_connections.erase(a.hdl);

                    disconnected(nullptr);
                }
                else if (a.type == MESSAGE)
                {
                    lock_guard<mutex> guard(m_connection_lock);

                    if (a.msg->get_opcode() == websocketpp::frame::opcode::value::binary)
                    {
                        if (auto ptr = a.hdl.lock())
                        {
                            const std::string & data = a.msg->get_raw_payload();

                            received(const_cast<char*>(data.data()), data.size(), ptr.get());
                        }
                    }
                    else
                    {
                        // got text message
                        std::cout << "websocket: got text message: " << a.msg->get_payload() << std::endl;
                    }

                } else {
                    // undefined.
                }
            }

            // done
        }

    protected:
        typedef std::set<connection_hdl, std::owner_less<connection_hdl> > con_list;

        con_list m_connections;
        server m_server;
        uint16_t m_port;

    private:
        std::queue<action> m_actions;

        mutex m_action_lock;
        mutex m_connection_lock;
        condition_variable m_action_cond;

        websocketpp::lib::thread *ws_thread;
        websocketpp::lib::thread *server_thread;
        std::atomic_bool m_run;
    };

}

#endif
