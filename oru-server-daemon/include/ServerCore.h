#pragma once

#include "DaemonLib.h"
#include <cstdlib>
#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

namespace ORU
{
/**
 * \brief Server daemon class
 */
class ServerDaemon : public OruDaemon
{

  private:
    // File descriptor for the server socket
    int _serverFd;
    // File descriptor for the client socket
    int _clientFd;
    // Structure to hold the address information for Unix domain socket communication
    struct sockaddr_un _address;
    // Length of the address structure
    int _addrlen;
    // Path for the server socket
    std::string _socketServerPath;

    void processMessage();

  public:
    ServerDaemon();
    ~ServerDaemon();

    ReturnCode createCommunicationServices(); ///< Function to create communication services
    void destroyCommunicationServices();      ///< Function to destroy communication services
    ReturnCode core();                        ///< Function for core logic

}; // class ServerDaemon

} // namespace ORU
