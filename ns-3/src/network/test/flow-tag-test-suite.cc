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

#include "ns3/test.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/flow-tag.h"
#include "ns3/string.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("flow-tag-test-suite");

/**
 * \ingroup network-test
 * \ingroup tests
 *
 * FlowTag unit tests.
 */
class FlowTagTestCase : public TestCase
{
public:
  FlowTagTestCase ();
  virtual void DoRun (void);
};

FlowTagTestCase::FlowTagTestCase ()
  : TestCase ("Sanity check on flow tag")
{
}
void
FlowTagTestCase::DoRun (void)
{
  FlowTag tag;
  tag.SetFlowType (FlowType::LIGHT);
  Ptr<Packet> p = Create<Packet> (100);
  p->AddPacketTag (tag);
  Ptr<Packet> aCopy = p->Copy ();

  FlowTag tagCopy;
  p->PeekPacketTag (tagCopy);
  NS_ASSERT (tagCopy.GetFlowType () == tag.GetFlowType ());


  Ptr<DropTailQueue<Packet> > queue = CreateObject<DropTailQueue<Packet> > ();
  queue->Enqueue (p);

  Ptr<Packet> packet = queue->Dequeue ();
  FlowTag actualTag;
  packet->PeekPacketTag (actualTag);
  NS_TEST_EXPECT_MSG_EQ ((packet != 0), true, "I want to remove the first packet");
  NS_TEST_EXPECT_MSG_EQ (tag.GetFlowType (), actualTag.GetFlowType (), " Expected flow type is different");
}

/**
 * \ingroup network-test
 * \ingroup tests
 *
 * \brief FlowTag TestSuite
 */
class FlowTagTestSuite : public TestSuite
{
public:
  FlowTagTestSuite ()
    : TestSuite ("flow-tag", UNIT)
  {
    AddTestCase (new FlowTagTestCase (), TestCase::QUICK);
  }
};

static FlowTagTestSuite g_flowTagTestSuite; //!< Static variable for test initialization
