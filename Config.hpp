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
 * File:    Config.hpp
 * Author:  raymond@burkholder.net
 * Project: Mqtt2Telegram
 * Created: October 22, 2023 12:56:29
 */

#pragma once

#include <string>

#include <ou/mqtt/config.hpp>

namespace config {

struct Telegram {
  std::string sToken;
   uint64_t idChat;
};

struct Values {

  Telegram telegram;
  ou::mqtt::Config mqtt;

};

bool Load( const std::string& sFileName, Values& );

} // namespace config