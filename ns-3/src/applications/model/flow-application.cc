//
// Created by Vinayak Agarwal on 4/26/22.
//

#include "flow-application.h"

#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/socket.h"
#include "ns3/flow-tag.h"
#include "ns3/random-variable-stream.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/tcp-socket-factory.h"
#include "ns3/flow-application.h"
#include <cstdlib>
#include <cstdio>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("FlowApplication");
NS_OBJECT_ENSURE_REGISTERED (FlowApplication);

TypeId
FlowApplication::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::FlowApplication")
        .SetParent<Application> ()
        .SetGroupName("Applications")
        .AddConstructor<FlowApplication> ()
        .AddAttribute ("FlowType", "Type of flow", EnumValue (FlowType::BURSTY),
                       MakeEnumAccessor (&FlowApplication::m_flowType),
                       MakeEnumChecker (FlowType::BURSTY, "Bursty", FlowType::HEAVY, "Heavy",
                                        FlowType::LIGHT, "Light"))
        .AddAttribute ("RemoteAddress",
                       "The destination Address of the outbound packets",
                       AddressValue (),
                       MakeAddressAccessor (&FlowApplication::m_peerAddress),
                       MakeAddressChecker ())
        .AddAttribute ("RemotePort", "The destination port of the outbound packets",
                       UintegerValue (100),
                       MakeUintegerAccessor (&FlowApplication::m_peerPort),
                       MakeUintegerChecker<uint16_t> ())
        .AddAttribute ("PacketSize",
                       "Size of packets generated. The minimum packet size is 12 bytes which is the size of the header carrying the sequence number and the time stamp.",
                       UintegerValue (1024),
                       MakeUintegerAccessor (&FlowApplication::m_size),
                       MakeUintegerChecker<uint32_t> (12,65507))
        .AddAttribute ("MaxPackets", "Max # of packets generated.",
                       UintegerValue (25),
                       MakeUintegerAccessor (&FlowApplication::m_max_packets),
                       MakeUintegerChecker<uint32_t> (12,65507))
        .AddAttribute ("Protocol", "The type of protocol to use. This should be a subclass of ns3::SocketFactory",
                       TypeIdValue (UdpSocketFactory::GetTypeId ()),
                       MakeTypeIdAccessor (&FlowApplication::m_tid),
                       // This should check for SocketFactory as a parent
                       MakeTypeIdChecker ())
      ;
  return tid;
}

FlowApplication::FlowApplication ()
{
  NS_LOG_FUNCTION (this);
  m_sent = 0;
  m_totalTx = 0;
  m_socket = 0;
  m_sendEvent = EventId ();
  m_connected = false;
}

FlowApplication::~FlowApplication ()
{
  NS_LOG_FUNCTION (this);
}

void
FlowApplication::SetRemote (Address ip, uint16_t port)
{
  NS_LOG_FUNCTION (this << ip << port);
  m_peerAddress = ip;
  m_peerPort = port;
}

void
FlowApplication::SetRemote (Address addr)
{
  NS_LOG_FUNCTION (this << addr);
  m_peerAddress = addr;
}

