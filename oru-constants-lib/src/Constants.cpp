#include <string>
#include <cstring>
#include "Constants.h"

namespace ORU
{
    std::string returnCodeNames[] = {
            "UNKNOWN_ERROR",                          ///< Unknown error
            "OK",                                     ///< Okay status
            "ERROR",                                  ///< Common error status
            "BAD_ARGUMENTS",                          ///< Bad arguments
            "FAILED",                                 ///< Failed status
            "IS_INTERRUPTED",                         ///< Daemon is inerrupted
            "IS_RUNNING",                             ///< Daemon is running
    };

    std::string getReturnCodeNames(const int id)
    {
        if (id >= -1 && id < ReturnCode::CODE_AMOUNT) {
            return returnCodeNames[id + 1];
        }

        return std::string("UNKNOWN_ERROR");
    }
}
