#pragma once

#include <string>
namespace ORU
{
    /**
     * \brief Status codes
     */
    typedef enum {
    	UNKNOWN_ERROR      = -1,                ///< Unknown error
        OK                 =  0,                ///< Okay status
        ERROR,                                  ///< Common error status
        BAD_ARGUMENTS,                          ///< Bad arguments
        FAILED,                                 ///< Failed status
        IS_INTERRUPTED,                         ///< Daemon is inerrupted
        IS_RUNNING,                             ///< Daemon is running
        
        CODE_AMOUNT
    } ReturnCode;

    std::string getReturnCodeNames(const int id);
}
