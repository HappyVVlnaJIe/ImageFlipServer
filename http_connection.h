#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
#include <opencv2/opencv.hpp>
#include <vector>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

class HttpConnection : public std::enable_shared_from_this<HttpConnection>
{
public:
    HttpConnection(tcp::socket socket)
        : socket(std::move(socket))
    {
    }

    // Initiate the asynchronous operations associated with the connection.
    void
    start()
    {
        ReadRequest();
        CheckDeadline();
    }

private:
    std::vector<uchar> v_char;
    // The socket for the currently connected client.
    tcp::socket socket;

    // The buffer for performing reads.
    beast::flat_buffer buffer{8192};

    // The request message.
    http::request<http::dynamic_body> request;

    // The response message.
    http::response<http::dynamic_body> response;

    // The timer for putting a deadline on connection processing.
    net::steady_timer deadline{
        socket.get_executor(), std::chrono::seconds(60)};

    // Asynchronously receive a complete request message.
    void ReadRequest();

    // Determine what needs to be done with the request message.
    void ProcessRequest();

    void FlipImage();

    // Construct a response message based on the program state.
    void CreateResponse();

    // Asynchronously transmit the response message.
    void WriteResponse();

    // Check whether we have spent enough time on this connection.
    void CheckDeadline();
};
