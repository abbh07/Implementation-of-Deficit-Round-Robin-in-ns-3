/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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

#ifndef BFDRRFlow_H
#define BFDRRFlow_H

#include "ns3/flow-tag.h"
#include "ns3/queue.h"

namespace ns3 {

/**
 * \ingroup queue
 *
 * \brief A FIFO packet queue that drops tail-end packets on overflow
 */
class BFDRRFlow : public Queue<QueueDiscItem>
{
public:
  /**22
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  /**
   * \brief BFDRRFlow Constructor
   *
   * Creates a droptail queue with a maximum size of 100 packets by default
   */
  BFDRRFlow ();

  virtual ~BFDRRFlow ();

  virtual bool Enqueue (Ptr<QueueDiscItem> item);
  virtual Ptr<QueueDiscItem> Dequeue (void);
  virtual Ptr<QueueDiscItem> Remove (void);
  virtual Ptr<const QueueDiscItem> Peek (void) const;

  /**
   * \enum FlowStatus
   * \brief Used to determine the status of this flow queue
   */
  enum FlowStatus
  {
    INACTIVE,
    ACTIVE
  };

  /**
   * \brief Set the deficit for this flow
   * \param deficit the deficit for this flow
   */
  void SetDeficit (uint32_t deficit);


  /**
   * \brief Get the deficit for this flow
   * \return the deficit for this flow
   */
  int32_t GetDeficit (void) const;


  /**
   * \brief Increase the deficit for this flow
   * \param deficit the amount by which the deficit is to be increased
   */
  void IncreaseDeficit (int32_t deficit);


  /**
   * \brief Set the status for this flow
   * \param status the status for this flow
   */
  void SetStatus (FlowStatus status);


  /**
   * \brief Get the status of this flow
   * \return the status of this flow
   */
  FlowStatus GetStatus (void) const;

  FlowType GetFlowType(void) const;

  void SetFlowType(FlowType flowType);
private:

  uint32_t m_deficit;   //!< the deficit for this flow
  FlowStatus m_status; //!< the status of this flow
  FlowType m_flowType;

  using Queue<QueueDiscItem>::Head;
  using Queue<QueueDiscItem>::Tail;
  using Queue<QueueDiscItem>::DoEnqueue;
  using Queue<QueueDiscItem>::DoDequeue;
  using Queue<QueueDiscItem>::DoRemove;
  using Queue<QueueDiscItem>::DoPeek;

//  NS_LOG_TEMPLATE_DECLARE;     //!< redefinition of the log component
};


/**
 * Implementation of the templates declared above.
 */

TypeId
BFDRRFlow::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::BFDRRFlow")
    .SetParent<Queue<QueueDiscItem> > ()
    .SetGroupName ("Network")
    .AddConstructor<BFDRRFlow > ()
//    .AddAttribute ("MaxSize",
//                   "The max queue size",
//                   QueueSizeValue (QueueSize ("100p")),
//                   MakeQueueSizeAccessor (&QueueBase::SetMaxSize,
//                                          &QueueBase::GetMaxSize),
//                   MakeQueueSizeChecker ())
  ;
  return tid;
}

BFDRRFlow::BFDRRFlow () :
  Queue<QueueDiscItem> ()
//  NS_LOG_TEMPLATE_DEFINE ("BFDRRQueueDisc")
{
  NS_LOG_FUNCTION (this);
}

BFDRRFlow::~BFDRRFlow ()
{
  NS_LOG_FUNCTION (this);
}

bool
BFDRRFlow::Enqueue (Ptr<QueueDiscItem> item)
{
  NS_LOG_FUNCTION (this << item);

  return DoEnqueue (Tail (), item);
}

Ptr<QueueDiscItem>
BFDRRFlow::Dequeue (void)
{
  NS_LOG_FUNCTION (this);

  Ptr<QueueDiscItem> item = DoDequeue (Head ());

  NS_LOG_LOGIC ("Popped " << item);

  return item;
}

Ptr<QueueDiscItem>
BFDRRFlow::Remove (void)
{
  NS_LOG_FUNCTION (this);

  Ptr<QueueDiscItem> item = DoRemove (Head ());

  NS_LOG_LOGIC ("Removed " << item);

  return item;
}

Ptr<const QueueDiscItem>
BFDRRFlow::Peek (void) const
{
  NS_LOG_FUNCTION (this);

  return DoPeek (Head ());
}

} // namespace ns3

#endif /* BFDRRFlow_H */
