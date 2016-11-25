#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace std;
namespace asio = boost::asio;
namespace pt = boost::posix_time;
namespace bs = boost::system;

template<typename Token>
typename boost::asio::async_result
<typename boost::asio::handler_type<Token,
                                    void(bs::error_code, pt::ptime)
                                    >::type
>::type
async_wait(asio::io_service& ios, int seconds, Token&& token) // (3)
{
  using handler_type = typename asio::handler_type
                       <Token, void(bs::error_code, pt::ptime)>::type; // (4)
  handler_type handler(std::forward<Token>(token)); // (5)
  asio::async_result<handler_type> result(handler); // (6)
  ios.post([&ios, handler, seconds]() mutable { // (7)
      asio::spawn(ios, [&ios, handler, seconds](asio::yield_context yield) mutable { // (8)
          asio::deadline_timer timer(ios, pt::seconds(seconds));
          timer.async_wait(yield);
          handler(bs::error_code(), pt::second_clock::local_time()); // (9)
      });
  });
  return result.get(); // (8)
}

int main() {
    asio::io_service ios;
    async_wait(ios, 1, [](bs::error_code, pt::ptime time) { // (1)
        cout << "Hello World 1: " << time << "\n";
    });
    asio::spawn(ios, [&ios](asio::yield_context yield) {
        pt::ptime time = async_wait(ios, 3, yield); // (2)
        cout << "Hello World 2: " << time << "\n";
    });
    ios.run();
    return 0;
}
