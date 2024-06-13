#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>

#include <getopt.h>

#include <execinfo.h>
#include <signal.h>
#include <stdio.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <syslog.h>
#include <unistd.h>
#include <string.h>

#include "DaemonLib.h"
#include "Logger.h"

using namespace std;

namespace ORU
{
/**
 * \brief Pointer to class instance
 */
OruDaemonBase* OruDaemonBase::_instance{nullptr};

/**
 * \brief Daemon base class constructor
 * \param[in] id - daemon codename
 */
OruDaemonBase::OruDaemonBase(const std::string& daemonName) : _name{daemonName}
{
    _instance = this;
    stringstream ss;
    ss << "//tmp//" << _name << ".pid";
    _pidFileName = ss.str();

    // clear stream
    ss.str(std::string());

    ss << "[" << _name << "]";
    _moduleName = ss.str();
}

/**
 * \brief Daemon base class destructor
 */
OruDaemonBase::~OruDaemonBase()
{
}

/**
 * \brief Shows usage option selector
 */
void OruDaemonBase::usage()
{
    cout << "usage: " << _name << " [-fnqh]" << endl;
    cout << "    -f, --fore" << endl;
    cout << "    -n, --normal" << endl;
    cout << "    -q, --quit" << endl;
    cout << "    -h, --help" << endl;
}

/**
 * \brief Parsing input options
 * \param[in] argc - number of options
 * \param[in] argv - list of options
 * \retval ::DaemonInputOption
 */
DaemonInputOption OruDaemonBase::parseArgs(int argc, char** argv)
{
    int c = 0;
    DaemonInputOption opt = DM_OPT_UNKNOWN;

    static const struct option lopts[] = {
        {"fore", 0, 0, 'f'},
        {"normal", 0, 0, 'n'},
        {"quit", 0, 0, 'q'},
        {"help", 0, 0, 'h'},
        {0, 0, 0, 0}};

    while (1)
    {
        c = getopt_long(argc, argv, "dfnqh", lopts, NULL);

        if (c == -1)
        {
            break;
        }

        switch (c)
        {
        case 'f':
            opt = DM_OPT_FOREGROUND;
            break;
        case 'n':
            opt = DM_OPT_NORMAL;
            break;
        case 'q':
            opt = DM_OPT_QUIT;
            break;
        default:
            break;
        }
    }

    return opt;
}

/**
 * \brief Logger initializer
 */
void OruDaemonBase::initLog()
{
    openlog(_name.c_str(), LOG_CONS | LOG_PID, LOG_LOCAL0);
}

/**
 * \brief Daemon version
 * \param[in] bCout - cout or log
 */
void OruDaemonBase::logVersion(bool bCout)
{
    if (bCout)
    {
        cout << _name << " version: " << _version << endl;
    }
    else
    {
        syslog(LOG_INFO) << "Daemons: ";
        syslog(LOG_INFO) << _name << " version: " << _version;
    }
}

/**
 * \brief PID getter from filename
 * \param[in] fileName - string w/ filename
 * \retval pid - integer w/ process id
 */
int OruDaemonBase::getPidFromFile(const string& fileName)
{
    int pid = -1;
    string line;
    ifstream i(fileName);

    // header
    if (!getline(i, line))
    {
        return -1;
    }

    istringstream in(line);

    in >> pid;

    return pid;
}

/**
 * \brief PID setter to filename
 * \retval ::ReturnCode
 */
ReturnCode OruDaemonBase::setPidToFile()
{
    std::ofstream o(_pidFileName);

    if (o.is_open())
    {
        o << getpid();
        return ReturnCode::OK;
    }
    else
    {
        syslog(LOG_ERR) << _moduleName << " Set PID to file failed";
        return ReturnCode::FAILED;
    }
}

/**
 * \brief Checking process method
 * \retval ::ReturnCode
 */
ReturnCode OruDaemonBase::checkProcess()
{
    int rc;
    // get pid from file
    int pid = getPidFromFile(_pidFileName);
    if (pid > 0)
    {
        // check process
        if (0 != (rc = kill(pid, 0)))
        {
            // daemon is not running but pid file exists
            stringstream ss;
            ss << " (" << _pidFileName << ")";

            // glog is not started yet
            syslog(LOG_INFO) << _moduleName << " It is not running but pid file" << ss.str() << " does exist";
            rc = unlink(_pidFileName.c_str());
            if (rc < 0)
            {
                syslog(LOG_ERR) << _moduleName << " Cannot delete pid file" << ss.str();
                return ReturnCode::FAILED;
            }
        }
        else
        {
            if (_startOption == DM_OPT_QUIT)
            {
                rc = kill(pid, SIGQUIT);
                if (rc)
                {
                    syslog(LOG_ERR) << _moduleName << " Failed to stop daemon, pid = " << pid;
                }
                else
                {
                    syslog(LOG_INFO) << _moduleName << " Stop, pid = " << pid;
                }
                return ReturnCode::IS_INTERRUPTED;
            }
            else
            {
                // daemon is running
                syslog(LOG_INFO) << _moduleName << " was already started, pid = " << pid;
                return ReturnCode::IS_RUNNING;
            }
        }
    }
    else
    {
        if (_startOption == DM_OPT_QUIT)
        {
            syslog(LOG_INFO) << _moduleName << " It is not running";
            return ReturnCode::IS_INTERRUPTED;
        }
    } // if (pid > 0)
    return ReturnCode::OK;
}

/**
 * \brief Set file descriptor limit
 * \retval  0 - success
 * \retval -1 - failure
 */
int OruDaemonBase::setfileDescriptorLimit()
{
    // current and max limits for open descriptors
    struct rlimit lim = {(rlim_t)_fdLimit, (rlim_t)_fdLimit};

    int status = setrlimit(RLIMIT_NOFILE, &lim);

    if (status)
    {
        syslog(LOG_ERR) << _moduleName << " Set file descriptor's limit failed";
    }

    return status;
}

/**
 * \brief Set stop signal handlers
 */
void OruDaemonBase::setStopSignalHandlers()
{
    struct sigaction sigact = {0};

    sigact.sa_flags = SA_SIGINFO;
    sigact.sa_sigaction = OruDaemonBase::stopSignalHandler;

    sigemptyset(&sigact.sa_mask);

    sigaction(SIGQUIT, &sigact, 0);
    sigaction(SIGINT, &sigact, 0);
    sigaction(SIGTERM, &sigact, 0);
}

/**
 * \brief Stop signal handler method
 * \param[in] signo - signal number integer
 * \param[in] si, ptr - todo update
 */
void OruDaemonBase::stopSignalHandler(int signo, siginfo_t* si, void* ptr)
{
    _instance->_signo = signo;

    if (signo == SIGTERM || signo == SIGINT || signo == SIGQUIT)
    {
        _instance->stop();
    }
}

/**
 * \brief Clean method
 */
void OruDaemonBase::clean()
{
    if (_signo)
    {
        // got signal before
        syslog(LOG_INFO) << _moduleName.c_str() << " Signal " << strsignal(_signo);
        if (_signo == SIGTERM || _signo == SIGINT || _signo == SIGQUIT)
        {
            // got stop signal
            syslog(LOG_INFO) << _name << " Stop";
        }
    }

    // delete pid file
    unlink(_pidFileName.c_str());

    syslog(LOG_INFO) << _name << " Stopped";
}

/**
 * \brief Run daemon method
 * \param[in] argc - number of options
 * \param[in] argv - list of options
 * \retval ::ReturnCode
 */
ReturnCode OruDaemonBase::run(int argc, char** argv)
{
    if (!argv)
    {
        cout << "incorrect incoming parameters" << endl;
        return ReturnCode::BAD_ARGUMENTS;
    }
    // parse arguments
    _startOption = parseArgs(argc, argv);
    if (_startOption == DM_OPT_UNKNOWN)
    {
        usage();
        return ReturnCode::BAD_ARGUMENTS;
    }

    // check daemon was already started
    int rc = checkProcess();
    if (!rc)
    {
        if (_startOption != DM_OPT_FOREGROUND)
        {
            int pid = fork();
            // cannot fork
            if (pid == -1)
            {
                syslog(LOG_CRIT) << _moduleName << " Start daemon error";
                cout << "Start daemon error: " << errno << endl;
                return ReturnCode::FAILED;
            }
            // daemon
            else if (!pid)
            {
                umask(0);
                setsid();
                rc = chdir("/");
                try
                {
                    rc = mainLoop();
                }
                catch (...)
                {
                    syslog(LOG_ERR) << _moduleName << " Unhandled exception";
                }
            }
            // launcher
            else
            {
                return ReturnCode::OK;
            }
        }
        else
        {
            umask(0);
            setsid();
            rc = chdir("/");
            try
            {
                rc = mainLoop();
            }
            catch (...)
            {
                syslog(LOG_ERR) << _moduleName << " Unhandled exception";
            }
        }
    }

    return ReturnCode::OK;
}

/**
 * \brief Main loop of OruDaemon class
 * \retval ::ReturnCode
 */
int OruDaemon::mainLoop()
{
    ReturnCode rc = ReturnCode::OK;
    syslog(LOG_INFO) << _moduleName << " Started";

    // init logging
    initLog();

    // save version
    logVersion();

    // set signal handlers
    setStopSignalHandlers();

    // set limits
    setfileDescriptorLimit();

    // set pid to file
    setPidToFile();

    // run loop
    _running = true;

    // communications
    rc = rc ? rc : createCommunicationServices();

    // daemon core loop
    for (; !rc && _running;)
    {
        rc = core();
    }

    clean();
    destroyCommunicationServices();

    return rc;
}

void OruDaemon::logVersion(bool bCout)
{
    _version = 0;

    if (bCout)
    {
        cout << _name << " version: " << _version << endl;
    }
    else
    {
        syslog(LOG_INFO) << _name << " version: " << _version;
    }
}

} // namespace ORU
