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

    // Set up an HTTP GET request message
    http::file_body::value_type body;
    beast::error_code ec;
    body.open(test_image_path.c_str(), beast::file_mode::read, ec);
    auto const size = body.size();

    http::request<http::file_body> req;
    req.method(http::verb::post);
    req.version(version);
    req.set(http::field::host, host_);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    req.body() = std::move(body);

    // Send the HTTP request to the remote host
    http::request_serializer<http::file_body,  http::fields> sr{req};
    http::write(socket, sr);
    std::cout << "send image in flip test" << std::endl; // TODO: убрать

    boost::beast::flat_buffer buffer;
    http::response<http::dynamic_body> res;
    http::read(socket, buffer, res);
    socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);

    std::cout << "read image in test" << std::endl; // TODO: убрать
    std::string raw_data = buffers_to_string(res.body().data());
    Mat res_image(cv::imdecode(std::vector<char>(raw_data.begin(), raw_data.end()), cv::IMREAD_ANYCOLOR));  
    Mat diff;
    absdiff(flip_image, res_image, diff);
    if (countNonZero(diff) != 0)
    {
        std::cout << "flip image test fall" << std::endl;
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

    // Закрываем соединение
    socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);

    if (buffers_to_string(res.body().data()) != "Invalid request-method 'GET'")
    {
        std::cout << "wrong request method test fall" << std::endl;
    }
    else 
    {
        std::cout << "wrong request method test ok" << std::endl;
    }
}

void ClientForTests::RunAllTests()
{
    WrongRequestMethodTest();
    FlipTest();
}