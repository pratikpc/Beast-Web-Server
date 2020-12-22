#include <ServerAPI.hxx>

#include <boost/beast/core/ostream.hpp>

#include <boost/beast/http/message.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/http/write.hpp>

#include <boost/asio/basic_socket_acceptor.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>

#include <boost/lexical_cast.hpp>

#include <iostream>

//#define FunctionHeaderServerAPIWithTemplates \
//   template <typename RequestBody,           \
//             typename ResponseBody,          \
//             typename RequestInternal,       \
//             typename ResponseInternal>
//
//#define ClassParamsServerAPIWithTemplates \
//   RequestBody, ResponseBody, RequestInternal, ResponseInternal
//
#define FunctionHeaderServerAPIWithTemplates
#define ClassParamsServerAPIWithTemplates

#include <optional>

namespace PC::Lib::HTTPServer
{

   FunctionHeaderServerAPIWithTemplates std::optional<ServerAPI::RouteFn>
                                        ServerAPI /*<ClassParamsServerAPIWithTemplates>*/ ::RunCallback(
           Types::Methods method, std::string_view route) noexcept
   {
      for (auto [it, end] = routingTable.equal_range(method); it != end; ++it)
      {
         auto       routingTableElem = it->second;
         std::regex regexVal         = routingTableElem.first;
         if (!std::regex_match(std::string(route), regexVal))
         {
            continue;
         }
         // Return the function Callback
         return routingTableElem.second;
      }
      return std::nullopt;
   }
   FunctionHeaderServerAPIWithTemplates Net::awaitable<void>
                                        ServerAPI /*<ClassParamsServerAPIWithTemplates>*/ ::ExecuteSingle(
           tcp::socket socket, Net::use_awaitable_t<> awaitable)
   {
      try
      {
         Beast::tcp_stream stream{std::move(socket)};
         stream.expires_after(Timeout);
         Request<RequestBody> request = co_await ReadARequest(stream, awaitable);

         std::optional<RequestBody> const userData = UserAuth(request);

         Response<ResponseBody> response;
         if (userData.has_value())
         {
            request.UserData = userData.value();
            // Generate response to user's request
            response = co_await RespondToRequest(std::move(request), awaitable);
         }
         else
         {
            response = Response<ResponseBody>::UnAuthorized(ResponseBody{});
         }
         co_await WriteAResponse(stream, std::move(response), awaitable);
         stream.socket().shutdown(tcp::socket::shutdown_send);
      }
      catch (std::exception const& ex)
      {
         std::cout << "Error " << ex.what() << "\n";
         socket.shutdown(tcp::socket::shutdown_send);
      }
   }

   FunctionHeaderServerAPIWithTemplates Net::awaitable<void>
       ServerAPI /*<ClassParamsServerAPIWithTemplates>*/ ::operator()(
           tcp::endpoint const& endpoint, Net::use_awaitable_t<> awaitable)
   {
      // Get current executor
      auto executor = co_await Net::this_coro::executor;

      // Open the acceptor
      tcp::acceptor acceptor(executor, endpoint);

      // Allow address reuse
      acceptor.set_option(Net::socket_base::reuse_address(true));

      // Start listening for connections
      acceptor.listen(Net::socket_base::max_listen_connections);

      for (;;)
      {
         tcp::socket socket = co_await acceptor.async_accept(awaitable);
         Net::co_spawn(executor, ExecuteSingle(std::move(socket)), Net::detached);
         std::cout << "Execute socket End\n";
      }
   }

   FunctionHeaderServerAPIWithTemplates Net::awaitable<Request<ServerAPI::RequestBody>>
                                        ServerAPI /*<ClassParamsServerAPIWithTemplates>*/ ::ReadARequest(
           Beast::tcp_stream& stream, Net::use_awaitable_t<> awaitable)
   {
      Beast::flat_buffer readBuffer;

      Http::request<RequestInternal> req;

      std::cout << "Launch\n";
      auto const size = co_await Http::async_read(stream, readBuffer, req, awaitable);
      std::cout << "Read" << size << "bytes\n";
      
      Request<RequestBody> request;
      request.body = boost::lexical_cast<RequestBody>(req.body());

      // Set headers
      request.headers = req.base();
      request.method  = req.method();
      request.path    = req.target();

      co_return request;
   }
   FunctionHeaderServerAPIWithTemplates Net::awaitable<Response<ServerAPI::ResponseBody>>
                                        ServerAPI /*<ClassParamsServerAPIWithTemplates>*/ ::RespondToRequest(
           Request<ServerAPI::RequestBody>&& req, Net::use_awaitable_t<> awaitable)
   {
      auto callbackOpt = RunCallback(req.method, req.path);
      if (!callbackOpt.has_value())
         co_return Response<ServerAPI::ResponseBody>::NotFound(
             boost::lexical_cast<ResponseBody>(req.path + " Not found"));

      Response<ResponseBody> const response =
          co_await callbackOpt.value()(req, awaitable);
      co_return response;
   }
   FunctionHeaderServerAPIWithTemplates Net::awaitable<void>
                                        ServerAPI /*<ClassParamsServerAPIWithTemplates>*/ ::WriteAResponse(
           Beast::tcp_stream&                  stream,
           Response<ServerAPI::ResponseBody>&& response,
           Net::use_awaitable_t<>              awaitable)

   {
      Http::response<ResponseInternal> res{};
      res.result(response.status);
      res.base() = response.headers;
      res.set(Http::field::content_type, "text/plain");
      res.body() =
          boost::lexical_cast<typename RequestInternal::value_type>(response.body);
      res.prepare_payload();

      co_await Http::async_write(stream, res, awaitable);
   }
}; // namespace PC::Lib::HTTPServer

#undef FunctionHeaderServerAPIWithTemplates
#undef ClassParamsServerAPIWithTemplates