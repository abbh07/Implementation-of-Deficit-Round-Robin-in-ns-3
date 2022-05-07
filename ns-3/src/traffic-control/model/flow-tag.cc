//
// Created by Vinayak Agarwal on 4/26/22.
//

#include "flow-tag.h"

TypeId
FlowTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::FlowTag")
                          .SetParent<Tag> ()
                          .AddConstructor<FlowTag> ()
      ;
  return tid;
}

TypeId
FlowTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}
uint32_t
FlowTag::GetSerializedSize (void) const
{
  return 1;
}
void
FlowTag::Serialize (TagBuffer i) const
{
  i.WriteU8 (m_flowType);
}
void
FlowTag::Deserialize (TagBuffer i)
{
  m_flowType = static_cast<FlowType>(i.ReadU8 ());
}
void
FlowTag::Print (std::ostream &os) const
{
  os << "v=" << (uint32_t) m_flowType;
}
void
FlowTag::SetSimpleValue (FlowType value)
{
  m_flowType = value;
}
FlowType
FlowTag::GetSimpleValue (void) const
{
  return m_flowType;
}