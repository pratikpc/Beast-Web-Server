#pragma once
namespace PC::Lib::HTTPServer
{
   namespace Beast = ::boost::beast;
   namespace Http  = Beast::http;
   namespace Net   = ::boost::asio;
} // namespace PC::Lib::HTTPServer

namespace PC::Lib::HTTPServer::Types
{
   using Methods    = Http::verb;
   using StatusCode = Http::status;
} // namespace PC::Lib::HTTPServer::Types
