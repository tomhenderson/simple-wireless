/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright 2018 University of Washington
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

// This is a program to evaluate link performance
//
// Network topology:
//
//  Simple wireless 192.168.1.0
//
//   sender              receiver 
//    * <-- distance -->  *
//    |                   |
//    n0                  n1
//
// Users may vary the following command-line arguments in addition to the
// attributes, global values, and default values typically available:
//
//    (to be completed)
//

#include <fstream>
#include <iostream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/propagation-module.h"
#include "ns3/simple-wireless-channel.h"
#include "ns3/simple-wireless-net-device.h"
#include "ns3/snr-per-error-model.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("LinkPerformanceExample");

std::ofstream g_fileRssi;

void
ReceiveTrace (Ptr<const Packet> p, double rxPower, Mac48Address from)
{
  g_fileRssi << Simulator::Now ().GetSeconds () << " " << rxPower << std::endl;
}

int
main (int argc, char *argv[])
{
  uint16_t serverPort = 9;
  DataRate dataRate = DataRate ("1Mbps");
  double distance = 25.0; // meters
  double maxPackets = 1000;
  double noisePower = -100; // dbm

  CommandLine cmd;
  cmd.AddValue("distance","the distance between the two nodes",distance);
  cmd.AddValue("maxPackets","the number of packets to send",maxPackets);
  cmd.AddValue("noisePower","noise power in dBm",noisePower);
  cmd.Parse (argc, argv);

  g_fileRssi.open ("link-performance-rssi.dat", std::ofstream::out);
  
  Ptr<Node> senderNode = CreateObject<Node> ();
  Ptr<Node> receiverNode = CreateObject<Node> ();
  NodeContainer nodes;
  nodes.Add (senderNode);
  nodes.Add (receiverNode);

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAllocator = CreateObject<ListPositionAllocator> ();
  positionAllocator->Add (Vector (0.0, 0.0, 0.0));
  positionAllocator->Add (Vector (distance, 0.0, 0.0));
  mobility.SetPositionAllocator (positionAllocator);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);

  auto lossModel = CreateObject<FriisPropagationLossModel> ();
  lossModel->SetFrequency (5e9);  // 5 GHz

  NetDeviceContainer devices;
  Ptr<SimpleWirelessChannel> channel = CreateObject<SimpleWirelessChannel> ();
  channel->AddPropagationLossModel (lossModel);
  Ptr<SimpleWirelessNetDevice> senderDevice = CreateObject<SimpleWirelessNetDevice> ();
  senderDevice->SetChannel (channel);
  senderDevice->SetNode (senderNode);
  senderDevice->SetAddress (Mac48Address::Allocate ());
  senderDevice->SetDataRate (dataRate);
  senderDevice->SetNoisePower (noisePower);
  senderNode->AddDevice (senderDevice);
  devices.Add (senderDevice);
  Ptr<SimpleWirelessNetDevice> receiverDevice = CreateObject<SimpleWirelessNetDevice> ();
  receiverDevice->SetChannel (channel);
  receiverDevice->SetNode (receiverNode);
  receiverDevice->SetAddress (Mac48Address::Allocate ());
  receiverDevice->SetDataRate (dataRate);
  receiverDevice->SetNoisePower (noisePower);
  receiverDevice->TraceConnectWithoutContext ("PhyRxEnd", MakeCallback (&ReceiveTrace));
  Ptr<BpskSnrPerErrorModel> errorModel = CreateObject<BpskSnrPerErrorModel> ();
  receiverDevice->SetSnrPerErrorModel (errorModel);
  receiverNode->AddDevice (receiverDevice);
  devices.Add (receiverDevice);

  InternetStackHelper stack;
  stack.Install (nodes);

  Ipv4AddressHelper address;
  address.SetBase ("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = address.Assign (devices);

  UdpEchoServerHelper echoServer (serverPort);
  ApplicationContainer serverApps = echoServer.Install (nodes.Get (1));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient (interfaces.GetAddress (1), serverPort);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (maxPackets));
  echoClient.SetAttribute ("Interval", TimeValue (MilliSeconds (100)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install (nodes.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  Simulator::Run ();
  Simulator::Destroy ();
  g_fileRssi.close ();
  return 0;
}
