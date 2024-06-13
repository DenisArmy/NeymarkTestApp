#pragma once

#include "DaemonLib.h"
#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

namespace ORU
{

/**
 * \brief Client daemon class
 */
class ClientDaemon : public OruDaemon
{

  private:
    // File descriptor for the client socket
    int _clientFd;
    // Structure to hold the address information for Unix domain socket communication
    struct sockaddr_un _address;
    // Path for the server socket
    std::string _socketServerPath;

    void processMessage();

  public:
    ClientDaemon();
    ~ClientDaemon();

    ReturnCode createCommunicationServices(); ///< Function to create communication services
    void destroyCommunicationServices();      ///< Function to destroy communication services
    ReturnCode core();                        ///< Function for core logic

}; // class ClientDaemon

} // namespace ORU
