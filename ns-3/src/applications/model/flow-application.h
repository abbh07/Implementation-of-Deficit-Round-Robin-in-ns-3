//
// Created by Vinayak Agarwal on 4/26/22.
//

#ifndef NS_3_FLOW_APPLICATION_H
#define NS_3_FLOW_APPLICATION_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/socket.h"
#include "ns3/ipv4-address.h"
#include "ns3/flow-tag.h"

namespace ns3 {

class FlowApplication : public Application
{
public:
  static TypeId GetTypeId (void);

  FlowApplication ();

  virtual ~FlowApplication ();

  void SetRemote (Address ip, uint16_t port);
  void SetRemote (Address addr);
  void SetFlowType (FlowType flowType);
  void SetPacketSize (uint32_t packetSize);
  void SetMaxPackets (uint32_t maxPackets);
  void SetProtocol (TypeId id);

  uint64_t GetTotalTx () const;

protected:
  virtual void DoDispose (void);

private:

  virtual void StartApplication (void);
  virtual void StopApplication (void);
  /**
   * \brief Handle a Connection Succeed event
   * \param socket the connected socket
   */
  void ConnectionSucceeded (Ptr<Socket> socket);
  /**
   * \brief Handle a Connection Failed event
   * \param socket the not connected socket
   */
  void ConnectionFailed (Ptr<Socket> socket);
  void Send (void);
  void SendBursty (void);
  void SendHeavy (void);
  void SendLight (void);

  FlowType m_flowType;
  uint32_t m_count;
  Time m_interval;
  uint32_t m_size;
  uint32_t m_max_packets;
  TypeId m_tid;          //!< Type of the socket used
  bool m_connected;    //!< True if connected

  uint32_t m_sent;
  uint64_t m_totalTx;
  Ptr<Socket> m_socket;
  Address m_peerAddress;
  uint16_t m_peerPort;
  EventId m_sendEvent;
#ifdef NS3_LOG_ENABLE
  std::string m_peerAddressString;
#endif // NS3_LOG_ENABLE

};

} // namespace ns3

#endif //NS_3_FLOW_APPLICATION_H
