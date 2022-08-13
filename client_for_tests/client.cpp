#include "client.h"

#include <vector>

tcp::socket ClientForTests::Connect()
{
    // I/O контекст, необходимый для всех I/O операций
    boost::asio::io_context ioc;

    // Resolver для определения endpoint'ов
    boost::asio::ip::tcp::resolver resolver(ioc);

    // Tcp сокет, использующейся для соединения
    boost::asio::ip::tcp::socket socket(ioc);

    // Резолвим адрес и устанавливаем соединение
    boost::asio::connect(socket, resolver.resolve(host_, port_));

    return socket;
}

void ClientForTests::FlipTest()
{
    std::cout << "start flip test" << std::endl;
    using namespace cv;
    const std::string test_image_path = samples::findFile("../images/test_image.jpg");
    const int version = 11;

    Mat test_image = imread(test_image_path, IMREAD_COLOR);
    Mat flip_image;
    flip(test_image, flip_image, 0);

    tcp::socket socket(Connect());

    std::vector<uchar> req_image_buffer;
    imencode(".jpg", test_image, req_image_buffer);
    // Set up an HTTP GET request message
    http::request<http::dynamic_body> req;
    req.version(version);
    req.method(http::verb::post);
    req.set(http::field::host, host_);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    req.set(http::field::content_length, req_image_buffer.size());
    for (int i=0; i< req_image_buffer.size();i ++)
    {
        beast::ostream(req.body()) << req_image_buffer[i];
    }

    // Send the HTTP request to the remote host
    http::write(socket, req);

    boost::beast::flat_buffer buffer;
    http::response<http::dynamic_body> res;
    http::read(socket, buffer, res);
    socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);

    std::string raw_data = buffers_to_string(res.body().data());
    Mat res_image(cv::imdecode(std::vector<char>(raw_data.begin(), raw_data.end()), cv::IMREAD_ANYCOLOR));  
    Mat diff;
    absdiff(flip_image, res_image, diff);
    if (countNonZero(diff) != 0)
    {
        std::cout << "flip image test bad" << std::endl;
    }
    else 
    {
        std::cout << "flip image test ok" << std::endl;
    }

}

void ClientForTests::WrongRequestMethodTest()
{
    std::cout << "start wrong request method test" << std::endl;
    const std::string target = "/echo?input=test";
    const std::string expected_response = "Invalid request-method 'GET'";
    const int version = 11;

    tcp::socket socket(Connect());

    // Дальше необходимо создать HTTP GET реквест с указанием таргета
    http::request<http::string_body> req(http::verb::get, target, version);

    // Задаём поля HTTP заголовка
    req.set(http::field::host, host_);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    
    // Отправляем реквест через приконекченный сокет
    http::write(socket, req);

    // Часть, отвечающая за чтение респонса
    boost::beast::flat_buffer buffer;
    http::response<http::dynamic_body> res;
    http::read(socket, buffer, res);

    std::cout << res << std::endl;

    // Закрываем соединение
    socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);

    if (buffers_to_string(res.body().data()) != "")
    {
        std::cout << "wrong request method test bad" << std::endl;
    }
    else 
    {
        std::cout << "wrong request method test ok" << std::endl;
    }
}

void ClientForTests::RunAllTests()
{
    FlipTest();
    WrongRequestMethodTest();
}