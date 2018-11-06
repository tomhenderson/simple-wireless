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
 *
 */

#ifndef SNR_PER_ERROR_MODEL_H
#define SNR_PER_ERROR_MODEL_H

#include "ns3/object.h"

namespace ns3 {

class Packet;

class SnrPerErrorModel : public Object
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  SnrPerErrorModel ();
  virtual ~SnrPerErrorModel ();

  double QFunction (double x) const;

  double BerToPer (double ber, uint32_t bytes) const;

  double Receive (double rxPowerDbm, double noisePowerDbm, uint32_t bytes);
private:
  /**
   * Corrupt a packet according to the specified model.
   * \param p the packet to corrupt
   * \returns true if the packet is corrupted
   */
  virtual double DoReceive (double rxPowerDbm, double noisePowerDbm, uint32_t bytes) = 0;
};

class BpskSnrPerErrorModel : public SnrPerErrorModel
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  BpskSnrPerErrorModel ();
  virtual ~BpskSnrPerErrorModel ();

private:
  double DoReceive (double rxPowerDbm, double noisePowerDbm, uint32_t bytes);
};
} // namespace ns3
#endif
