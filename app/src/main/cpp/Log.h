#ifndef _SIM_LOG_H_
#define _SIM_LOG_H_

class Log {
public:
    static void error(const char* pMessage, ...);
    static void warn(const char* pMessage, ...);
    static void info(const char* pMessage, ...);
    static void debug(const char* pMessage, ...);
};

#ifndef NDEBUG
    #define sim_Log_debug(...) Log::debug(__VA_ARGS__)
#else
    #define sim_Log_debug(...)
#endif

#endif
