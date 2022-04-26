//
// Created by Vinayak Agarwal on 4/26/22.
//

#include "flow-tag.h"

namespace ns3 {
NS_LOG_COMPONENT_DEFINE ("FlowTag");
NS_OBJECT_ENSURE_REGISTERED (FlowTag);

TypeId
FlowTag::GetTypeId (void)
{
  static TypeId tid =
      TypeId ("ns3::FlowTag")
          .SetParent<Tag> ()
          .AddConstructor<FlowTag> ()
          .AddAttribute ("FlowType", "Type of flow", EnumValue (FlowType::BURSTY),
                         MakeEnumAccessor (&FlowTag::m_flowType),
                         MakeEnumChecker (FlowType::BURSTY, "Bursty", FlowType::HEAVY, "Heavy",
                                          FlowType::LIGHT, "Light"));
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
  return sizeof (FlowType);
}
void
FlowTag::Serialize (TagBuffer i) const
{
  i.WriteU8 (m_flowType);
}
void
FlowTag::Deserialize (TagBuffer i)
{
  m_flowType = static_cast<FlowType> (i.ReadU8 ());
}
void
FlowTag::Print (std::ostream &os) const
{
  os << "v=" << (uint32_t) m_flowType;
}
void
FlowTag::SetFlowType (FlowType value)
{
  m_flowType = value;
}
FlowType
FlowTag::GetFlowType (void) const
{
  return m_flowType;
}
} // namespace ns3