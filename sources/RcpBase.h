#ifndef RCPBASE_H
#define RCPBASE_H

namespace rcp
{
    class RcpBase
    {
    public:
        static void postVersion();
        static void rabbitPost(const char* msg);
        static void rabbitPostOneline(const char* msg);
        static void postRabbitcontrolInit();

        // config
        static bool debugLogging;
    };

}

#endif // RCPBASE_H
