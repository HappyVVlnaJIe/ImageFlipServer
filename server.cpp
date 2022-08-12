#include "server.h"



namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

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
