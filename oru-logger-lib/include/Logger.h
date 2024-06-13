#pragma once

#include <syslog.h>
#ifndef SYSLOGPP_DEFAULT_PRIO
    #define SYSLOGPP_DEFAULT_PRIO LOG_ALERT
#endif

#include <stdarg.h>
#include <stdexcept>
#include <sstream>

const char* get_log_level_name(const int prio);

class syslogbuf:
    public std::stringbuf
{
protected:
                int     m_prio;

inline          int     sync();
public:
inline                  syslogbuf(int prio);
inline          void    setprio(int prio);
inline          size_t  size() const;
inline          bool    empty() const;
inline          void    clear();
};

class syslog_t:
    protected syslogbuf,
    public std::ostream
{
public:
inline                  syslog_t(int prio = SYSLOGPP_DEFAULT_PRIO);
inline                  ~syslog_t();

//operator of priority modification
inline  std::ostream&   operator () (int prio);

//standard interface
inline  void            open(const char* procname, int option, int facility);
inline  void            close();
inline  int             setmask(int mask);
inline  void            operator () (int prio, const char* fmt, ...) const;
};

syslogbuf::syslogbuf(int prio):
    std::stringbuf(std::ios_base::out),
    m_prio(prio)
{
}

void syslogbuf::setprio(int prio)
{
    pubsync();
    m_prio = prio;
}

size_t syslogbuf::size() const
{
    return pptr() - pbase();
}

bool syslogbuf::empty() const
{
    return 0 == size();
}

void syslogbuf::clear()
{
    setp(pbase(), epptr());
}

int syslogbuf::sync()
{
    if (!empty()) {
        sputc('\0');
        syslog(m_prio, "%s: %s", get_log_level_name(m_prio), pbase());
        clear();
    }
    return 0; //success
}

syslog_t::syslog_t(int prio):
    syslogbuf(prio), std::ostream(static_cast<std::streambuf*>(this))
{
}

syslog_t::~syslog_t()
{
    pubsync();
}

void syslog_t::open(const char* procname, int option, int facility)
{
    openlog(procname, option, facility);
}

void syslog_t::close()
{
    closelog();
}

int syslog_t::setmask(int mask)
{
    return setlogmask(mask);
}

std::ostream& syslog_t::operator () (const int prio)
{
    setprio(prio);
    return *this;
}

void syslog_t::operator () (int prio, const char* fmt, ...) const
{
    va_list args;
    va_start(args, fmt);
    vsyslog(prio, fmt, args);
    va_end(args);
}

syslog_t& syslog_get();
#define syslog syslog_get()
