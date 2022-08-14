#include "client.h"

#include <iostream>

int main(int argc, char* argv[])
{
    // Check command line arguments.
    if(argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <address> <port>\n";
        std::cerr << "  For IPv4, try:\n";
        std::cerr << "    receiver 0.0.0.0 80\n";
        std::cerr << "  For IPv6, try:\n";
        std::cerr << "    receiver 0::0 80\n";
        return EXIT_FAILURE;
    }
    ClientForTests client(argv[1], argv[2]);
    std::cout << "start test" << std::endl;
    client.RunAllTests();

    return EXIT_SUCCESS;
}