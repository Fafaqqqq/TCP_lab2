#include <iostream>
#include <boost/asio.hpp>
#include <vector>
#include <sstream>
#include <fstream>

#include "matrix.h"

using namespace boost::asio;
using ip::tcp;
using std::cout;
using std::endl;
using std::string;

static int to_int(const std::string& str)
{
  std::istringstream sin(str);
  int res;

  sin >> res;

  return res;
}

static std::vector<lab2::matrix> matrix_generator(int argc, char** argv)
{
  std::vector<lab2::matrix> result;

  if (1 == argc)
  {
    return result;
  }

  for (int i = 1; i < argc; ++i)
  {
    if (std::string(argv[i]) == "--matrix")
    {
      int rows = to_int(argv[++i]);
      int cols = to_int(argv[++i]);
      
      lab2::matrix mat(rows, cols);
      
      for(int i = 0; i < rows; i++)
      {
        for(int j = 0; j < cols; j++)
        {
            mat[i][j] = rand() % 11;
        }
      }

      result.push_back(std::move(mat));
    }
  }

  return result;
}

string read_(tcp::socket &socket)
{
  boost::asio::streambuf buf;
  boost::asio::read_until(socket, buf, "\n");
  string data = boost::asio::buffer_cast<const char *>(buf.data());
  return data;
}

void send_(tcp::socket &socket, const string &message)
{
  const string msg = message + "\n";
  boost::asio::write(socket, boost::asio::buffer(message));
}

int main(int argc, char** argv)
{
  srand(time(NULL));

  std::ofstream fout;
  int port;
  std::string filename;

  if (1 == argc)
  {
    std::cerr << "Input params was`n specified!" << std::endl;
    exit(1);
  }

  for (int i = 0; i < argc; i++)
  {
    if (std::string(argv[i]) == "--port")
    {
      port = to_int(argv[++i]);
    }
    if (std::string(argv[i]) == "--file")
    {
      fout.open(argv[++i], std::ios::app);
      filename = argv[i];
    }
  }


  if (!fout.is_open())
  {
    std::cerr << "File \"" << filename << "\" doesn`t exit!" << std::endl;
    exit(1);
  }

  auto mat_v = matrix_generator(argc, argv);

  if (mat_v.size() < 2)
  {
    std::cerr << "The required number of matrices wasn`t specified!" << std::endl;
    exit(1);
  }

  // auto result = mat_v[0] * mat_v[1];
  
  boost::asio::io_service io_service;
  // listen for new connection
  tcp::acceptor acceptor_(io_service, tcp::endpoint(tcp::v4(), 1234));
  // socket creation
  tcp::socket socket_(io_service);
  // waiting for connection
  std::cout << "Wainting a connection...\n" << std::endl;


  acceptor_.accept(socket_);
  // read operation
  string message = read_(socket_);
  cout << message << endl;
  // write operation
  send_(socket_, "Hello From Server!");
  cout << "Servent sent Hello message to Client!" << endl;
  return 0;
}