/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 NITK Surathkal
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
 * Authors: Akhil Udathu <akhilu077@gmail.com>
 *          Kaushik S Kalmady <kaushikskalmady@gmail.com>
 *          Vilas M <vilasnitk19@gmail.com>
*/

#ifndef BFDRR_QUEUE_DISC
#define BFDRR_QUEUE_DISC

#include "ns3/queue-disc.h"
#include "ns3/object-factory.h"
#include "ns3/drop-tail-queue.h"
//#include "ns3/bfdrr-flow-queue.h"
#include <list>
#include <map>
#include "ns3/queue.h"
#include "ns3/bfdrrflow.h"

namespace ns3 {

/**
* \ingroup traffic-control
*
* \brief A BFDRRpacket queue disc
*/
//typedef BFDRRFlow<QueueDiscItem> BFDRRInternalQueue;

class BFDRRQueueDisc : public QueueDisc
{
  static constexpr const char* LIMIT_EXCEEDED_DROP = "Queue disc limit exceeded";  //!< Packet dropped due to queue disc limit exceeded

public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  /**
   * \brief BFDRRQueueDisc constructor
   */
  BFDRRQueueDisc ();

  virtual ~BFDRRQueueDisc ();


  /**
 * \brief Set the quantum value.
 *
 * \param quantum The initial number of bytes each queue gets to dequeue on each round of the scheduling algorithm
 */
  void SetQuantum (uint32_t quantum);


  /**
 * \brief Get the quantum value.
 *
 * \returns The initial number of bytes each queue gets to dequeue on each round of the scheduling algorithm
 */
  uint32_t GetQuantum (void) const;

  // Reasons for dropping packets
  static constexpr const char* UNCLASSIFIED_DROP = "Unclassified drop";  //!< No packet filter able to classify packet
  static constexpr const char* OVERLIMIT_DROP = "Overlimit drop";        //!< Overlimit dropped packets


private:
  virtual bool DoEnqueue (Ptr<QueueDiscItem> item);
  virtual Ptr<QueueDiscItem> DoDequeue (void);
  virtual Ptr<const QueueDiscItem> DoPeek (void);
  virtual bool CheckConfig (void);
  virtual void InitializeParams (void);

  /**
   * \brief Drop a packet from the tail of the queue with the largest current byte count (Packet Stealing)
   * \return the index of the queue with the largest current byte count
   */
  uint32_t BFDRRDrop (void);

  uint32_t m_packets;      //!< cumulative sum of packets across all flows
  QueueSize m_soft_limit;              //!< Maximum number of bytes in the queue disc
  QueueSize m_hard_limit;              //!< Maximum number of bytes in the queue disc
  uint32_t m_quantum;        //!< total number of bytes that a flow can send
  uint32_t m_flows;          //!< Number of flow queues

  std::list<Flow*> m_overflowingBurstyFlows;

  std::list<Flow*> m_flowList;    //!< The list of flows

  std::map<uint32_t, Flow*> m_flowsIndices;    //!< Map with the index of class for each flow

  ObjectFactory m_flowFactory;         //!< Factory to create a new flow
  ObjectFactory m_queueDiscFactory; //!< Factory to create a new queue
};

} // namespace ns3

#endif /* BFDRR_QUEUE_DISC */
