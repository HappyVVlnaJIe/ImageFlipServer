#pragma once

#include "http_connection.h"
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <memory>
#include <string>

namespace ImageFlipServer
{
    class BaseServer
    {
    public:
        void Start(tcp::acceptor &acceptor, tcp::socket &socket);
    };
}