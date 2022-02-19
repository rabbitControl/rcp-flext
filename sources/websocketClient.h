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

#ifndef RABBITCONTROL_WEBSOCKET_CLIENT_H
#define RABBITCONTROL_WEBSOCKET_CLIENT_H

// NOTE: needed to figure out ASIO_STANDALONE before including websocketpp
#include <asio.hpp>

#include <websocketpp/client.hpp>
#include <websocketpp/common/thread.hpp>

#ifndef RCP_NO_SSL
#include <asio/ssl.hpp>
#include <websocketpp/config/asio_client.hpp>
typedef websocketpp::client<websocketpp::config::asio_tls_client> ssl_client;
typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;
#else
#include <websocketpp/config/asio_no_tls_client.hpp>
#endif

#define RABBITHOLE_HOSTNAME "rabbithole.rabbitcontrol.cc"


typedef websocketpp::client<websocketpp::config::asio_client> client;


using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

// pull out the type of messages sent by our config
typedef websocketpp::config::asio_client::message_type::ptr message_ptr;

using websocketpp::lib::thread;


namespace rcp
{

    class IWebsocketClientListener
    {
    public:
        //
        virtual void connected() = 0;
        virtual void disconnected() = 0;
        virtual void received(char* data, size_t size) = 0;
        virtual void received(const std::string& msg) = 0;
    };


    class websocketClient : public IWebsocketClientListener
    {

    public:

        websocketClient();
        virtual ~websocketClient();

    public:
        void on_open(websocketpp::connection_hdl /*hdl*/)
        {
            connected();
        }

        void on_fail(websocketpp::connection_hdl /*hdl*/)
        {
            disconnected();
        }

        void on_close(websocketpp::connection_hdl /*hdl*/)
        {
            disconnected();
        }

        bool isOpen() const;

        virtual void connect(const std::string& uri, const std::string& subprotocol = "");
        void disconnect();

        void on_message(connection_hdl hdl, client::message_ptr msg);
        void send(char* data, size_t size);

    #ifndef RCP_NO_SSL
        // SSL
        context_ptr on_tls_init(websocketpp::connection_hdl);

        #ifdef RCP_VERIFY_SSL
        bool verify_subject_alternative_name(X509 * cert);
        bool verify_common_name(X509 * cert);
        bool verify_certificate(bool preverified, asio::ssl::verify_context& ctx);
        #endif
    #endif

    private:
        std::string m_hostname;

        client m_client;
        websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_thread;
        client::connection_ptr m_con;

    #ifndef RCP_NO_SSL
        // ssl
        ssl_client m_sslClient;
        websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_sslThread;
        ssl_client::connection_ptr m_sslCon;
    #endif
    };

}

#endif
