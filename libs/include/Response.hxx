#pragma once

#include <types.hxx>
namespace PC::Lib::HTTPServer
{
   namespace Types::Header
   {
      using Response = Http::response_header<Http::fields>;
   } // namespace Types::Header

   template <typename ResponseBody>
   struct Response
   {
      // HTTP Request Headers stored as key value pairs
      Types::Header::Response headers{};

      // Store the Response Status Code
      Types::StatusCode status;
      // Store the HTTP Response
      ResponseBody body;

      Response() : status{Types::StatusCode::not_implemented}, body{}
      {
      }
      Response(Types::StatusCode status, ResponseBody&& body) :
          status{status}, body{body}
      {
      }

      inline static Response<ResponseBody> Ok(decltype(body)&& body)
      {
         return Response(Types::StatusCode::ok, std::move(body));
      }
      inline static Response NotFound(decltype(body)&& body)
      {
         return Response(Types::StatusCode::not_found, std::move(body));
      }
      inline static Response<ResponseBody> UnAuthorized(decltype(body)&& body)
      {
         return Response(Types::StatusCode::unauthorized, std::move(body));
      }
   };

} // namespace PC::Lib::HTTPServer