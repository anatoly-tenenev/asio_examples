#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <memory>
#include <unordered_map>

using namespace std;
namespace asio = boost::asio;
namespace pt = boost::posix_time;
namespace bs = boost::system;

class ScopeGuard {
public:
    ScopeGuard() {}
    ~ScopeGuard() { cout << "Good bye, Scope!\n"; }
};

class TimerService : public asio::io_service::service { // (9)
public:
    struct implementation_type { // (10)
        pt::ptime datetime;
    };
    typedef implementation_type impl_type;

    TimerService(asio::io_service& ios) : service(ios) {} // (11)
    ~TimerService() {}

    static asio::io_service::id id; // (12)
    void construct(impl_type&) {} // (13)
    void destroy(impl_type&) {} // (14)

    template<typename Handler>
    void async_wait(impl_type& impl, Handler&& handler) { // (15)
        unique_ptr<asio::deadline_timer> timer{new asio::deadline_timer(get_io_service(),
                                                                        impl.datetime)}; // (16)
        timer->async_wait([this, &impl, _handler=std::move(handler)](bs::error_code) mutable { // (17)
            get_io_service().post([this, &impl, _handler]() mutable { // (18)
                _handler(bs::error_code()); // (19)
                m_timers.erase(&impl); // (20)
            });
        });
        m_timers[&impl] = std::move(timer); // (21)
    }
private:
    unordered_map<void*, unique_ptr<asio::deadline_timer>> m_timers; // (22)
    void shutdown_service() {} // (23)
};
asio::io_service::id TimerService::id;

class CustomTimer : public asio::basic_io_object<TimerService> { // (3)
public:
    CustomTimer(asio::io_service& ios, const pt::time_duration& td) :
        basic_io_object(ios) // (4)
    {
        auto& impl = get_implementation(); // (5)
        impl.datetime = pt::second_clock::universal_time() + td; // (6)
    }

    template<typename Token>
    auto async_wait(Token&& token) { // (7)
        using handler_type = typename asio::handler_type
                             <Token, void(bs::error_code)>::type;
        handler_type handler(std::forward<Token>(token));
        asio::async_result<handler_type> result(handler);
        get_service().async_wait(get_implementation(), std::move(handler)); // (8)
        return result.get();
    }
};

int main() {
    unique_ptr<asio::io_service> ios1{new asio::io_service};
    unique_ptr<asio::io_service> ios2{new asio::io_service};
    asio::spawn(*ios1, [&ios1](asio::yield_context yield) {
        ScopeGuard sg;
        CustomTimer timer(*ios1, pt::seconds(999999)); // (1)
        timer.async_wait(yield);
        cout << "Will it be printed?\n";
    });
    asio::spawn(*ios2, [&ios1, &ios2](asio::yield_context yield) {
        CustomTimer timer(*ios2, pt::seconds(3)); // (2)
        timer.async_wait(yield);
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
