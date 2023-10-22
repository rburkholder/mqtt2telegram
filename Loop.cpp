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
 * File:    Loop.cpp
 * Author:  raymond@burkholder.net
 * Project: Mqtt2Telegram
 * Created: October 23, 2023 13:43:38
 */

#include <unistd.h>
#include <limits.h>

#include <iostream>

#include <boost/log/trivial.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>

#include <boost/asio/post.hpp>

#include <nutclient.h>

#include <Telegram/Bot.hpp>

#include "Loop.hpp"
#include "Config.hpp"
#include "mqtt.hpp"

Loop::Loop( const config::Values& choices, asio::io_context& io_context )
: m_choices( choices ), m_io_context( io_context )
, m_signals( io_context, SIGINT ) // SIGINT is called '^C'
{

  int rc;
  char szHostName[ HOST_NAME_MAX + 1 ];
  rc = gethostname( szHostName, HOST_NAME_MAX + 1 );
  if ( 0 != rc ) {
    m_sMqttId = szHostName;
  }
  else {
    assert( 0 < choices.mqtt.sId.size() );
    m_sMqttId = choices.mqtt.sId;
  }

  // https://www.boost.org/doc/libs/1_79_0/doc/html/boost_asio/reference/signal_set.html

  //signals.add( SIGKILL ); // not allowed here
  m_signals.add( SIGHUP ); // use this as a config change?
  //signals.add( SIGINFO ); // control T - doesn't exist on linux
  m_signals.add( SIGTERM );
  m_signals.add( SIGQUIT );
  m_signals.add( SIGABRT );

  m_signals.async_wait( [this]( const boost::system::error_code& error_code, int signal_number ){
    Signals( error_code, signal_number );
  } );

  try {
    if ( m_choices.telegram.sToken.empty() ) {
      BOOST_LOG_TRIVIAL(warning) << "telegram: no token available" << std::endl;
    }
    else {
      m_telegram_bot = std::make_unique<telegram::Bot>( m_choices.telegram.sToken );

      //auto id = m_telegram_bot->GetChatId();
      m_telegram_bot->SetChatId( choices.telegram.idChat );
      //Telegram_SendMessage();

    }
  }
  catch (...) {
    BOOST_LOG_TRIVIAL(error) << "telegram open failure";
  }

  try {
    m_pMqtt = std::make_unique<Mqtt>( choices, choices.mqtt.sId.c_str() );

    m_pMqtt->Subscribe(
      choices.mqtt.sTopic + "/#",
      [this]( const std::string_view& svTopic, const std::string_view& svMessage ){
        // nut/furnace-sm1500 {"battery.charge":100,"battery.runtime":6749,"battery.voltage":26.4,"ups.status":"OL"}
        std::string sTopic( svTopic );
        std::string sMessage( svMessage );

        static const boost::regex expr {"\"ups.status\":\"([^\"]+)\""};
        boost::smatch what;
        if ( boost::regex_search( sMessage, what, expr ) ) {
          umapStatus_t::iterator iterStatus = m_umapStatus.find( sTopic );
          //std::cout << "what0" << what[0] << '\n';
          //std::cout << "what1" << what[1] << '\n';
          if ( m_umapStatus.end() == iterStatus ) {
            m_umapStatus.emplace( sTopic, what[ 1 ] );
            m_telegram_bot->SendMessage( sTopic + ": " + what[ 1 ]  );
          }
          else {
            if ( what[ 1 ] == iterStatus->second ) {
              // nothing to do
            }
            else {
              iterStatus->second = what[ 1 ];
              m_telegram_bot->SendMessage( sTopic + ": " + what[ 1 ]  );
            }
          }
        }
      } );
  }
  catch ( const Mqtt::runtime_error& e ) {
    BOOST_LOG_TRIVIAL(error) << "mqtt error: " << e.what() << '(' << e.rc << ')';
    throw e;
  }

  m_pWorkGuard = std::make_unique<work_guard_t>( asio::make_work_guard( io_context ) );
  //asio::post( m_io_context, std::bind( &Loop::Poll, this, true, choices.nut.bEnumerate ) );

  std::cout << "ctrl-c to end" << std::endl;

}

void Loop::Signals( const boost::system::error_code& error_code, int signal_number ) {

  BOOST_LOG_TRIVIAL(info)
    << "signal"
    << "(" << error_code.category().name()
    << "," << error_code.value()
    << "," << signal_number
    << "): "
    << error_code.message()
    ;

  bool bContinue( true );

  switch ( signal_number ) {
    case SIGHUP:
      BOOST_LOG_TRIVIAL(info) << "sig hup no-op";
      break;
    case SIGTERM:
      BOOST_LOG_TRIVIAL(info) << "sig term";
      bContinue = false;
      break;
    case SIGQUIT:
      BOOST_LOG_TRIVIAL(info) << "sig quit";
      bContinue = false;
      break;
    case SIGABRT:
      BOOST_LOG_TRIVIAL(info) << "sig abort";
      bContinue = false;
      break;
    case SIGINT:
      BOOST_LOG_TRIVIAL(info) << "sig int";
      bContinue = false;
      break;
    case SIGPIPE:
      BOOST_LOG_TRIVIAL(info) << "sig pipe";
      bContinue = false;
      break;
    default:
      break;
  }

  if ( bContinue ) {
    m_signals.async_wait( [this]( const boost::system::error_code& error_code, int signal_number ){
    Signals( error_code, signal_number );
  } );
  }
  else {
    m_pWorkGuard.reset();
    bContinue = false;
  }
}

void Loop::Telegram_GetMe() {
  if ( m_telegram_bot ) {
    m_telegram_bot->GetMe();
  }
  else {
    std::cout << "telegram bot is not available" << std::endl;
  }
}

void Loop::Telegram_SendMessage() {
  if ( m_telegram_bot ) {
    m_telegram_bot->SendMessage( "mqtt2telegram test" );
  }
  else {
    std::cout << "telegram bot is not available" << std::endl;
  }
}

Loop::~Loop() {
  m_pWorkGuard.reset();
  m_telegram_bot.reset();
  m_pMqtt->UnSubscribe( m_choices.mqtt.sTopic + "/#" );
  m_pMqtt.reset();
}
