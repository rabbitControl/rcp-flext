#ifndef PDCLIENTTRANSPORTER_H
#define PDCLIENTTRANSPORTER_H

#include <rcp_client_transporter.h>

#include "IClientTransporter.h"

namespace rcp
{
    class ParameterClient;

    class PdClientTransporter : public IClientTransporter
    {
    public:
        PdClientTransporter(ParameterClient* client);
        ~PdClientTransporter();

        ParameterClient* pdClient() const;

    public:
        // IClientTransporter
        rcp_client_transporter* transporter() const override;
        void open(const std::string& /*address*/) override {}
        void close() override {}
        void pushData(char* /*data*/, size_t /*size*/) const override;

    private:
        ParameterClient* m_pdClient{nullptr};
        rcp_client_transporter* m_transporter;
    };
}

#endif // PDCLIENTTRANSPORTER_H
