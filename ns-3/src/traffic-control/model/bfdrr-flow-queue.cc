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

#include "ns3/bfdrr-flow-queue.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("BFDRRFlow");

NS_OBJECT_ENSURE_REGISTERED (BFDRRFlow);

void
BFDRRFlow::SetDeficit (uint32_t deficit)
{
  NS_LOG_FUNCTION (this << deficit);
  m_deficit = deficit;
}

int32_t
BFDRRFlow::GetDeficit (void) const
{
  NS_LOG_FUNCTION (this);
  return m_deficit;
}

void
BFDRRFlow::IncreaseDeficit (int32_t deficit)
{
  NS_LOG_FUNCTION (this << deficit);
  m_deficit += deficit;
}

void
BFDRRFlow::SetStatus (FlowStatus status)
{
  NS_LOG_FUNCTION (this);
  m_status = status;
}

typename BFDRRFlow::FlowStatus
BFDRRFlow::GetStatus (void) const
{
  NS_LOG_FUNCTION (this);
  return m_status;
}

FlowType
BFDRRFlow::GetFlowType (void) const
{
  NS_LOG_FUNCTION (this);
  return m_flowType;
}

void
BFDRRFlow::SetFlowType (FlowType flowType)
{
  NS_LOG_FUNCTION (this);
  m_flowType = flowType;
}

} // namespace ns3
