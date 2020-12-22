#pragma once

#include <types.hxx>

namespace PC::Lib::HTTPServer
{
   namespace Types::Header
   {
      using Request = Http::request_header<Http::fields>;
   } // namespace Types::Header

   template <typename RequestInternal>
   struct Request
   {
      std::string path;

      // HTTP Request Headers stored as key value pairs
      Types::Header::Request headers;

      // Store the body of the HTTP Request
      RequestInternal body;
      // Store the body of the HTTP Request
      RequestInternal query;
      // Method
      Types::Methods method;
      // User information
      RequestInternal UserData;
   };
} // namespace PC::Lib::HTTPServer

