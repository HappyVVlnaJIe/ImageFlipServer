#include "http_connection.h"

/*
template<class ConstBufferSequence>
std::vector<char> toVector(ConstBufferSequence const& buffers)
{
    std::vector<char> data;
    data.reserve(boost::asio::buffer_size(buffers));
    for(boost::asio::const_buffer b : buffers)
        data.push_back(boost::asio::buffer_cast<char const*>(b),
            boost::asio::buffer_size(b));
    return s;
}
*/
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
                    self->ProcessRequest();
            }
    );
}

void HttpConnection::ProcessRequest()
{
    response.version(request.version());
    response.keep_alive(false);

    switch(request.method())
    {
        case http::verb::post:
            CreateResponse();
            break;

        default:
            // We return responses indicating an error if
            // we do not recognize the request method.
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
    std::cout << "request image is empty " << request_image.empty() << std::endl;   //TODO: убрать
    cv::Mat image;               // dst must be a different Mat
    cv::flip(request_image, image, 0);
    cv::imwrite("test.jpg", image);                   //TODO: убрать                                                                              // TODO: убрать
    cv::imencode(".jpg", image, v_char);
    std::cout<< "image is empty " << image.empty() <<std::endl; //TODO: убрать
}

void HttpConnection::CreateResponse()
{
    FlipImage();
    response.result(http::status::ok);
    response.set(http::field::server, "ImageFlipServer");
    response.set(http::field::content_type, "image/jpeg");
    response.content_length(v_char.size());
    //beast::ostream(response.body());
    for (int i=0; i< v_char.size();i ++)
    {
        beast::ostream(response.body()) << v_char[i];
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

    // Check whether we have spent enough time on this connection.
    void HttpConnection::CheckDeadline()
    {
        auto self = shared_from_this();

        deadline.async_wait(
            [self](beast::error_code ec)
            {
                if(!ec)
                {
                    // Close socket to cancel any outstanding operation.
                    self->socket.close(ec);
                }
            });
    }