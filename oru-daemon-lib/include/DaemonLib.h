#pragma once

#include <atomic>
#include <csignal>
#include <future>
#include <vector>

#include "Constants.h"

namespace ORU
{
/**
 * \brief Enumeration values for daemon input options
 */
typedef enum
{
    DM_OPT_UNKNOWN = 0,    ///< Help or unknown
    DM_OPT_NORMAL = 1,     ///< Normal
    DM_OPT_FOREGROUND = 2, ///< Foreground
    DM_OPT_QUIT = 3,       ///< Quit
} DaemonInputOption;

/**
 * \brief   Base class for ORU daemons
 * \details Parent class for Supervisor.
 */
class OruDaemonBase
{
  protected:
    static OruDaemonBase* _instance;

    void usage();
    DaemonInputOption parseArgs(int argc, char** argv);
    void initLog();
    ReturnCode checkProcess();
    void setStopSignalHandlers();
    void clean();

    void stop()
    {
        _running = false;
    }

    int setfileDescriptorLimit();

    static void stopSignalHandler(int signo, siginfo_t* si, void* ptr);

    static const size_t _fdLimit{10240};            ///< Limit of file descriptors
    std::atomic<bool> _running{false};              ///< Alive attribute
    int _signo{0};                                  ///< Last catched signal
    std::string _name{""};                          ///< Daemon name
    std::string _moduleName{""};                    ///< Daemon name
    std::string _pidFileName{""};                   ///< PID file name
    DaemonInputOption _startOption{DM_OPT_UNKNOWN}; ///< Daemon start option
    uint32_t _version;                              ///< Daemon version

    virtual int mainLoop()
    {
        return ReturnCode::OK;
    }

    virtual void logVersion(bool bCout = false); ///< Logging daemon (and library) version

    virtual ReturnCode createCommunicationServices() = 0; ///< Function to create communication services
    virtual void destroyCommunicationServices() = 0;      ///< Function to destroy communication services
    virtual ReturnCode core() = 0;                        ///< Function for core logic

  public:
    explicit OruDaemonBase(const std::string& daemonName);

    virtual ~OruDaemonBase();

    // int     statusReplyHandler(socket_message_t *msg);
    ReturnCode run(int argc, char** argv);

    uint32_t getVersion() const
    {
        return _version;
    }

    // utilities
    static int getPidFromFile(const std::string& fileName);
    ReturnCode setPidToFile();

}; // OruDaemon

/**
 * \brief   Base class for ORU daemons
 * \details Parent class for all ORU daemons except Supervisor.
 *          It has Synchro Client to send daemon status to Supervisor.
 */
class OruDaemon : public OruDaemonBase
{
    virtual int mainLoop();

  public:
    explicit OruDaemon(const std::string& daemonName) : OruDaemonBase(daemonName)
    {
    }

    ~OruDaemon()
    {
    }

    void logVersion(bool bCout = false); ///< Logging daemon version

}; // OruDaemon

} // namespace ORU
