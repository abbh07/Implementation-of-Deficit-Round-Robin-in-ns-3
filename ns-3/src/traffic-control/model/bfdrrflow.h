//
// Created by Vinayak Agarwal on 5/6/22.
//

#ifndef NS_3_BFDRRFLOW_H
#define NS_3_BFDRRFLOW_H

#include <list>
#include "ns3/flow-tag.h"
#include "ns3/enum.h"

namespace ns3 {

enum FlowStatus
{
  INACTIVE,
  ACTIVE
};

class Flow {
public:

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
  uint32_t idx;
  Ptr<Queue<QueueDiscItem> > selfQ;
private:
  uint32_t m_deficit;   //!< the deficit for this flow
  FlowStatus m_status; //!< the status of this flow
  FlowType m_flowType;
};



void
Flow::SetDeficit (uint32_t deficit)
{
  m_deficit = deficit;
}

int32_t
Flow::GetDeficit (void) const
{
  return m_deficit;
}

void
Flow::IncreaseDeficit (int32_t deficit)
{
  m_deficit += deficit;
}

void
Flow::SetStatus (FlowStatus status)
{
  m_status = status;
}

FlowStatus
Flow::GetStatus (void) const
{
  return m_status;
}

FlowType
Flow::GetFlowType (void) const
{
  return m_flowType;
}

void
Flow::SetFlowType (FlowType flowType)
{
  m_flowType = flowType;
}

} // namespace ns3

#endif //NS_3_BFDRRFLOW_H
