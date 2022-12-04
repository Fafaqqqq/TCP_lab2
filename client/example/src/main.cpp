#include <iostream>
#include <boost/asio.hpp>
#include <thread>
#include <chrono>

using namespace boost::asio;
using ip::tcp;
using std::cout;
using std::endl;
using std::string;

int main()
{
  boost::asio::io_service io_service;
  // socket creation
  tcp::socket socket(io_service);
  // connection

  bool connected = false;
  int  connecte_time = 0;
  int  time_to_connect = 10;

  for (uint32_t i = 0; i < time_to_connect && !connected; ++i)
  {
    try
    {
      socket.connect(tcp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 1234));
      connected = true;
    }
    catch (const std::exception &e)
    {
      std::cout << "Trying to connect... Attempt: {" << i << "}" << std::endl;
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }

  if (!connected)
  {
    std::cout << "\nClien couldn`t connect to server!" << std::endl;
    exit(1);
  }

  // request/message from client
  string msg = "ready\n";
  boost::system::error_code error;
  boost::asio::write(socket, boost::asio::buffer(msg), error);
  if (!error)
  {
    cout << "Ready!" << endl;
  }
  else
  {
    cout << "send failed: " << error.message() << endl;
  }

  // getting response from server
  boost::asio::streambuf receive_buffer;
  boost::asio::read_until(socket, receive_buffer, "\n", error);

  if (error && error != boost::asio::error::eof)
  {
    cout << "receive failed: " << error.message() << endl;
  }
  else
  {
    const char *data = boost::asio::buffer_cast<const char *>(receive_buffer.data());
    cout << data << endl;
  }

  msg = "done\n";
  boost::asio::write(socket, boost::asio::buffer(msg), error);
  if (error)
  {
    cout << "send failed: " << error.message() << endl;
  }

  // boost::asio::streambuf receive_buffer1;
  boost::asio::read_until(socket, receive_buffer, "\n", error);

  if (error && error != boost::asio::error::eof)
  {
    cout << "receive failed: " << error.message() << endl;
  }
  else
  {
    const char *data = boost::asio::buffer_cast<const char *>(receive_buffer.data());
    cout << data << endl;
  }
  
  return 0;
}