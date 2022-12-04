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

static int to_int(const std::string &str)
{
  std::istringstream sin(str);
  int res;

  sin >> res;

  return res;
}

static std::vector<lab2::matrix> matrix_generator(int argc, char **argv)
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

      for (int i = 0; i < rows; i++)
      {
        for (int j = 0; j < cols; j++)
        {
          mat[i][j] = rand() % 11;
        }
      }

      std::cout << mat << std::endl
                << std::endl;

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

void send_buffer(tcp::socket &socket, const string &buffer)
{
  boost::asio::write(socket, boost::asio::buffer(buffer));
}

int main(int argc, char **argv)
{
  srand(time(NULL));

  std::ofstream fout;
  int port;
  std::string filename;

  if (1 == argc)
  {
    std::cerr << "Input params wasn`t specified!" << std::endl;
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
    std::cerr << "File \"" << filename << "\" doesn`t exist!" << std::endl;
    exit(1);
  }

  auto mat_v = matrix_generator(argc, argv);

  if (mat_v.size() != 2)
  {
    std::cerr << "The required number of matrices wasn`t specified!" << std::endl;
    exit(1);
  }

  auto mat_a = mat_v[0].data();
  auto mat_b = mat_v[1].data();

  int tx_buf_a_size = mat_v[0].cols() * mat_v[0].rows() / 2;
  int tx_buf_b_size = mat_v[1].cols() * mat_v[1].rows() / 2;

  // std::cout << mat_v[0] << std::endl
  //           << std::endl;
  // std::cout << mat_v[1] << std::endl
  //           << std::endl;

  std::vector<char> tx_buf(tx_buf_a_size * sizeof(int) + 2 * sizeof(int) + 1 + 
                           tx_buf_b_size * sizeof(int) + 2 * sizeof(int) + 1);

  auto tx_buf_iter = tx_buf.begin();
  {
    (int &)*tx_buf_iter = mat_v[0].rows();
    tx_buf_iter += sizeof(int);
    (int &)*tx_buf_iter = mat_v[0].cols();
    tx_buf_iter += sizeof(int);
  }

  for (int i = 0; i < mat_v[0].rows(); i++)
  {
    for (int j = 0; j < mat_v[0].cols() / 2; j++)
    {
      (int &)*tx_buf_iter = mat_a[i][j];
      //std::cout << (int &)*tx_buf_iter << ' ';
      tx_buf_iter += sizeof(int);
    }
    //std::cout << std::endl;
  }
  //std::cout << std::endl;

  {
    (int &)*tx_buf_iter = mat_v[1].rows();
    tx_buf_iter += sizeof(int);
    (int &)*tx_buf_iter = mat_v[1].cols();
    tx_buf_iter += sizeof(int);
  }

  for (int i = 0; i < mat_v[1].rows() / 2; i++)
  {
    for (int j = 0; j < mat_v[1].cols(); j++)
    {
      (int &)*tx_buf_iter = mat_b[i][j];
      tx_buf_iter += sizeof(int);
      // std::cout << mat_b[i][j] << ' ';
    }
    // std::cout << std::endl;
  }

  tx_buf.back() = '\n';

  // std::cout << std::endl;

  auto tx_a_ptr = tx_buf.data();

  string data(tx_buf.begin(), tx_buf.end());
  // auto tx_b_ptr = mat_tx_buf_a.data();
  // auto result = mat_v[0] * mat_v[1];

  boost::asio::io_service io_service;
  // listen for new connection
  tcp::acceptor acceptor_(io_service, tcp::endpoint(tcp::v4(), 1234));
  // socket creation
  tcp::socket socket_(io_service);
  // waiting for connection
  std::cout << "Wainting for connection...\n"
            << std::endl;

  acceptor_.accept(socket_);
  // read operation
  string status = read_(socket_);

  //string data(tx_buf.begin(), tx_buf.end());
  send_buffer(socket_, data);

  cout << status << endl;

  string response = read_(socket_);

  std::cout << response << std::endl;

  send_(socket_, "123\n");
  
  return 0;
}