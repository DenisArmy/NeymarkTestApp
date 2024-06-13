#ifdef HAVE_CONFIG_H
    #include "Config.h"
#endif


#include "Logger.h"

// log level names are in exact increasing order as corresponding LOG_* constants
const char * log_levels_table[] = 
    {"EMERG", "ALERT", "CRIT", "ERROR", "WARNING", "NOTICE", "INFO", "DEBUG"};

syslog_t g_syslog;

syslog_t& syslog_get()
{
    return g_syslog;
}

const char* get_log_level_name(const int prio)
{
    if (prio >= 0) {
        if (prio > LOG_DEBUG) {
            return "DEBUG";
        }
        else {
            return log_levels_table[prio];
        }
    }

    return "UNKNOWN";
}