#include "http_connection.h"

void HttpConnection::ReadRequest()
{
    auto self = shared_from_this();

    http::async_read(
        socket,
        buffer,
        request,
        [self](beast::error_code ec,
                std::size_t bytes_transferred)
        {
            boost::ignore_unused(bytes_transferred);
            if(!ec)
            {
                self->ProcessRequest();
            }
            std::cout << ec << std::endl; //TODO: убрать
        }
    );
}

void HttpConnection::ProcessRequest()
{
    std::cout << "start ProcessRequest" << std::endl; //TODO: убрать
    response.version(request.version());
    response.keep_alive(false);

    switch(request.method())
    {
        case http::verb::post:
             std::cout << "start CreateResponse" << std::endl; //TODO: убрать
            CreateResponse();
            break;

        default:
            response.result(http::status::bad_request);
            response.set(http::field::content_type, "text/plain");
            beast::ostream(response.body())
                << "Invalid request-method '"
                << std::string(request.method_string())
                << "'";
            break;
    }

    WriteResponse();
}

void HttpConnection::FlipImage()
{
    std::cout << "start flip image" << std::endl; //TODO: убрать
    std::string raw_data = buffers_to_string(request.body().data());
    cv::Mat request_image(cv::imdecode(std::vector<char>(raw_data.begin(), raw_data.end()), cv::IMREAD_ANYCOLOR));              
    cv::Mat image;               //TODO: добавить проверку на содержимое запроса
    cv::flip(request_image, image, 0);
    cv::imencode(".jpg", image, response_image_buffer);
}

void HttpConnection::CreateResponse()
{
    FlipImage();

    std::cout << "start create response" << std::endl; //TODO: убрать
    response.result(http::status::ok);
    response.set(http::field::server, "ImageFlipServer");
    response.set(http::field::content_type, "image/jpeg");
    response.content_length(response_image_buffer.size());
    for (int i=0; i< response_image_buffer.size();i ++)
    {
        beast::ostream(response.body()) << response_image_buffer[i];
    }
    response.keep_alive(request.keep_alive());
}

void HttpConnection::WriteResponse()
{
    auto self = shared_from_this();

    http::async_write(
        socket,
        response,
        [self](beast::error_code ec, std::size_t)
        {
            self->socket.shutdown(tcp::socket::shutdown_send, ec);
            self->deadline.cancel();
        }
    );
    std::cout << "send image" << std::endl; //TODO: убрать
}


void HttpConnection::CheckDeadline()
{
    auto self = shared_from_this();

    deadline.async_wait(
        [self](beast::error_code ec)
        {
            if(!ec)
            {
                self->socket.close(ec);
            }
        }
    );
}