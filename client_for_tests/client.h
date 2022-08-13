#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
#include <opencv2/opencv.hpp>
#include <string>

namespace beast = boost::beast;         
namespace http = beast::http;           
namespace net = boost::asio;            
using tcp = boost::asio::ip::tcp;     

class ClientForTests
{
public:
    ClientForTests(std::string host, const char* port) : host_(host), port_(port) {}
    void RunAllTests();
    void FlipTest();
    void WrongRequestMethodTest();
private:
    tcp::socket Connect();
    std::string host_;
    std::string port_;
};