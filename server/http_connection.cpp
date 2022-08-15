#include "http_connection.h"

namespace ImageFlipServer
{
    void HttpConnection::start()
    {
        ReadRequest();
        CheckDeadline();
    }

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
                if (!ec)
                {
                    self->ProcessRequest();
                }
            });
    }

    void HttpConnection::ProcessRequest()
    {
        response.version(request.version());
        response.keep_alive(false);

        switch (request.method())
        {
        case http::verb::post:
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
        std::string raw_data = buffers_to_string(request.body().data());
        cv::Mat request_image(cv::imdecode(std::vector<char>(raw_data.begin(), raw_data.end()), cv::IMREAD_ANYCOLOR));
        cv::Mat image;
        cv::flip(request_image, image, 0);
        cv::imencode(".jpg", image, response_image_buffer);
    }

    void HttpConnection::CreateResponse()
    {
        FlipImage();

        response.result(http::status::ok);
        response.set(http::field::server, "ImageFlipServer");
        response.set(http::field::content_type, "image/jpeg");
        response.content_length(response_image_buffer.size());
        for (size_t i = 0; i < response_image_buffer.size(); i++)
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
            });
    }

    void HttpConnection::CheckDeadline()
    {
        auto self = shared_from_this();

        deadline.async_wait(
            [self](beast::error_code ec)
            {
                if (!ec)
                {
                    self->socket.close(ec);
                }
            });
    }
}