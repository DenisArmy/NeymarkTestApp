#include <cstdlib>
#include <cstring>
#include <string>

#include "Logger.h"
#include "ServerCore.h"
#include "Transport.h"

using namespace std;

namespace ORU
{
/**
 * \brief Server daemon class
 */

/**
 * @brief Server daemon constructor
 */
ServerDaemon::ServerDaemon()
    : OruDaemon("Server"),
      _addrlen(sizeof(_address)),
      _socketServerPath("/tmp/unix.socket")
{
}

/**
 * @brief Server daemon destructor
 */
ServerDaemon::~ServerDaemon()
{
}

/**
 * @brief Creates communication services for the ServerDaemon.
 * @return ReturnCode::OK if the communication services are successfully created,
 *         ReturnCode::FAILED otherwise.
 */
ReturnCode ServerDaemon::createCommunicationServices()
{
    if ((_serverFd = socket(AF_UNIX, SOCK_STREAM, 0)) == 0)
    {
        return ReturnCode::FAILED;
    }
    unlink(_socketServerPath.c_str());
    _address.sun_family = AF_UNIX;
    strncpy(_address.sun_path, _socketServerPath.c_str(), sizeof(_address.sun_path) - 1);
    if (bind(_serverFd, (struct sockaddr*)&_address, sizeof(_address)) == -1)
    {
        return ReturnCode::FAILED;
    }
    if (listen(_serverFd, 5) == -1)
    {
        return ReturnCode::FAILED;
    }
    return ReturnCode::OK;
}

/**
 * @brief Destroys communication services for the ServerDaemon.
 */
void ServerDaemon::destroyCommunicationServices()
{
    close(_serverFd);
    unlink(_socketServerPath.c_str());
}

/**
 * @brief Core function of the ServerDaemon.
 * @return ReturnCode::OK if the operation is successful, ReturnCode::FAILED otherwise.
 */
ReturnCode ServerDaemon::core()
{
    _clientFd = accept(_serverFd, (struct sockaddr*)&_address, (socklen_t*)&_addrlen);
    if (_clientFd == -1)
    {
        syslog(LOG_INFO) << "Failed to accept connection" << endl;
        return ReturnCode::FAILED;
    }
    processMessage();
    close(_clientFd);

    return ReturnCode::OK;
}

/**
 * @brief Processing incomming message.
 *
 * This function parse incomming message, process it and send responce
 */
void ServerDaemon::processMessage()
{
    char buffer[128];
    int numBytes = read(_clientFd, buffer, sizeof(buffer));
    syslog(LOG_INFO) << "Received " << numBytes << " bytes" << endl;

    if (numBytes > 0)
    {
        Header* header = reinterpret_cast<Header*>(buffer);
        syslog(LOG_INFO) << "Receive message:" << endl;
        syslog(LOG_INFO) << "Version: " << static_cast<int>(header->version) << endl;
        syslog(LOG_INFO) << "Seq number: " << static_cast<int>(header->seqNum) << endl;
        syslog(LOG_INFO) << "Message type: " << static_cast<int>(header->type) << endl;
        syslog(LOG_INFO) << "Message size: " << static_cast<int>(header->size) << endl;
        syslog(LOG_INFO) << "Message status: " << static_cast<int>(header->status) << endl;

        switch (header->type)
        {
        case MSG_PING:
        {
            syslog(LOG_INFO) << "Receive PING message" << endl;
            Ping resp = Transport::createPongMsg(header);
            send(_clientFd, &resp, sizeof(resp), 0);
        }
            break;
        case MSG_REQ_STATUS:
        {
            syslog(LOG_INFO) << "Receive REQ_STATUS message" << endl;
            Status resp = Transport::createRespStatusMsg(header, STATUS_PROCESSING);
            send(_clientFd, &resp, sizeof(resp), 0);
        }
            break;
        default:
        {
            syslog(LOG_INFO) << "Receive unsupported message" << endl;
            Error resp = Transport::createErrorMsg(header);
            send(_clientFd, &resp, sizeof(resp), 0);
        }
            break;
        }
    }
}

} // namespace ORU
