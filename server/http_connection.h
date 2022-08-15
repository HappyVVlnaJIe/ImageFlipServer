#pragma once

#include "constants.h"

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

namespace ImageFlipServer
{
    class HttpConnection : public std::enable_shared_from_this<HttpConnection>
    {
    public:
        HttpConnection(tcp::socket socket)
            : socket(std::move(socket))
        {
        }

        void start();

    private:
        std::vector<uchar> response_image_buffer;
        // Сокет для текущего подключенного клиента
        tcp::socket socket;

        beast::flat_buffer buffer{buffer_size};

        http::request<http::dynamic_body> request;

        http::response<http::dynamic_body> response;

        // Таймер на обработку соединения
        net::steady_timer deadline{
            socket.get_executor(), deadline_time};

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
}