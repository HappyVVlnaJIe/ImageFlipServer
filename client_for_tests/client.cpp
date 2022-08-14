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
    char const* target = "/";

    Mat test_image = imread(test_image_path, IMREAD_COLOR);
    Mat flip_image;
    flip(test_image, flip_image, 0);

    imwrite("../images/flip_image.jpg", flip_image); // TODO: убрать

    tcp::socket socket(Connect());

    std::vector<uchar> req_image_buffer;
    imencode(".jpg", test_image, req_image_buffer);

    http::request<http::dynamic_body> req;
    req.version(version);
    req.method(http::verb::post);
    req.target(target);
    req.set(http::field::host, host_);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    req.set(http::field::content_type, "image/jpeg");
    req.content_length(req_image_buffer.size());
    for (int i=0; i< req_image_buffer.size();i ++)
    {
        beast::ostream(req.body()) << req_image_buffer[i];
    }

    // Send the HTTP request to the remote host
    http::write(socket, req);
    std::cout << "send image in flip test" << std::endl; // TODO: убрать

    boost::beast::flat_buffer buffer;
    http::response<http::dynamic_body> res;
    http::read(socket, buffer, res);

    std::cout << "read image in test" << std::endl; // TODO: убрать
    std::string raw_data = buffers_to_string(res.body().data());
    std::cout << "try create res_image" << std::endl; // TODO: убрать
    Mat res_image(cv::imdecode(std::vector<char>(raw_data.begin(), raw_data.end()), cv::IMREAD_ANYCOLOR));  
    
    Mat diff;
    std::cout << "try DIFF" << std::endl; // TODO: убрать
    absdiff(flip_image, res_image, diff);
    std::cout << "res_image is empty " << res_image.empty() << std::endl; // TODO: убрать
    std::cout << "diff is empty " << diff.empty() << std::endl; // TODO: убрать

    imwrite("../images/res_image.jpg", res_image); // TODO: убрать
    imwrite("../images/diff_image.jpg", diff); // TODO: убрать
    
    if (countNonZero(diff) == 0)
    {
        std::cout << "flip image test ok" << std::endl;
    }
    else 
    {
        std::cout << "flip image test fall" << std::endl;
    }
    socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
}

void ClientForTests::WrongRequestMethodTest()
{
    std::cout << "wrong request method test start" << std::endl;
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

    if (buffers_to_string(res.body().data()) != "Invalid request-method 'GET'")
    {
        std::cout << "wrong request method test fall" << std::endl;
    }
    else 
    {
        std::cout << "wrong request method test ok" << std::endl;
    }

    // Закрываем соединение
    socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);

}

void ClientForTests::RunAllTests()
{
    WrongRequestMethodTest();
    FlipTest();
}