/************************************************************************
 * Copyright(c) 2023, One Unified. All rights reserved.                 *
 * email: info@oneunified.net                                           *
 *                                                                      *
 * This file is provided as is WITHOUT ANY WARRANTY                     *
 *  without even the implied warranty of                                *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                *
 *                                                                      *
 * This software may not be used nor distributed without proper license *
 * agreement.                                                           *
 *                                                                      *
 * See the file LICENSE.txt for redistribution information.             *
 ************************************************************************/

/*
 * File:    Config.cpp
 * Author:  raymond@burkholder.net
 * Project: Mqtt2Telegram
 * Created: October 22, 2023 12:56:29
 */

#include <vector>
#include <fstream>
#include <exception>

#include <boost/log/trivial.hpp>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include "Config.hpp"

namespace {
  static const std::string sValue_Mqtt_Id( "mqtt_id" );
  static const std::string sValue_Mqtt_Host( "mqtt_host" );
  static const std::string sValue_Mqtt_UserName( "mqtt_username" );
  static const std::string sValue_Mqtt_Password( "mqtt_password" );
  static const std::string sValue_Mqtt_Topic( "mqtt_topic" );

  static const std::string sValue_Telegram_Token( "telegram_token" );
  static const std::string sValue_Telegram_ChatId( "telegram_chat_id" );

  template<typename T>
  bool parse( const std::string& sFileName, po::variables_map& vm, const std::string& name, T& dest ) {
    bool bOk = true;
    if ( 0 < vm.count( name ) ) {
      dest = vm[name].as<T>();
      BOOST_LOG_TRIVIAL(info) << name << " = " << dest;
    }
    else {
      BOOST_LOG_TRIVIAL(error) << sFileName << " missing '" << name << "='";
      bOk = false;
    }
  return bOk;
  }
}

namespace config {

bool Load( const std::string& sFileName, Values& values ) {

  bool bOk( true );

  try {

    std::string sDateHistory;
    std::string sDateTrading;
    unsigned int nDaysFront;
    unsigned int nDaysBack;

    po::options_description config( "MQTT2Telegram Config" );
    config.add_options()

      ( sValue_Mqtt_Id.c_str(), po::value<std::string>( &values.mqtt.sId ), "mqtt client id" )
      ( sValue_Mqtt_Host.c_str(), po::value<std::string>( &values.mqtt.sHost )->default_value( "localhost" ), "mqtt host address or name" )
      ( sValue_Mqtt_UserName.c_str(), po::value<std::string>( &values.mqtt.sUserName ), "mqtt username" )
      ( sValue_Mqtt_Password.c_str(), po::value<std::string>( &values.mqtt.sPassword ), "mqtt password" )
      ( sValue_Mqtt_Topic.c_str(), po::value<std::string>( &values.mqtt.sTopic )->default_value( "nut/" ), "mqtt topic" )

      ( sValue_Telegram_Token.c_str(), po::value<std::string>( &values.telegram.sToken ), "telegram token" )
      ( sValue_Telegram_ChatId.c_str(), po::value<uint64_t>( &values.telegram.idChat ), "telegram chat id" )
      ;
    po::variables_map vm;
    //po::store( po::parse_command_line( argc, argv, config ), vm );

    std::ifstream ifs( sFileName.c_str() );

    if ( !ifs ) {
      BOOST_LOG_TRIVIAL(error) << "nut2mqtt config file " << sFileName << " does not exist";
      bOk = false;
    }
    else {
      po::store( po::parse_config_file( ifs, config), vm );

      bOk &= parse<std::string>( sFileName, vm, sValue_Mqtt_Id, values.mqtt.sId );
      bOk &= parse<std::string>( sFileName, vm, sValue_Mqtt_Host, values.mqtt.sHost );
      bOk &= parse<std::string>( sFileName, vm, sValue_Mqtt_UserName, values.mqtt.sUserName );
      bOk &= parse<std::string>( sFileName, vm, sValue_Mqtt_Password, values.mqtt.sPassword );
      bOk &= parse<std::string>( sFileName, vm, sValue_Mqtt_Topic, values.mqtt.sTopic );

      bOk &= parse<std::string>( sFileName, vm, sValue_Telegram_Token, values.telegram.sToken );
      bOk &= parse<uint64_t>( sFileName, vm, sValue_Telegram_ChatId, values.telegram.idChat );
    }

  }
  catch( std::exception& e ) {
    BOOST_LOG_TRIVIAL(error) << sFileName << " parse error: " << e.what();
    bOk = false;
  }

  return bOk;
}

} // namespace config
