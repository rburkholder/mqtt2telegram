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
 * File:    Loop.hpp
 * Author:  raymond@burkholder.net
 * Project: Mqtt2Telegram
 * Created: October 23, 2023 13:43:38
 */

#include <set>
#include <memory>
#include <string>

#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/executor_work_guard.hpp>

namespace config {
  class Values;
}

class Mqtt;

namespace telegram {
  class Bot;
}

namespace asio = boost::asio; // from <boost/asio/context.hpp>

class Loop {
public:

  Loop( const config::Values&, asio::io_context& );
  ~Loop();

protected:
private:

  const config::Values& m_choices;
  asio::io_context& m_io_context;

  std::string m_sMqttId;

  std::unique_ptr<Mqtt> m_pMqtt;
  std::unique_ptr<telegram::Bot> m_telegram_bot;

  struct ups_state_t {
    std::string sStatus_basic;
    std::string sStatus_full;
    std::string sRunTime;

    ups_state_t( const std::string& sStatus_basic_ )
    : sStatus_basic( sStatus_basic_ )
    {}
    ups_state_t( const std::string& sStatus_basic_,
                 const std::string& sStatus_full_,
                 const std::string& sRunTime_ )
    : sStatus_basic( sStatus_basic_ )
    , sStatus_full( sStatus_full_ )
    , sRunTime( sRunTime_ )
    {}
    ups_state_t( ups_state_t&& state )
    : sStatus_basic( std::move( state.sStatus_basic ) )
    , sStatus_full( std::move( state.sStatus_full ) )
    , sRunTime( std::move( state.sRunTime ) )
    {}
  };

  using umapStatus_t = std::unordered_map<std::string,ups_state_t>;
  umapStatus_t m_umapStatus;

  using work_guard_t = asio::executor_work_guard<asio::io_context::executor_type>;
  using pWorkGuard_t = std::unique_ptr<work_guard_t>;

  pWorkGuard_t m_pWorkGuard;

  asio::signal_set m_signals;

  void Signals( const boost::system::error_code&, int );

  void Telegram_GetMe();
  void Telegram_SendMessage();

};