/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 University of Washington
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <cmath>

#include "snr-per-error-model.h"

#include "ns3/assert.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("SnrPerErrorModel");

NS_OBJECT_ENSURE_REGISTERED (SnrPerErrorModel);

TypeId SnrPerErrorModel::GetTypeId (void)
{ 
  static TypeId tid = TypeId ("ns3::SnrPerErrorModel")
    .SetParent<Object> ()
    .SetGroupName("SimpleWireless")
  ;
  return tid;
}

SnrPerErrorModel::SnrPerErrorModel ()
{
  NS_LOG_FUNCTION (this);
}

SnrPerErrorModel::~SnrPerErrorModel ()
{
  NS_LOG_FUNCTION (this);
}

double
SnrPerErrorModel::Receive (double rxPowerDbm, double noisePowerDbm, uint32_t bytes)
{ 
  NS_LOG_FUNCTION (this << rxPowerDbm << noisePowerDbm << bytes);
  return DoReceive (rxPowerDbm, noisePowerDbm, bytes);
}

double
SnrPerErrorModel::QFunction (double x) const
{
  NS_LOG_FUNCTION (this << x);
  // Chernoff bound for now
  double approx = ((1 - exp (-1.4 * x))*exp(-0.5 * x * x))/ (5.04 * x);
  return approx;
}

double
SnrPerErrorModel::BerToPer (double ber, uint32_t bytes) const
{
  NS_LOG_FUNCTION (this << ber << bytes);
  return (1 - pow ((1 - ber), (8 * bytes)));
}

NS_OBJECT_ENSURE_REGISTERED (BpskSnrPerErrorModel);

TypeId BpskSnrPerErrorModel::GetTypeId (void)
{ 
  static TypeId tid = TypeId ("ns3::BpskSnrPerErrorModel")
    .SetParent<SnrPerErrorModel> ()
    .SetGroupName("SimpleWireless")
    .AddConstructor<BpskSnrPerErrorModel> ()
  ;
  return tid;
}


BpskSnrPerErrorModel::BpskSnrPerErrorModel ()
{
  NS_LOG_FUNCTION (this);
}

BpskSnrPerErrorModel::~BpskSnrPerErrorModel () 
{
  NS_LOG_FUNCTION (this);
}

double
BpskSnrPerErrorModel::DoReceive (double rxPowerDbm, double noisePowerDbm, uint32_t bytes)
{ 
  NS_LOG_FUNCTION (this << rxPowerDbm << noisePowerDbm << bytes);
  double snr = rxPowerDbm - noisePowerDbm;
  double ber = QFunction (sqrt(2*snr));
  return BerToPer (ber, bytes);
}

} // namespace ns3

