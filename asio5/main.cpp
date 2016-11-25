#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <memory>

using namespace std;
namespace asio = boost::asio;
namespace pt = boost::posix_time;
namespace bs = boost::system;

class ScopeGuard {
public:
    ScopeGuard() {}
    ~ScopeGuard() { cout << "Good bye, Scope!\n"; }
};

template<typename Token>
auto async_wait(asio::io_service& ios, int seconds, Token&& token)
{
  using handler_type = typename asio::handler_type
                       <Token, void(bs::error_code, pt::ptime)>::type;
  handler_type handler(std::forward<Token>(token));
  asio::async_result<handler_type> result(handler);
  ios.post([&ios, handler, seconds]() mutable {
      asio::spawn(ios, [&ios, handler, seconds](asio::yield_context yield) mutable {
          asio::deadline_timer timer(ios, pt::seconds(seconds));
          timer.async_wait(yield);
          handler(bs::error_code(), pt::second_clock::local_time());
      });
  });
  return result.get();
}

int main() {
    unique_ptr<asio::io_service> ios1{new asio::io_service};
    unique_ptr<asio::io_service> ios2{new asio::io_service};
    asio::spawn(*ios1, [&ios1](asio::yield_context yield) {
        ScopeGuard sg;
        async_wait(*ios1, 999999, yield); // (1)
        cout << "Will it be printed?\n";
    });
    asio::spawn(*ios2, [&ios1, &ios2](asio::yield_context yield) {
        async_wait(*ios2, 3, yield); // (2)
        ios1.reset();
        cout << "Coroutine finish\n";
    });
    while (true) {
        if (!ios1 || !ios2) break;
        ios1->poll();
        ios2->poll();
    }
    cout << "Exit\n";
    return 0;
}
