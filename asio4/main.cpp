#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <memory>

using namespace std;
namespace asio = boost::asio;
namespace pt = boost::posix_time;
namespace bs = boost::system;

class ScopeGuard { // (1)
public:
    ScopeGuard() {}
    ~ScopeGuard() { cout << "Good bye, Scope!\n"; }
};

int main() {
    unique_ptr<asio::io_service> ios1{new asio::io_service}; // (2)
    unique_ptr<asio::io_service> ios2{new asio::io_service}; // (3)
    asio::spawn(*ios1, [&ios1](asio::yield_context yield) {
        ScopeGuard sg; // (4)
        asio::deadline_timer timer(*ios1, pt::seconds(999999)); // (5)
        timer.async_wait(yield);
        cout << "Will it be printed?\n"; // (6)
    });
    asio::spawn(*ios2, [&ios1, &ios2](asio::yield_context yield) {
        asio::deadline_timer timer(*ios2, pt::seconds(3)); // (7)
        timer.async_wait(yield);
        ios1.reset(); // (8)
        cout << "Coroutine finish\n"; // (9)
    });
    while (true) { // (10)
        if (!ios1 || !ios2) break; // (11)
        ios1->poll(); // (12)
        ios2->poll(); // (13)
    }
    cout << "Exit\n"; // (14)
    return 0;
}
