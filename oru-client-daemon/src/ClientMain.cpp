#include "ClientCore.h"
#include <iostream>

using namespace std;

/**
 * \brief Main.
 *
 * \details Main.
 *
 * \param[in] argc - number of options
 * \param[in] argv - list of options
 *
 * \retval Exit status
 */
int main(int argc, char* argv[])
{
    ORU::ClientDaemon daemon;

    return daemon.run(argc, argv);
}
