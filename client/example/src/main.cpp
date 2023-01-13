#include <iostream>
#include <boost/asio.hpp>
#include <boost/stacktrace.hpp>
#include <thread>
#include <chrono>
#include <cstdarg>
#include <vector>

#include "matrix.h"

using namespace boost::asio;
using ip::tcp;
using std::cout;
using std::endl;
using std::string;

void _Assert(bool expression, const char* file, int line, const char* format, ...)
{
  // va_list params;
  // va_start(params, format);

  string stacktrace = boost::stacktrace::to_string(boost::stacktrace::stacktrace());
  if (!expression)
  {
    printf("<----------------------------- ASSERT: ----------------------------->\n");
    printf("FILE: %s.\n", file);
    printf("LINE: %d.\n", line);
    // printf("MESSAGE: ");
    // printf(format, params);
    printf("STACKSTARACE: \n");
    printf("%s", stacktrace.c_str());
    exit(-1);
  }
}

#define Assert(expression, format, ...) _Assert(expression, __FILE__ , __LINE__, format, ##__VA_ARGS__)

static int to_int(const std::string &str)
{
  std::istringstream sin(str);
  int res;

  sin >> res;

  return res;
}

std::vector<lab2::matrix> read_matrixes(tcp::socket& socket)
{
  boost::system::error_code error;
  boost::asio::streambuf receive_buffer;

  int bytes_cnt;
  boost::asio::read(socket, boost::asio::buffer(&bytes_cnt, sizeof(int)), error);
  // Assert(!(!error), "[ERROR] -- Receive failed\n");
  
  std::vector<int> buffer(bytes_cnt);
  boost::asio::read(socket, boost::asio::buffer(buffer.data(), bytes_cnt * sizeof(int)), error);
  // Assert(!(!error), "[ERROR] -- Receive failed\n");

  auto buf_iter = buffer.begin();
  std::vector<lab2::matrix> result;

  for (int matrix_cnt = 0; matrix_cnt < 2; matrix_cnt++)
  {
    int rows = *buf_iter++;
    int cols = *buf_iter++;

    lab2::matrix matrix(rows, cols);

    for (int i = 0; i < rows; i++)
    {
      for (int j = 0; j < cols; j++)
      {
        matrix[i][j] = *buf_iter++;
      }
    }
    
    result.push_back(std::move(matrix));
  }

  return result;
}

std::vector<int> matrix_to_vector(lab2::matrix& matrix)
{
  std::vector<int> mat_v(matrix.rows() * matrix.cols() + 3);
  auto mat_v_iter = mat_v.begin();

  *mat_v_iter++ = mat_v.size() - 1;
  *mat_v_iter++ = matrix.rows();
  *mat_v_iter++ = matrix.cols();

  for (int i = 0; i < matrix.rows(); i++)
  {
    for (int j = 0; j < matrix.cols(); j++)
    {
      *mat_v_iter++ = matrix[i][j];
    }
  }

  return mat_v;
}

void write_matrix(tcp::socket &socket, lab2::matrix& matrix)
{
  auto tx_buf = matrix_to_vector(matrix);

  boost::system::error_code error;
  boost::asio::write(socket, boost::asio::buffer(tx_buf.data(), tx_buf.size() * sizeof(int)), error);
  // Assert(!(!error), "Can`t write data.\n");
}

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


  auto mat_v = read_matrixes(socket);

  auto result = mat_v[0] * mat_v[1];

  write_matrix(socket, result);

  return 0;
}