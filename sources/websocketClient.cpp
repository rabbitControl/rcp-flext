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

#include "websocketClient.h"

#ifndef RCP_NO_SSL
#include <openssl/asn1.h>
#ifdef _WIN32
#define strcasecmp _stricmp
#endif
#endif

namespace rcp
{

websocketClient::websocketClient()
    : m_hostname(RABBITHOLE_HOSTNAME)
{
    // Set logging to be pretty verbose (everything except message payloads)
    m_client.clear_access_channels(websocketpp::log::alevel::all);
    m_client.clear_error_channels(websocketpp::log::alevel::all);
//    m_client.clear_access_channels(websocketpp::log::alevel::frame_payload);
    m_client.set_access_channels(websocketpp::log::alevel::frame_payload);


    // Initialize Asio Transport

    m_client.init_asio();
    m_client.start_perpetual();
    m_thread = websocketpp::lib::make_shared<websocketpp::lib::thread>(&client::run, &m_client);

#ifndef RCP_NO_SSL
    m_sslClient.clear_access_channels(websocketpp::log::alevel::all);
    m_sslClient.clear_error_channels(websocketpp::log::alevel::all);
//    m_sslClient.clear_access_channels(websocketpp::log::alevel::frame_payload);
    m_sslClient.set_tls_init_handler(bind(&websocketClient::on_tls_init, this, ::_1));

    m_sslClient.init_asio();
    m_sslClient.start_perpetual();
    m_sslThread = websocketpp::lib::make_shared<websocketpp::lib::thread>(&ssl_client::run, &m_sslClient);
#endif
}

websocketClient::~websocketClient()
{
    if (m_con) m_con->set_close_handler(nullptr);
#ifndef RCP_NO_SSL
    if (m_sslCon) m_sslCon->set_close_handler(nullptr);
#endif


    m_client.stop_perpetual();
#ifndef RCP_NO_SSL
    m_sslClient.stop_perpetual();
#endif


    disconnect();

    m_thread->join();
#ifndef RCP_NO_SSL
    m_sslThread->join();
#endif
}


void websocketClient::disconnect()
{
#ifndef RCP_NO_SSL
    if (m_sslCon)
    {
        m_sslCon->set_open_handler(nullptr);
        m_sslCon->set_fail_handler(nullptr);
        m_sslCon->set_message_handler(nullptr);

        if (m_sslCon->get_state() == websocketpp::session::state::open)
        {
            websocketpp::lib::error_code ec;
            m_sslCon->close(websocketpp::close::status::normal, "close", ec);
            if (ec) {
                std::cout << "closing failed: " << ec.message() << std::endl << std::endl;
            }
        }

        m_sslCon.reset();
    }
#endif

    if (m_con)
    {
        m_con->set_open_handler(nullptr);
        m_con->set_fail_handler(nullptr);
        m_con->set_message_handler(nullptr);

        if (m_con->get_state() == websocketpp::session::state::open)
        {
            websocketpp::lib::error_code ec;
            m_con->close(websocketpp::close::status::normal, "close", ec);
            if (ec) {
                std::cout << "closing failed: " << ec.message() << std::endl << std::endl;
            }
        }

        m_con.reset();
    }
}

void websocketClient::connect(const std::string& uri, const std::string& subprotocol)
{
    // close first?
    disconnect();

    if (uri.empty())
    {
        return;
    }

    if (uri.find("ws") != 0)
    {
        std::cout << "invalid url: " << uri << std::endl;
        return;
    }

    websocketpp::lib::error_code ec;
    if (uri.find("wss", 0) == 0)
    {
#ifndef RCP_NO_SSL
        m_sslCon = m_sslClient.get_connection(uri, ec);
        if (ec) {
            std::cout << "could not create connection: " << ec.message() << std::endl;
            m_sslCon.reset();
            return;
        }

        m_sslCon->set_open_handler(bind(&websocketClient::on_open, this, ::_1));
        m_sslCon->set_fail_handler(bind(&websocketClient::on_fail, this, ::_1));
        m_sslCon->set_close_handler(bind(&websocketClient::on_close, this, ::_1));
        m_sslCon->set_message_handler(bind(&websocketClient::on_message, this, ::_1, ::_2));

        if (!subprotocol.empty())
        {
            m_sslCon->add_subprotocol(subprotocol);
        }

        try {
            m_sslClient.connect(m_sslCon);
        }
        catch (const std::exception & e) {
            std::cout << "connect error: " << e.what() << std::endl;
        }
#else
        // unable to connect to wss without ssl
        std::cerr << "can not connect to secure websocket - no ssl" << std::endl;
#endif
    }
    else
    {
        m_con = m_client.get_connection(uri, ec);
        if (ec) {
            std::cout << "could not create connection: " << ec.message() << std::endl;
            m_con.reset();
            return;
        }

        m_con->set_open_handler(bind(&websocketClient::on_open, this, ::_1));
        m_con->set_fail_handler(bind(&websocketClient::on_fail, this, ::_1));
        m_con->set_close_handler(bind(&websocketClient::on_close, this, ::_1));
        m_con->set_message_handler(bind(&websocketClient::on_message, this, ::_1, ::_2));

        if (!subprotocol.empty())
        {
            m_con->add_subprotocol(subprotocol);
        }

        try {
            m_client.connect(m_con);
        }
        catch (const std::exception & e) {
            std::cout << "connect error: " << e.what() << std::endl;
        }
    }
}

bool websocketClient::isOpen() const
{
#ifndef RCP_NO_SSL
    if (m_sslCon)
    {
        return m_sslCon->get_state() == websocketpp::session::state::open;
    }
#endif

    if (m_con)
    {
        return m_con->get_state() == websocketpp::session::state::open;
    }

    return false;
}


void websocketClient::on_message(connection_hdl /*hdl*/, client::message_ptr msg)
{
    if (msg->get_opcode() == websocketpp::frame::opcode::value::binary)
    {
        const std::string & data = msg->get_raw_payload();
        received(const_cast<char*>(data.data()), data.size());
    }
    else
    {
        // got text message
        received(msg->get_payload());
    }
}

void websocketClient::send(char* data, size_t size)
{
#ifndef RCP_NO_SSL
    if (m_sslCon)
    {
        websocketpp::lib::error_code ec;
        m_sslClient.send(m_sslCon->get_handle(), data, size, websocketpp::frame::opcode::binary, ec);
        if (ec) {
            std::cout << "sending failed: " << ec.message() << std::endl << std::endl;
        }
    }
#endif

    if (m_con)
    {
        websocketpp::lib::error_code ec;
        m_client.send(m_con->get_handle(), data, size, websocketpp::frame::opcode::binary, ec);
        if (ec) {
            std::cout << "sending failed: " << ec.message() << std::endl << std::endl;
        }
    }
}


#ifndef RCP_NO_SSL

// ssl

context_ptr websocketClient::on_tls_init(websocketpp::connection_hdl)
{
    context_ptr ctx = websocketpp::lib::make_shared<websocketpp::lib::asio::ssl::context>(asio::ssl::context::sslv23);

    try
    {
        ctx->set_options(asio::ssl::context::default_workarounds |
                         asio::ssl::context::no_sslv2 |
                         asio::ssl::context::no_sslv3 |
                         asio::ssl::context::single_dh_use);

#ifdef RCP_VERIFY_SSL
        ctx->set_verify_mode(asio::ssl::verify_peer);
        ctx->set_verify_callback(bind(&websocketClient::verify_certificate, this, ::_1, ::_2));

        // Here we load the CA certificates of all CA's that this client trusts.
        ctx->load_verify_file("ca-chain.cert.pem");
#else
        ctx->set_verify_mode(asio::ssl::verify_none);
#endif

    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    return ctx;
}


#ifdef RCP_VERIFY_SSL
/// Verify that one of the subject alternative names matches the given hostname
bool websocketClient::verify_subject_alternative_name(X509 * cert)
{
    STACK_OF(GENERAL_NAME) * san_names = NULL;

    san_names = (STACK_OF(GENERAL_NAME) *) X509_get_ext_d2i(cert, NID_subject_alt_name, NULL, NULL);
    if (san_names == NULL) {
        return false;
    }

    int san_names_count = sk_GENERAL_NAME_num(san_names);

    bool result = false;

    for (int i = 0; i < san_names_count; i++) {
        const GENERAL_NAME * current_name = sk_GENERAL_NAME_value(san_names, i);

        if (current_name->type != GEN_DNS) {
            continue;
        }

        char const * dns_name = (char const *) ASN1_STRING_get0_data(current_name->d.dNSName);

        // Make sure there isn't an embedded NUL character in the DNS name
        if (size_t(ASN1_STRING_length(current_name->d.dNSName)) != strlen(dns_name)) {
            break;
        }
        // Compare expected hostname with the CN
        result = (strcasecmp(m_hostname.c_str(), dns_name) == 0);
    }
    sk_GENERAL_NAME_pop_free(san_names, GENERAL_NAME_free);

    return result;
}

/// Verify that the certificate common name matches the given hostname
bool websocketClient::verify_common_name(X509 * cert)
{
    // Find the position of the CN field in the Subject field of the certificate
    int common_name_loc = X509_NAME_get_index_by_NID(X509_get_subject_name(cert), NID_commonName, -1);
    if (common_name_loc < 0) {
        return false;
    }

    // Extract the CN field
    X509_NAME_ENTRY * common_name_entry = X509_NAME_get_entry(X509_get_subject_name(cert), common_name_loc);
    if (common_name_entry == NULL) {
        return false;
    }

    // Convert the CN field to a C string
    ASN1_STRING * common_name_asn1 = X509_NAME_ENTRY_get_data(common_name_entry);
    if (common_name_asn1 == NULL) {
        return false;
    }

    char const * common_name_str = (char const *) ASN1_STRING_get0_data(common_name_asn1);

    // Make sure there isn't an embedded NUL character in the CN
    if (size_t(ASN1_STRING_length(common_name_asn1)) != strlen(common_name_str)) {
        return false;
    }

    // Compare expected hostname with the CN
    return (strcasecmp(m_hostname.c_str(), common_name_str) == 0);
}

bool websocketClient::verify_certificate(bool preverified, asio::ssl::verify_context& ctx)
{
    // The verify callback can be used to check whether the certificate that is
    // being presented is valid for the peer. For example, RFC 2818 describes
    // the steps involved in doing this for HTTPS. Consult the OpenSSL
    // documentation for more details. Note that the callback is called once
    // for each certificate in the certificate chain, starting from the root
    // certificate authority.

    // Retrieve the depth of the current cert in the chain. 0 indicates the
    // actual server cert, upon which we will perform extra validation
    // (specifically, ensuring that the hostname matches. For other certs we
    // will use the 'preverified' flag from Asio, which incorporates a number of
    // non-implementation specific OpenSSL checking, such as the formatting of
    // certs and the trusted status based on the CA certs we imported earlier.
    int depth = X509_STORE_CTX_get_error_depth(ctx.native_handle());

    // if we are on the final cert and everything else checks out, ensure that
    // the hostname is present on the list of SANs or the common name (CN).
    if (depth == 0 && preverified) {
        X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());

        if (verify_subject_alternative_name(cert)) {
            return true;
        } else if (verify_common_name(cert)) {
            return true;
        } else {
            return false;
        }
    }

    return preverified;
}
#endif // RCP_VERIFY_SSL

#endif

}
