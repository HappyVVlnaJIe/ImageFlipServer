#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
#include <opencv2/opencv.hpp>
#include <vector>

namespace beast = boost::beast;         
namespace http = beast::http;           
namespace net = boost::asio;            
using tcp = boost::asio::ip::tcp;       

class HttpConnection : public std::enable_shared_from_this<HttpConnection>
{
public:
    HttpConnection(tcp::socket socket)
        : socket(std::move(socket))
    {
    }

    void start()
    {
        ReadRequest();
        CheckDeadline();
    }

private:
    std::vector<uchar> response_image_buffer;
    // Сокет для текущего подключенного клиента
    tcp::socket socket;

    beast::flat_buffer buffer{8192};

    http::request<http::dynamic_body> request;

    http::response<http::dynamic_body> response;

    // Таймер на обработку соединения
    net::steady_timer deadline{
        socket.get_executor(), std::chrono::seconds(60)
    };

    // Асинхронное получение запроса
    void ReadRequest();

    // Обработка запроса
    void ProcessRequest();

    // Работа с изображением изображения
    void FlipImage();

    // Формирование ответа
    void CreateResponse();

    // Асинхронная передача ответа
    void WriteResponse();

    // Проверка времени обработки соединения
    void CheckDeadline();
};
