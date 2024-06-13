#pragma once

#include <cstdlib>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define PROTOCOL_VERSION 0

namespace ORU
{

/**
 * \brief Message type
 */
typedef enum
{
    MSG_PING,            ///< Ping message
    MSG_PONG,            ///< Pong message (ping answer)
    MSG_REQ_STATUS,      ///< Request service status
    MSG_RESP_STATUS,     ///< Responce service status
    MSG_ERROR            ///< Error message
}
MessageType;

/**
 * \brief Message statuses
 */
typedef enum
{
    STATUS_OK,           ///< Request was successfully proceed
    STATUS_ERROR,        ///< Error is happen
    STATUS_STARTING,     ///< Service is starting
    STATUS_PROCESSING,   ///< Service is processing
    STATUS_STOPPING      ///< Service is stopping
}
MessageStatus;

/**
 * \brief Message header
 */
typedef struct __attribute__((__packed__))
{
    uint8_t version;     ///< Protocol version
    uint32_t seqNum;     ///< Sequence number
    uint8_t type;        ///< Message type
    uint8_t size;        ///< Message payload size
    uint8_t status;      ///< Message status
}
Header;

/**
 * \brief Error message
 */
typedef struct
{
    Header header;        ///< Message header
}
Error;

/**
 * \brief Message for ping
 */
typedef struct
{
    Header header;        ///< Message header
}
Ping;

/**
 * \brief Message for checking status
 */
typedef struct
{
    Header header;        ///< Message header
    MessageStatus status; ///< Service status
}
Status;

/**
 * \brief Transport class
 */
class Transport
{

  private:
    // Sequence number
    static uint32_t _seqNum;

  public:
    Transport()  { _seqNum = 0; }  ///< Constructor
    ~Transport() {};               ///< Destructor

    static Ping createPingMsg()
    {
        Ping req;
        req.header.version = PROTOCOL_VERSION;
        req.header.seqNum = _seqNum++;
        req.header.type = MSG_PING;
        req.header.size = 0;
        req.header.status = STATUS_OK;
        return req;
    }
    static Ping createPongMsg(Header* request)
    {
        Ping resp;
        resp.header.version = PROTOCOL_VERSION;
        resp.header.seqNum = request->seqNum;
        resp.header.type = MSG_PONG;
        resp.header.size = 0;
        resp.header.status = STATUS_OK;
        return resp;
    }

    static Status createReqStatusMsg()
    {
        Status req;
        req.header.version = PROTOCOL_VERSION;
        req.header.seqNum = _seqNum++;
        req.header.type = MSG_REQ_STATUS;
        req.header.size = 0;
        req.header.status = STATUS_OK;
        return req;
    }
    static Status createRespStatusMsg(Header* request, MessageStatus status)
    {
        Status resp;
        resp.header.version = PROTOCOL_VERSION;
        resp.header.seqNum = request->seqNum;
        resp.header.type = MSG_RESP_STATUS;
        resp.header.size = 4;
        resp.header.status = STATUS_OK;
        resp.status = status;
        return resp;
    }

    static Error createErrorMsg(Header* request)
    {
        Error resp;
        resp.header.version = PROTOCOL_VERSION;
        resp.header.seqNum = request->seqNum;
        resp.header.type = MSG_ERROR;
        resp.header.size = 0;
        resp.header.status = STATUS_ERROR;
        return resp;
    }

}; // class Transport

} // namespace ORU

