#ifndef WEBSOCKETCLIENTIMPL_H
#define WEBSOCKETCLIENTIMPL_H

#include "websocketClient.h"

namespace rcp
{
    class WebsocketClientImpl : public websocketClient
    {
    public:
        WebsocketClientImpl(IWebsocketClientListener* listener);

    public:
        // websocketClient
        void connected() override;
        void disconnected() override;
        void received(char* data, size_t size) override;
        void received(const std::string& msg) override;

    private:
        IWebsocketClientListener* m_listener{nullptr};
    };
}

#endif // WEBSOCKETCLIENTIMPL_H
