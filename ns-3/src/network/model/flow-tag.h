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

#ifndef FLOWTAG_H
#define FLOWTAG_H

#include "ns3/tag.h"
#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include <iostream>
#include <ns3/enum.h>

namespace ns3
{

  enum FlowType { HEAVY = 0, LIGHT, BURSTY };

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
    void SetFlowType (FlowType value);
    /**
   * Get the tag value
   * \return the tag value.
   */
    FlowType GetFlowType (void) const;

  private:
    FlowType m_flowType; //!< tag value
  };
}

#endif /* FLOWTAG_H */