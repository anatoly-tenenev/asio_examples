#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace std;
namespace asio = boost::asio;
namespace pt = boost::posix_time;
namespace bs = boost::system;

int main() {
    asio::io_service ios; // (1)
    asio::spawn(ios, [&ios](asio::yield_context yield) { // (2)
        asio::deadline_timer timer(ios, pt::seconds(3)); // (3)
        timer.async_wait(yield); // (4)
        cout << "Hello World!\n";
    });
    ios.run(); // (5)
    return 0;
}
