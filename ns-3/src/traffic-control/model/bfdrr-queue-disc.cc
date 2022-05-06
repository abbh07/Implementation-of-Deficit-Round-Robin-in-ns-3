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

#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/queue.h"
#include "ns3/bfdrr-queue-disc.h"
#include "ns3/net-device-queue-interface.h"
#include "ns3/ipv4-packet-filter.h"
#include <algorithm>
#include <list>
#include "ns3/bfdrrflow.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("BFDRRQueueDisc");

NS_OBJECT_ENSURE_REGISTERED (BFDRRQueueDisc);


TypeId
BFDRRQueueDisc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::BFDRRQueueDisc")
                          .SetParent<QueueDisc> ()
                          .SetGroupName ("TrafficControl")
                          .AddConstructor<BFDRRQueueDisc> ()
                          .AddAttribute ("SoftLimit",
                                         "The maximum number of packets accepted by this queue disc.",
                                         QueueSizeValue (QueueSize ("80p")),
                                         MakeQueueSizeAccessor (&BFDRRQueueDisc::m_soft_limit),
                                         MakeQueueSizeChecker ())
                          .AddAttribute ("HardLimit",
                                         "The maximum number of packets accepted by this queue disc.",
                                         QueueSizeValue (QueueSize ("100p")),
                                         MakeQueueSizeAccessor (&BFDRRQueueDisc::m_hard_limit),
                                         MakeQueueSizeChecker ())
                          .AddAttribute ("Flows",
                                         "The number of queues into which the incoming packets are classified",
                                         UintegerValue (1024),
                                         MakeUintegerAccessor (&BFDRRQueueDisc::m_flows),
                                         MakeUintegerChecker<uint32_t> ())
      ;
  return tid;
}

BFDRRQueueDisc::BFDRRQueueDisc ()
    : m_quantum (0)
{
  NS_LOG_FUNCTION (this);
  m_flowFactory.SetTypeId ("ns3::DropTailQueue<QueueDiscItem>");
  // Change max size to soft or hard limit based on type of flow
  m_flowFactory.Set ("MaxSize", QueueSizeValue (m_hard_limit));
  AddPacketFilter(CreateObject<BFDRRIpv4PacketFilter>());
}

BFDRRQueueDisc::~BFDRRQueueDisc ()
{
  NS_LOG_FUNCTION (this);
}

void
BFDRRQueueDisc::SetQuantum (uint32_t quantum)
{
  NS_LOG_FUNCTION (this << quantum);
  m_quantum = quantum;
}

uint32_t
BFDRRQueueDisc::GetQuantum (void) const
{
  return m_quantum;
}

bool
BFDRRQueueDisc::DoEnqueue (Ptr<QueueDiscItem> item)
{
  NS_LOG_FUNCTION (this << item);

  int32_t ret = Classify (item);
  uint32_t h;

  // Assuming the packet filter generates a hash that is used to decide which flow the packet
  // belongs to. If this doesn't work, create a map of hash to flow and use that directly.
  if (ret == PacketFilter::PF_NO_MATCH)
    {
      NS_LOG_WARN ("No filter has been able to classify this packet.");
      h = m_flows; // place all unfiltered packets into a separate flow queue
    }

  else
    {
      h = ret % m_flows;
    }

  // Add check for more than m_flows but that isn't necessary rn.

  Ptr<Queue<QueueDiscItem> > flowQ;
  Flow *flow;
  if (m_flowsIndices.find (h) == m_flowsIndices.end ())
    {
      NS_LOG_INFO ("Creating a new flow queue with index " << h);
      flowQ = m_flowFactory.Create<DropTailQueue<QueueDiscItem>> ();

      flow = new Flow();
      // Set flow type from packet tag
      Ptr<Packet> packet = item->GetPacket();
      FlowTag packetTag;
      packet->PeekPacketTag (packetTag);
      flow->SetFlowType(packetTag.GetFlowType());
      flow->idx = GetNInternalQueues () - 1;
      flow->selfQ = flowQ;

      NS_LOG_INFO ("max q size " << flowQ->GetMaxSize());

      m_flowsIndices[h] = flow;
      AddInternalQueue (flowQ);
    }
  else
    {
      flow = m_flowsIndices[h];
      flowQ = m_flowsIndices[h]->selfQ;
    }

  NS_LOG_INFO ("current q size " << flowQ->GetNPackets());

  // Get limit for the type of flow
  QueueSize limit = m_soft_limit;
  if (flow->GetFlowType ()== FlowType::BURSTY)
    {
      limit = m_hard_limit;
    }

  if ((flowQ->GetCurrentSize() + item) > limit)
    {
      NS_LOG_LOGIC ("Queue full -- dropping pkt");
      DropBeforeEnqueue (item, LIMIT_EXCEEDED_DROP);
      return false;
    }

  // Indicates a bursty flow is overflowing its bursty limit
  // fix cond what if there already
  if (flowQ->GetCurrentSize() + item > m_soft_limit)
    {
      NS_LOG_LOGIC ("Overflowing bursty flow detected. Increasing the limit temporarily");
      m_overflowingBurstyFlows.push_back(flow);
    }

  flowQ->Enqueue (item);

  NS_LOG_DEBUG ("Packet enqueued into flow " << h << "; flow index " << m_flowsIndices[h]->idx);

  // are inactive queues in the internal queue?
  if (flow->GetStatus () == FlowStatus::INACTIVE)
    {
      NS_LOG_DEBUG ("Setting flow as ACTIVE");
      flow->SetStatus (FlowStatus::ACTIVE);
      m_flowList.push_back (flow);
    }

  return true;
}

