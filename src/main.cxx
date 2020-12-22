#define BOOST_ASIO_HAS_STD_COROUTINE
#define BOOST_ASIO_HAS_CO_AWAIT

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include <iostream>
#include <optional>
#include <regex>
#include <string>
#include <thread>

#include <ServerAPI.hxx>

int main()
{
   try
   {
      boost::asio::io_context io_context(3);

      boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
      signals.async_wait([&](auto, auto) {
         std::cout << "Stopping server\n";
         io_context.stop();
      });

      PC::Lib::HTTPServer::ServerAPI api;
      api.Get(std::regex("/"),
              [](PC::Lib::HTTPServer::Request<std::string>&& x,
                 boost::asio::use_awaitable_t<>)
                  -> boost::asio::awaitable<PC::Lib::HTTPServer::Response<std::string>> {
                 std::this_thread::sleep_for(std::chrono::seconds(4));
                 co_return PC::Lib::HTTPServer::Response<std::string>{
                     boost::beast::http::status::ok, "Hi"};
              });
      boost::asio::co_spawn(
          io_context, api({boost::asio::ip::tcp::v4(), 55555}), boost::asio::detached);
      // Run the I/O service on the requested number of threads
      static auto constexpr threads = 3;

      std::vector<std::thread> v;
      v.reserve(threads - 1);
      for (auto i = threads - 1; i > 0; --i)
         v.emplace_back([&io_context] {
            std::cout << std::this_thread::get_id() << " started\n";
            io_context.run();
            std::cout << std::this_thread::get_id() << " over\n";
         });
      std::cout << std::this_thread::get_id() << " started\n";
      io_context.run();
      std::cout << std::this_thread::get_id() << " overed\n";
      for (auto& thread : v)
         thread.join();
   }
   catch (...)
   {
      std::cout << "Exception";
   }
}
