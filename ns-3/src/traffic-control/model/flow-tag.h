/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006,2007 INRIA
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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#include "ns3/tag.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include <iostream>

using namespace ns3;

enum FlowType {
  HEAVY = 0,
  LIGHT,
  BURSTY
};

/**
 * \ingroup network
 * A simple example of an Tag implementation
 */
class FlowTag : public Tag
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);
  virtual void Print (std::ostream &os) const;

  // these are our accessors to our tag structure
  /**
   * Set the tag value
   * \param value The tag value.
   */
  void SetSimpleValue (FlowType value);
  /**
   * Get the tag value
   * \return the tag value.
   */
  FlowType GetSimpleValue (void) const;
private:
  FlowType m_flowType;  //!< tag value
};




//int main (int argc, char *argv[])
//{
//  // create a tag.
//  FlowTag tag;
//  tag.SetSimpleValue (0x56);
//
//  // store the tag in a packet.
//  Ptr<Packet> p = Create<Packet> (100);
//  p->AddPacketTag (tag);
//
//  // create a copy of the packet
//  Ptr<Packet> aCopy = p->Copy ();
//
//  // read the tag from the packet copy
//  FlowTag tagCopy;
//  p->PeekPacketTag (tagCopy);
//
//  // the copy and the original are the same !
//  NS_ASSERT (tagCopy.GetSimpleValue () == tag.GetSimpleValue ());
//
//  aCopy->PrintPacketTags (std::cout);
//  std::cout << std::endl;
//
//  return 0;
//}
