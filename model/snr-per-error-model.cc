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
#include <map>

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
SnrPerErrorModel::Receive (double snr, uint32_t bytes)
{ 
  NS_LOG_FUNCTION (this << snr << bytes);
  return DoReceive (snr, bytes);
}

double
SnrPerErrorModel::QFunction (double x) const
{
  NS_LOG_FUNCTION (this << x);
  // Chernoff bound for now
  double approx = ((1 - exp (-1.4 * x))*exp(-0.5 * x * x))/ (2.845 * x);
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
BpskSnrPerErrorModel::DoReceive (double snr, uint32_t bytes)
{ 
  NS_LOG_FUNCTION (this << snr << bytes);
  double ber = QFunction (sqrt(2*snr));
  return BerToPer (ber, bytes);
}

NS_OBJECT_ENSURE_REGISTERED (TableSnrPerErrorModel);

TypeId TableSnrPerErrorModel::GetTypeId (void)
{ 
  static TypeId tid = TypeId ("ns3::TableSnrPerErrorModel")
    .SetParent<SnrPerErrorModel> ()
    .SetGroupName("SimpleWireless")
    .AddConstructor<TableSnrPerErrorModel> ()
  ;
  return tid;
}

TableSnrPerErrorModel::TableSnrPerErrorModel ()
{
  NS_LOG_FUNCTION (this);
  m_cachedValue = std::pair<double, double> (std::numeric_limits<double>::max (), 0);
}

TableSnrPerErrorModel::~TableSnrPerErrorModel () 
{
  NS_LOG_FUNCTION (this);
}

double
TableSnrPerErrorModel::DoReceive (double snr, uint32_t bytes)
{ 
  NS_LOG_FUNCTION (this << snr << bytes);
  // Check cached value first
  if (m_cachedValue.first == snr)
    {
      NS_LOG_INFO ("Returning cached value for SNR " << snr << " of PER " << m_cachedValue.second);
      return m_cachedValue.second;
    }
  auto it = m_perMap.find (snr);
  if (it != m_perMap.end ())
    {
      NS_LOG_INFO ("Found exact match for SNR " << snr << " of PER " << it->second);
      m_cachedValue = std::pair<double, double> (snr, it->second);
      return it->second;
    }
  else
    {
       auto lower = m_perMap.lower_bound (snr);
       if (lower == m_perMap.begin ())
         {
           NS_LOG_INFO ("SNR " << snr << " is below lower bound of " << lower->first << "; returning 1");
           m_cachedValue = std::pair<double, double> (snr, 1);
           return 1;
         } 
       auto upper = m_perMap.upper_bound (snr);
       if (upper == m_perMap.end ())
         {
           NS_LOG_INFO ("SNR " << snr << " is above upper bound " << upper->first << "; returning 0");
           m_cachedValue = std::pair<double, double> (snr, 0);
           return 0;
         } 
       lower = upper; 
       lower--;
       double per = lower->second + (snr - lower->first)/ (upper->first - lower->first) * (upper->second - lower->second);
       NS_LOG_INFO ("SNR " << snr << " is interpolated between " << upper->first << " and " << lower->first << "; returning " << per);
       m_cachedValue = std::pair<double, double> (snr, per);
       return per;
    }
}

void
TableSnrPerErrorModel::AddValue (double snr, double per)
{
  NS_LOG_FUNCTION (this << snr << per);
  NS_ABORT_MSG_IF (per < 0 || per > 1, "Illegal PER value " << per);
  NS_ABORT_MSG_IF (snr < -100 || snr > 100, "Illegal SNR value " << snr);
  m_perMap.insert (std::pair<double, double> (snr, per));  
}

} // namespace ns3

