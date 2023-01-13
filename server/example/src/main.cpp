#include <iostream>
#include <boost/asio.hpp>
#include <boost/stacktrace.hpp>
#include <vector>
#include <sstream>
#include <fstream>
#include <cstdarg>

#include "matrix.h"

using namespace boost::asio;
using ip::tcp;
using std::cout;
using std::endl;
using std::string;

void _Assert(bool expression, const char* file, int line, const char* format, ...)
{
  va_list params;
  va_start(params, format);

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

      // std::cout << mat << std::endl
      //           << std::endl;

      result.push_back(std::move(mat));
    }
  }

  return result;
}

void write_matrixes(tcp::socket &socket, std::vector<lab2::matrix>& mat_v)
{
  int tx_buf_size = mat_v[0].cols() * mat_v[0].rows() / 2 + 
                    mat_v[1].cols() * mat_v[1].rows() / 2 + 5;

  std::vector<int> tx_buf(tx_buf_size);

  auto tx_buf_iter = tx_buf.begin();
  *tx_buf_iter++ = tx_buf.size() - 1;

  for (int mat_cnt = 0; mat_cnt < mat_v.size(); mat_cnt++)
  {
    int rows;
    int cols;

    if (!mat_cnt)
    {
      *tx_buf_iter++ = rows = mat_v[mat_cnt].rows();
      *tx_buf_iter++ = cols = mat_v[mat_cnt].cols() / 2;
    }
    else
    {
      *tx_buf_iter++ = rows = mat_v[mat_cnt].rows() / 2;
      *tx_buf_iter++ = cols = mat_v[mat_cnt].cols();
    }

    for (int i = 0; i < rows; i++)
    {
      for (int j = 0; j < cols; j++)
      {
        *tx_buf_iter++ = mat_v[mat_cnt][i][j];
      }
    }
  }

  boost::system::error_code error;
  boost::asio::write(socket, boost::asio::buffer(tx_buf.data(), tx_buf.size() * sizeof(int)), error);
  // Assert(!(!error), "Can`t write data.\n");
}

lab2::matrix read_matrix(tcp::socket& socket)
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

  return matrix;
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
  
  // auto result = mat_v[0] * mat_v[1];

  boost::asio::io_service io_service;
  // listen for new connection
  tcp::acceptor acceptor_(io_service, tcp::endpoint(tcp::v4(), 1234));
  // socket creation
  tcp::socket socket_(io_service);
  // waiting for connection
  std::cout << "Wainting for connection..."
            << std::endl;

  acceptor_.accept(socket_);

  auto begin = std::chrono::steady_clock::now();

  write_matrixes(socket_, mat_v);
  auto matrix = read_matrix(socket_);

  lab2::matrix mat1(mat_v[0].rows(), mat_v[0].cols() / 2 ), mat2(mat_v[1].rows() / 2 , mat_v[1].cols());

  for (int i = 0 ; i < mat_v[0].rows(); i++)
  {
    for (int j = mat_v[0].cols() / 2; j < mat_v[0].cols(); j++)
    {
      mat1[i][j - mat_v[0].cols() / 2] = mat_v[0][i][j];
    }
  }

  for (int i = mat_v[1].rows() / 2; i < mat_v[1].rows(); i++)
  {
    for (int j = 0; j < mat_v[1].cols(); j++)
    {
      mat2[i - mat_v[1].rows() / 2][j] = mat_v[1][i][j];
    }
  }

  lab2::matrix matrix1 = mat1 * mat2;

  // std::cout << matrix1 << std::endl;
  // std::cout << std::endl;

  for (int i = 0; i < matrix1.rows(); i++)
  {
    for (int j = 0; j < matrix1.cols(); j++)
    {
      matrix[i][j] += matrix1[i][j];
    }
  }
  
  auto end = std::chrono::steady_clock::now();

  auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);

  printf("[P]: time -> %ldms\n", elapsed_ms.count());


  begin = std::chrono::steady_clock::now();

  mat_v[0] * mat_v[1];

  end = std::chrono::steady_clock::now();

  elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);

  printf("[NP]: time -> %ldms\n", elapsed_ms.count());

  return 0;
}