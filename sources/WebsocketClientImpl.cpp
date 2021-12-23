#include "WebsocketClientImpl.h"

namespace rcp
{

    WebsocketClientImpl::WebsocketClientImpl(IWebsocketClientListener* listener)
        : websocketClient()
        , m_listener(listener)
    {
    }

    // websocketClient
    void WebsocketClientImpl::connected()
    {
        if (m_listener)
        {
            m_listener->connected();
        }
    }

    void WebsocketClientImpl::disconnected()
    {
        if (m_listener)
        {
            m_listener->disconnected();
        }
    }

    void WebsocketClientImpl::received(char* data, size_t size)
    {
        if (m_listener)
        {
            m_listener->received(data, size);
        }
    }

    void WebsocketClientImpl::received(const std::string& msg)
    {
        if (m_listener)
        {
            m_listener->received(msg);
        }
    }
}