void
FlowApplication::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void
FlowApplication::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  if (m_socket == 0)
    {
      m_socket = Socket::CreateSocket (GetNode (), m_tid);
      if (Ipv4Address::IsMatchingType(m_peerAddress) == true)
        {
          if (m_socket->Bind () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
          m_socket->Connect (InetSocketAddress (Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort));
        }
      else if (Ipv6Address::IsMatchingType(m_peerAddress) == true)
        {
          if (m_socket->Bind6 () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
          m_socket->Connect (Inet6SocketAddress (Ipv6Address::ConvertFrom(m_peerAddress), m_peerPort));
        }
      else if (InetSocketAddress::IsMatchingType (m_peerAddress) == true)
        {
          if (m_socket->Bind () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
          m_socket->Connect (m_peerAddress);
        }
      else if (Inet6SocketAddress::IsMatchingType (m_peerAddress) == true)
        {
          if (m_socket->Bind6 () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
          m_socket->Connect (m_peerAddress);
        }
      else
        {
          NS_FATAL_ERROR ("Incompatible address type: " << m_peerAddress);
        }

      m_socket->SetConnectCallback (
          MakeCallback (&FlowApplication::ConnectionSucceeded, this),
          MakeCallback (&FlowApplication::ConnectionFailed, this));
    }

#ifdef NS3_LOG_ENABLE
  std::stringstream peerAddressStringStream;
  if (Ipv4Address::IsMatchingType (m_peerAddress))
    {
      peerAddressStringStream << Ipv4Address::ConvertFrom (m_peerAddress);
    }
  else if (Ipv6Address::IsMatchingType (m_peerAddress))
    {
      peerAddressStringStream << Ipv6Address::ConvertFrom (m_peerAddress);
    }
  else if (InetSocketAddress::IsMatchingType (m_peerAddress))
    {
      peerAddressStringStream << InetSocketAddress::ConvertFrom (m_peerAddress).GetIpv4 ();
    }
  else if (Inet6SocketAddress::IsMatchingType (m_peerAddress))
    {
      peerAddressStringStream << Inet6SocketAddress::ConvertFrom (m_peerAddress).GetIpv6 ();
    }
  m_peerAddressString = peerAddressStringStream.str();
#endif // NS3_LOG_ENABLE

  m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
  m_socket->SetAllowBroadcast (true);
  m_sendEvent = Simulator::Schedule (Seconds (0.0), &FlowApplication::Send, this);
}

void
FlowApplication::StopApplication (void)
{
  NS_LOG_FUNCTION (this);
  Simulator::Cancel (m_sendEvent);
}

void
FlowApplication::Send (void)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_sendEvent.IsExpired ());


  if (m_flowType == FlowType::BURSTY)
    {
      SendBursty ();
    }
  else if (m_flowType == FlowType::HEAVY)
    {
      SendHeavy ();
    }
  else
    {
      SendLight ();
    }
}

void FlowApplication::SendBursty ()
{
  // Create a bursty flow like-> ||||| ||||     ||||||           ||||| |||||||

  Ptr<NormalRandomVariable> rand = CreateObject<NormalRandomVariable> ();
  uint32_t packetsToSend = rand->GetInteger(static_cast<uint32_t>(m_max_packets/2), static_cast<uint32_t>(m_max_packets/2), m_max_packets);

  Time random_offset = MilliSeconds (rand->GetValue(1000, 200));
  NS_LOG_INFO("Sending " << packetsToSend << " packets  with delay of " << random_offset);

  for (uint32_t i = 0; i < packetsToSend; i++)
    {
      Ptr<Packet> pkt = Create<Packet> (m_size-sizeof (FlowType));
      FlowTag tag;
      tag.SetFlowType (FlowType::BURSTY);
      pkt->AddPacketTag (tag);

      if ((m_socket->Send (pkt)) >= 0)
      {
        ++m_sent;
        m_totalTx += pkt->GetSize ();
//#ifdef NS3_LOG_ENABLE
//        NS_LOG_INFO ("TraceDelay TX " << m_size << " bytes to "
//                                  << m_peerAddressString << " Uid: "
//                                  << pkt->GetUid () << " Time: "
//                                  << (Simulator::Now ()).As (Time::S));
//#endif // NS3_LOG_ENABLE
      }
    }

  m_sendEvent = Simulator::Schedule (random_offset, &FlowApplication::Send, this);
}

void FlowApplication::SendHeavy ()
{
  Ptr<ExponentialRandomVariable> rand = CreateObject<ExponentialRandomVariable> ();
  uint32_t packetsToSend = rand->GetInteger(m_max_packets*0.75, m_max_packets);

  Ptr<NormalRandomVariable> normalRand = CreateObject<NormalRandomVariable> ();
  Time random_offset = MilliSeconds (normalRand->GetValue(10,2));

  NS_LOG_INFO("Sending " << packetsToSend << " packets  with delay of " << random_offset);

  for (uint32_t i = 0; i < packetsToSend; i++)
    {
      Ptr<Packet> pkt = Create<Packet> (m_size-sizeof (FlowType));
      FlowTag tag;
      tag.SetFlowType (FlowType::HEAVY);
      pkt->AddPacketTag (tag);

      if ((m_socket->Send (pkt)) >= 0)
        {
          ++m_sent;
          m_totalTx += pkt->GetSize ();
//#ifdef NS3_LOG_ENABLE
//          NS_LOG_INFO ("TraceDelay TX " << m_size << " bytes to "
//                                        << m_peerAddressString << " Uid: "
//                                        << pkt->GetUid () << " Time: "
//                                        << (Simulator::Now ()).As (Time::S));
//#endif // NS3_LOG_ENABLE
        }
    }

  m_sendEvent = Simulator::Schedule (random_offset, &FlowApplication::Send, this);
}

void FlowApplication::SendLight ()
{
  Ptr<ExponentialRandomVariable> rand = CreateObject<ExponentialRandomVariable> ();
  uint32_t packetsToSend = rand->GetInteger(m_max_packets*0.25, m_max_packets);

  Ptr<NormalRandomVariable> normalRand = CreateObject<NormalRandomVariable> ();
  Time random_offset = MilliSeconds (normalRand->GetValue(100,20));

  NS_LOG_INFO("Sending " << packetsToSend << " packets  with delay of " << random_offset);

  for (uint32_t i = 0; i < packetsToSend; i++)
    {
      Ptr<Packet> pkt = Create<Packet> (m_size-sizeof (FlowType));
      FlowTag tag;
      tag.SetFlowType (FlowType::LIGHT);
      pkt->AddPacketTag (tag);

      if ((m_socket->Send (pkt)) >= 0)
        {
          ++m_sent;
          m_totalTx += pkt->GetSize ();
//#ifdef NS3_LOG_ENABLE
//          NS_LOG_INFO ("TraceDelay TX " << m_size << " bytes to "
//                                        << m_peerAddressString << " Uid: "
//                                        << pkt->GetUid () << " Time: "
//                                        << (Simulator::Now ()).As (Time::S));
//#endif // NS3_LOG_ENABLE
        }
    }

  m_sendEvent = Simulator::Schedule (random_offset, &FlowApplication::Send, this);
}

void FlowApplication::ConnectionSucceeded (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  m_connected = true;
}

void FlowApplication::ConnectionFailed (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
}


uint64_t
FlowApplication::GetTotalTx () const
{
  return m_totalTx;
}
void
FlowApplication::SetFlowType (FlowType flowType)
{
  m_flowType = flowType;
}
void
FlowApplication::SetPacketSize (uint32_t packetSize)
{
  m_size = packetSize;
}
void
FlowApplication::SetMaxPackets (uint32_t maxPackets)
{
  m_max_packets = maxPackets;
}

void
FlowApplication::SetProtocol (TypeId id)
{
  m_tid = id;
}

} // namespace ns3