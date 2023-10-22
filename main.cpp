/*
  project: Mqtt2Telegram
  author:  raymond@burkholder.net
  date:    2023/10/22 12:32:29
*/

#include <iostream>

#include <boost/asio/io_context.hpp>

#include "Loop.hpp"
#include "Config.hpp"

namespace asio = boost::asio;

int main( int argc, char **argv ) {

  static const std::string c_sConfigFilename( "mqtt2telegram.cfg" );

  std::cout << "MQTT2Telegram (c)2023 One Unified Net Limited" << std::endl;

  config::Values choices;

  if ( Load( c_sConfigFilename, choices ) ) {
  }
  else {
    return EXIT_FAILURE;
  }

  asio::io_context io_context;

  try {
    Loop loop( choices, io_context );
    io_context.run();
  }
  catch(...) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
