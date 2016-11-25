#include <iostream>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace std;
namespace asio = boost::asio;
namespace pt = boost::posix_time;
namespace bs = boost::system;

int main() {
    asio::io_service ios; // (1)
    asio::deadline_timer timer(ios, pt::seconds(3)); // (2)
    timer.async_wait([](bs::error_code){ cout << "Hello World!\n"; }); // (3)
    ios.run(); // (4)
    return 0;
}
