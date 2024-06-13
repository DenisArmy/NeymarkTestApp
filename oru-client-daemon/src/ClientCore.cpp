#include <cstdlib>
#include <cstring>
#include <string>

#include "ClientCore.h"
#include "Logger.h"
#include "Transport.h"

using namespace std;

namespace ORU
{

/**
 * \brief Client daemon class
 */

/**
 * @brief Client daemon constructor
 */
ClientDaemon::ClientDaemon()
    : OruDaemon("Client"),
      _socketServerPath("/tmp/unix.socket")
{
}

/**
 * @brief Client daemon destructor
 */
ClientDaemon::~ClientDaemon()
{
}

/**
 * @brief Method for create communication services
 * @return return code(ReturnCode::OK if success)
 */
ReturnCode ClientDaemon::createCommunicationServices()
{
    return ReturnCode::OK;
}

/**
 * @brief Method for destroy communication services
 */
void ClientDaemon::destroyCommunicationServices()
{
}

/**
 * @brief Core function of the ClientDaemon.
 * @return ReturnCode::OK if the operation is successful, ReturnCode::FAILED otherwise.
 */

ReturnCode ClientDaemon::core()
{
    _address.sun_family = AF_UNIX;
    strncpy(_address.sun_path, _socketServerPath.c_str(), sizeof(_address.sun_path) - 1);
    if ((_clientFd = socket(AF_UNIX, SOCK_STREAM, 0/*protocol*/)) == -1)
    {
        syslog(LOG_INFO) << "Failed to create socket" << endl;
        return ReturnCode::FAILED;
    }
    if (connect(_clientFd, (struct sockaddr*)&_address, sizeof(_address)) == -1)
    {
        syslog(LOG_INFO) << "Failed to create connection to Server" << endl;
        return ReturnCode::FAILED;
    }

    Ping reqP = Transport::createPingMsg();
    if (send(_clientFd, &reqP/*message*/, sizeof(reqP)/*length*/, 0/*flags*/) == -1)
    {
        syslog(LOG_INFO) << "Failed to send message to Server" << endl;
        return ReturnCode::FAILED;
    }
    processMessage();
    close(_clientFd);

    return ReturnCode::OK;
}

/**
 * @brief Processing incomming message.
 * This function parse incomming message and process it
 */
void ClientDaemon::processMessage()
{
    char buffer[128];
    int numBytes = read(_clientFd, buffer, sizeof(buffer));
    syslog(LOG_INFO) << "Received " << numBytes << " bytes" << endl;

    if (numBytes > 0)
    {
        Header* header = reinterpret_cast<Header*>(buffer);
        Status* status = reinterpret_cast<Status*>(buffer);
        syslog(LOG_INFO) << "Receive message:" << endl;
        syslog(LOG_INFO) << "Version: " << static_cast<int>(header->version) << endl;
        syslog(LOG_INFO) << "Seq number: " << static_cast<int>(header->seqNum) << endl;
        syslog(LOG_INFO) << "Message type: " << static_cast<int>(header->type) << endl;
        syslog(LOG_INFO) << "Message size: " << static_cast<int>(header->size) << endl;
        syslog(LOG_INFO) << "Message status: " << static_cast<int>(header->status) << endl;

        switch (header->type)
        {
        case MSG_PONG:
            syslog(LOG_INFO) << "Receive PONG message" << endl;
            break;
        case MSG_RESP_STATUS:
            syslog(LOG_INFO) << "Receive RESP_STATUS message: " << static_cast<int>(status->status) << endl;
            break;
        case MSG_ERROR:
            syslog(LOG_INFO) << "Receive ERROR message" << endl;
            break;
        default:
            syslog(LOG_INFO) << "Receive unsupported message" << endl;
            break;
        }
    }
    sleep(5);
}

} // namespace ORU
