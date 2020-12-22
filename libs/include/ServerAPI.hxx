#pragma once

#include <Request.hxx>
#include <Response.hxx>
#include <types.hxx>

#include <hedley.h>

#ifdef PC_LIB_HTTP_SERVER_BUILD_MODE
#   define PC_LIB_HTTP_SERVER_SHARED_LIB_ACCESS HEDLEY_PUBLIC
#else
#   define PC_LIB_HTTP_SERVER_SHARED_LIB_ACCESS HEDLEY_IMPORT
#endif

namespace PC::Lib::HTTPServer
{
   namespace
   {

      using tcp = Net::ip::tcp;
   }
   namespace Types
   {
      using Route = ::std::regex;
      template < // Internal Representation of Request Beast Body
          typename RequestBody = std::string,
          // Internal Representation of Request Beast Body
          typename ResponseBody = std::string>

      using RouteFn = ::std::function<Net::awaitable<Response<ResponseBody>>(
          Request<RequestBody>, Net::use_awaitable_t<>)>;
   } // namespace Types
   // template <
   //    // Request Body accessed by the User
   //    typename RequestBody = ::std::string,
   //    // Response Body sent to us by the User
   //    typename ResponseBody = ::std::string,
   //    // Internal Representation of Request Beast Body
   //    typename RequestInternal = Http::string_body,
   //    // Internal Representation of Request Beast Body
   //    typename ResponseInternal = Http::string_body>
   class ServerAPI
   {
    public:
      // Request Body accessed by the User
      using RequestBody = ::std::string;
      // Response Body sent to us by the User
      using ResponseBody = ::std::string;
      // Internal Representation of Request Beast Body
      using RequestInternal = Http::string_body;
      // Internal Representation of Request Beast Body
      using ResponseInternal = Http::string_body;

      using RouteFn = Types::RouteFn<RequestBody, ResponseBody>;

    private:
      std::unordered_multimap<Types::Methods, std::pair<Types::Route, RouteFn>>
          routingTable{};

    public:
      // Use this to perform User Auth
      std::function<std::optional<RequestBody>(Request<RequestBody> const&)> UserAuth{
          [](Request<RequestBody> const&) { return std::make_optional(RequestBody{}); }};

      std::chrono::seconds Timeout{30};

      // Add Routes and Callback
      // To the Routing Table
      inline ServerAPI&
          Route(Types::Methods method, Types::Route&& route, RouteFn&& callback)
      {
         routingTable.emplace(method, std::make_pair(route, callback));
         return *this;
      }
      HEDLEY_ALWAYS_INLINE ServerAPI& Get(Types::Route&& route,
                                          RouteFn&&      callback) noexcept
      {
         return Route(Types::Methods::get, std::move(route), std::move(callback));
      }
      HEDLEY_ALWAYS_INLINE ServerAPI& Post(Types::Route&& route,
                                           RouteFn&&      callback) noexcept
      {
         return Route(Types::Methods::post, std::move(route), std::move(callback));
      }
      HEDLEY_ALWAYS_INLINE ServerAPI& Put(Types::Route&& route,
                                          RouteFn&&      callback) noexcept
      {
         return Route(Types::Methods::put, std::move(route), std::move(callback));
      }
      HEDLEY_ALWAYS_INLINE ServerAPI& Delete(Types::Route&& route,
                                             RouteFn&&      callback) noexcept
      {
         return Route(Types::Methods::delete_, std::move(route), std::move(callback));
      }

    public:
      PC_LIB_HTTP_SERVER_SHARED_LIB_ACCESS Net::awaitable<void>
                                           operator()(tcp::endpoint const&   endpoint,
                     Net::use_awaitable_t<> awaitable = Net::use_awaitable);

    private:
      Net::awaitable<void>
          ExecuteSingle(tcp::socket          socket,
                        Net::use_awaitable_t<> awaitable = Net::use_awaitable);

      Net::awaitable<Request<RequestBody>>
          ReadARequest(Beast::tcp_stream&     stream,
                       Net::use_awaitable_t<> awaitable = Net::use_awaitable);
      Net::awaitable<Response<ResponseBody>>
          RespondToRequest(Request<RequestBody>&& request,
                           Net::use_awaitable_t<> awaitable = Net::use_awaitable);
      Net::awaitable<void>
          WriteAResponse(Beast::tcp_stream&       stream,
                         Response<ResponseBody>&& response,
                         Net::use_awaitable_t<>   awaitable = Net::use_awaitable);
      // Execute the Route
      std::optional<ServerAPI::RouteFn> RunCallback(Types::Methods   method,
                                                    std::string_view path) noexcept;
   };
} // namespace PC::Lib::HTTPServer

#undef PC_LIB_HTTP_SERVER_SHARED_LIB_ACCESS