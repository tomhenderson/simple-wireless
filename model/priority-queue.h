/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (C) 2015 Massachusetts Institute of Technology
 * Copyright (c) 2007 University of Washington
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

#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/queue.h"
#include "ns3/packet.h"

#include <pcap.h>

#undef DLT_IEEE802_11_RADIO // Avoid namespace collision with ns3::YansWifiPhyHelper::DLT_IEEE802_11_RADIO

namespace ns3 {

/**
 * \ingroup queue
 *
 * \brief A strict priority queue with two subqueues, one for control packets and one for data
 */
template <typename Item>
class PriorityQueue : public Queue<Item>
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  /**
   * \brief PriorityQueue Constructor
   *
   * Creates a priority queue with a maximum size of 100 packets by default
   */
  PriorityQueue ();

  virtual ~PriorityQueue ();

  /**
   * \brief PriorityQueue Constructor
   *
   * Initializes priority queue
   */
  void Initialize ();

  /**
   * Attach a queue to hold data packets to the PriorityQueue.
   *
   * The PriorityQueue "owns" a sub queue that implements a queueing
   * method such as DropTailQueue, DropHeadQueue or RedQueue
   *
   * \param queue Ptr to the new queue.
   */
  void SetDataQueue (Ptr<Queue<Item>> q);

  /**
   * Attach a queue to hold control packets to the PriorityQueue.
   *
   * The PriorityQueue "owns" a sub queue that implements a queueing
   * method such as DropTailQueue, DropHeadQueue or RedQueue
   *
   * \param queue Ptr to the new queue.
   */
  void SetControlQueue (Ptr<Queue<Item>> q);

  /**
   * Get a copy of the attached Queue that holds control packets.
   *
   * \returns Ptr to the queue.
   */
  Ptr<Queue<Item>> GetControlQueue (void) const;

  /**
   * Get a copy of the attached Queue that holds data packets.
   *
   * \returns Ptr to the queue.
   */
  Ptr<Queue<Item>> GetDataQueue (void) const;

  /**
   * \brief Enumeration of the modes supported in the class.
   *
   */
  enum PacketClass
  {
    PACKET_CLASS_CONTROL,     /**< Packet classifier matched packet to control type */
    PACKET_CLASS_DATA,        /**< Packet classifier matched packet to control type */
  };

  virtual bool Enqueue (Ptr<Item> item);
  virtual Ptr<Item> Dequeue (void);
  virtual Ptr<Item> Remove (void);
  virtual Ptr<const Item> Peek (void) const;

private:

  PacketClass Classify (Ptr<const Packet> p);

  Ptr<Queue<Item>> m_controlQueue;         //!< queue for control traffic
  Ptr<Queue<Item>> m_dataQueue;            //!< queue for data traffic

  std::string m_classifier;          //!< classfier for control packets
  pcap_t * m_pcapHandle;             //!< handle for libpcap
  struct bpf_program m_bpf;          //!< compiled classifier for control packets

  NS_LOG_TEMPLATE_DECLARE;           //!< redefinition of the log component
};

/**
 * Implementation of the templates declared above.
 */

template <typename Item>
TypeId
PriorityQueue<Item>::GetTypeId (void)
{
  static TypeId tid = TypeId (("ns3::PriorityQueue" + GetTypeParamName<PriorityQueue<Item> > () + ">").c_str ())
    .SetParent<Queue<Item>> ()
    .template AddConstructor<PriorityQueue<Item> > ()
    .AddAttribute ("ControlQueue",
                   "A queue to use as the transmit queue in the device.",
                   PointerValue (),
                   MakePointerAccessor (&PriorityQueue::m_controlQueue),
                   MakePointerChecker<Queue<Item>> ())
    .AddAttribute ("DataQueue",
                   "A queue to use as the transmit queue in the device.",
                   PointerValue (),
                   MakePointerAccessor (&PriorityQueue::m_dataQueue),
                   MakePointerChecker<Queue<Item>> ())
    .AddAttribute ("ControlPacketClassifier",
                   "Pcap style filter to classify control packets",
                   StringValue (),
                   MakeStringAccessor (&PriorityQueue::m_classifier),
                   MakeStringChecker ())
  ;
  return tid;
}