Ptr<QueueDiscItem>
BFDRRQueueDisc::DoDequeue (void)
{
  NS_LOG_FUNCTION (this);

  Ptr<Queue<QueueDiscItem> > flowQ;
  Ptr<QueueDiscItem> item;

  if (m_flowList.empty ())
    {
      NS_LOG_DEBUG ("No active flows found");
      return 0;
    }

  Flow *flow;
  do
    {
      flow = m_flowList.front ();
      flowQ = flow->selfQ;
      m_flowList.pop_front ();
      flow->IncreaseDeficit (m_quantum);
      Ptr<const QueueDiscItem> t_item = flowQ->Peek ();

      if ( (uint32_t) flow->GetDeficit () >= t_item->GetSize ())
        {
          item = flowQ->Dequeue ();
          flow->IncreaseDeficit (-item->GetSize ());
          NS_LOG_DEBUG ("Dequeued packet " << item->GetPacket ());

          if (flowQ->GetNPackets () == 0)
            {
              NS_LOG_DEBUG ("Empty Flow, Setting it to INACTIVE");
              flow->SetDeficit (0);
              flow->SetStatus (FlowStatus::INACTIVE);
            }

          else
            {
              NS_LOG_DEBUG ("Flow still active, pushing back to active list");
              m_flowList.push_back (flow);
            }

          // If this was an overflowing bursty flow, and now it is under soft limit, remove it from the list
          if (std::find(m_overflowingBurstyFlows.begin(), m_overflowingBurstyFlows.end(), flow) != m_overflowingBurstyFlows.end () &&
              (flow->selfQ->GetCurrentSize()) <= m_soft_limit)
            {
              m_overflowingBurstyFlows.remove (flow);
//              m_overflowingBurstyFlows.erase (std::remove (m_overflowingBurstyFlows.begin (),
//                                                           m_overflowingBurstyFlows.end (), flow),
//                                              m_overflowingBurstyFlows.end ());
            }

          return item;
        }                   //End if(flow->GetDeficit ...)

      else
        {
          NS_LOG_DEBUG ("Packet size greater than deficit, pushing flow back to end of list");
          m_flowList.push_back (flow);
          item = 0;
        }
    } while (item == 0);

  return 0; //never reached
}

Ptr<const QueueDiscItem>
BFDRRQueueDisc::DoPeek (void)
{
  NS_LOG_FUNCTION (this);

  Flow *flow;

  if (!m_flowList.empty ())
    {
      flow = m_flowList.front ();
    }
  else
    {
      return 0;
    }

  return flow->selfQ->Peek ();
}

bool
BFDRRQueueDisc::CheckConfig (void)
{
  NS_LOG_FUNCTION (this);
  if (GetNQueueDiscClasses () > 0)
    {
      NS_LOG_ERROR ("BFDRRQueueDisc cannot have classes");
      return false;
    }

  if (GetNPacketFilters () == 0)
    {
      Ptr<BFDRRIpv4PacketFilter> ipv4Filter = CreateObject<BFDRRIpv4PacketFilter> ();
      AddPacketFilter (ipv4Filter);
      //      NS_LOG_ERROR ("BFDRRQueueDisc needs at least a packet filter");
      //      return false;
    }

  if (GetNInternalQueues () > 0)
    {
      NS_LOG_ERROR ("BFDRRQueueDisc cannot have internal queues");
      return false;
    }

  return true;
}

void
BFDRRQueueDisc::InitializeParams (void)
{
  NS_LOG_FUNCTION (this);

  // we are at initialization time. If the user has not set a quantum value,
  // set the quantum to the MTU of the device
  if (!m_quantum)
    {
      //Ptr<NetDevice> device = GetNetDevice ();
      //NS_ASSERT_MSG (device, "Device not set for the queue disc");
      //m_quantum = device->GetMtu ();
      m_quantum = 600;
      NS_LOG_DEBUG ("Setting the quantum to: " << m_quantum);
    }

//  m_flowFactory.SetTypeId ("ns3::BFDRRFlow");

//  m_queueDiscFactory.SetTypeId ("ns3::CoDelQueueDisc");

  //m_queueDiscFactory.Set ("Mode", EnumValue (QueueBase::QUEUE_MODE_BYTES));
  //m_queueDiscFactory.Set ("MaxPackets", UintegerValue (m_soft_limit + 1));
  // m_queueDiscFactory.Set ("Interval", StringValue (m_interval));
  //m_queueDiscFactory.Set ("Target", StringValue (m_target));
}


} // namespace ns3
