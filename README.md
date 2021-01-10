# Beast-Web-Server

### Why Coroutines?

Callback hell is bad and gives me headaches.    
Okay callbacks are not bad but its easier to write bad code which looks like 
1. Involves tons of magic and weird recursion
2. Needs one to write 10 lines about what the code does
3. Needs one to update those 10 lines once the code changes (yeah right?)

Coroutine codes look a lot more linear instead, sometimes end up being shorter and more easier to understand.

Compare
[Full Sample here](https://www.boost.org/doc/libs/develop/doc/html/boost_asio/example/cpp17/coroutines_ts/echo_server.cpp)
```cpp
    char data[1024];
    for (;;)
    {
      std::size_t n = co_await socket.async_read_some(boost::asio::buffer(data), use_awaitable);
      co_await async_write(socket, boost::asio::buffer(data, n), use_awaitable);
    }
```

vs
[Full Sample here](https://www.boost.org/doc/libs/develop/doc/html/boost_asio/example/cpp11/echo/async_tcp_echo_server.cpp)
```cpp
  void do_read()
  {
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        [this, self](boost::system::error_code ec, std::size_t length)
        {
            do_write(length);
        });
  }

  void do_write(std::size_t length)
  {
    boost::asio::async_write(socket_, boost::asio::buffer(data_, length),
        [this, self](boost::system::error_code, std::size_t /*length*/)
        {
            do_read();
        });
  }
```

####What does the above code do @pratikpc?    
It's a glorified echo server. A bit less obvious from the second sample I presume.

### Simplified usage

```cpp
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
          io_context, api({boost::asio::ip::tcp::v4(), PORT}), boost::asio::detached);
```

For a full sample, check https://github.com/pratikpc/Beast-Web-Sample/blob/main/src/main.cxx

### Why?

It's a simple proof of concept and it was fun to write

### References. I want to build my own

Refer to docs on Coroutines.
1. [Refer to Boost ASIO Docs](https://www.boost.org/doc/libs/develop/doc/html/boost_asio/example/cpp17/coroutines_ts/refactored_echo_server.cpp)
2. [Refer to Boost Beast Boost Coroutine docs](https://www.boost.org/doc/libs/develop/libs/beast/example/http/client/coro/http_client_coro.cpp)
3. A bit of Boost Beast code might look a bit anti-pattern when it comes to C++ Coroutines because a bit of it is so remember to focus on Boost.ASIO code well.

### Dependencies

1. CMake
2. Compiler that supports C++ Coroutines.
3. Dependency installation will be handled via VCPKG

### Use with my own CMake based project

```cmake
target_link_libraries(${PROJECT_NAME}
PUBLIC
libs::Beast-Web
)
```

For a full sample check https://github.com/pratikpc/Beast-Web-Sample/blob/main/src/CMakeLists.txt