template <typename Item>
PriorityQueue<Item>::PriorityQueue () :
  Queue<Item> (),
  NS_LOG_TEMPLATE_DEFINE ("PriorityQueue")
{
  NS_LOG_FUNCTION (this);

  m_pcapHandle = pcap_open_dead (DLT_EN10MB, 1500);
  NS_ASSERT_MSG (m_pcapHandle, "failed to open pcap handle");
}

template <typename Item>
PriorityQueue<Item>::~PriorityQueue ()
{
  NS_LOG_FUNCTION (this);

  m_controlQueue = 0;
  m_dataQueue = 0;

  pcap_close (m_pcapHandle);
}

template <typename Item>
void
PriorityQueue<Item>::Initialize ()
{
  NS_LOG_FUNCTION (this);

  int ret = pcap_compile (m_pcapHandle, &m_bpf,
                          m_classifier.c_str (), 1, PCAP_NETMASK_UNKNOWN);
  NS_ASSERT_MSG (ret == 0, "failed to compile control packet classifer");
}

template <typename Item>
void
PriorityQueue<Item>::SetControlQueue (Ptr<Queue<Item>> q)
{
  NS_LOG_FUNCTION (this << q);
  m_controlQueue = q;
}

template <typename Item>
void
PriorityQueue<Item>::SetDataQueue (Ptr<Queue<Item>> q)
{
  NS_LOG_FUNCTION (this << q);
  m_dataQueue = q;
}

template <typename Item>
Ptr<Queue<Item>>
PriorityQueue<Item>::GetControlQueue (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_controlQueue;
}

template <typename Item>
Ptr<Queue<Item>>
PriorityQueue<Item>::GetDataQueue (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_dataQueue;
}

template <typename Item>
typename PriorityQueue<Item>::PacketClass
PriorityQueue<Item>::Classify (Ptr<const Packet> p)
{
  pcap_pkthdr pcapPkthdr;
  pcapPkthdr.caplen = p->GetSize ();
  pcapPkthdr.len = p->GetSize ();
  uint8_t *data = new uint8_t[p->GetSize ()];
  p->CopyData (data, p->GetSize ());
  int ret = pcap_offline_filter (&m_bpf, &pcapPkthdr, data);
  delete [] data;

  if (ret == 0)
    {
      NS_LOG_DEBUG ("Packet is data packet");
      return PACKET_CLASS_DATA;
    }
  else
    {
      NS_LOG_DEBUG ("Packet is control packet");
      return PACKET_CLASS_CONTROL;
    }
}

template <typename Item>
bool
PriorityQueue<Item>::Enqueue (Ptr<Item> item)
{
  NS_LOG_FUNCTION (this << item);

  Ptr<Packet> p= DynamicCast<Packet> (item);
  NS_ABORT_MSG_UNLESS (p, "Class defined for Packet type only");

  PacketClass packetClass = Classify (p);

  if (packetClass == PACKET_CLASS_CONTROL)
    {
      return m_controlQueue->Enqueue (item);
    }
  else
    {
      return m_dataQueue->Enqueue (item);
    }
}

template <typename Item>
Ptr<Item>
PriorityQueue<Item>::Dequeue (void)
{
  NS_LOG_FUNCTION (this);

  Ptr<Item> item = 0;
  if (!m_controlQueue->IsEmpty ())
    {
      item = m_controlQueue->Dequeue ();
    }
  else
    {
      item = m_dataQueue->Dequeue ();
    }

  return item;
}

template <typename Item>
Ptr<Item>
PriorityQueue<Item>::Remove (void)
{
  NS_LOG_FUNCTION (this);

  Ptr<Item> item = 0;
  if (!m_controlQueue->IsEmpty ())
    {
      item = m_controlQueue->Remove ();
    }
  else
    {
      item = m_dataQueue->Remove ();
    }

  return item;
}

template <typename Item>
Ptr<const Item>
PriorityQueue<Item>::Peek (void) const
{
  NS_LOG_FUNCTION (this);

  Ptr<const Item> item = 0;
  if (!m_controlQueue->IsEmpty ())
    {
      item = m_controlQueue->Peek ();
    }
  else
    {
      item = m_dataQueue->Peek ();
    }

  return item;
}

} // namespace ns3

#endif
