/*
  can_filtering.h: Module to handle filtering of CAN-Packets.

  (C) Copyright 2009 - 2022 by OSB connagtive GmbH

  Use, modification and distribution are subject to the GNU General
  Public License, see accompanying file LICENSE.txt
*/
#ifndef CAN_FILTERING_H
#define CAN_FILTERING_H

#include <string>
#include "can_server_interface.h"

// Author: Martin Wodok
namespace can_filtering
{
  std::string config( const std::string &cmd );
  bool pass( unsigned bus, uint32_t id, unsigned dlc, uint8_t *databytes );
}

#endif
