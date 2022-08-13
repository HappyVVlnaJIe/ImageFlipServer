#include "server.h"



namespace beast = boost::beast;                             
using tcp = boost::asio::ip::tcp;       

void BaseServer::Start(tcp::acceptor& acceptor, tcp::socket& socket) 
{
    acceptor.async_accept(socket,
        [&](beast::error_code ec)
        {
            if(!ec)
                std::make_shared<HttpConnection>(std::move(socket))->start();
            Start(acceptor, socket);
        }
    );
}
