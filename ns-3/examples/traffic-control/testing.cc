/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 Universita' degli Studi di Napoli "Federico II"
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
 * Author: Pasquale Imputato <p.imputato@gmail.com>
 * Author: Stefano Avallone <stefano.avallone@unina.it>
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/netanim-module.h"

#include "ns3/seq-ts-size-frag-header.h"
#include "ns3/bursty-helper.h"
#include "ns3/burst-sink-helper.h"
#include "ns3/netanim-module.h"

// This simple example shows how to use TrafficControlHelper to install a
// QueueDisc on a device.
//
// The default QueueDisc is a pfifo_fast with a capacity of 1000 packets (as in
// Linux). However, in this example, we install a RedQueueDisc with a capacity
// of 10000 packets.
//
// Network topology
//
//       10.1.1.0
// n0 -------------- n1
//    point-to-point
//
// The output will consist of all the traced changes in the length of the RED
// internal queue and in the length of the netdevice queue:
//
//    DevicePacketsInQueue 0 to 1
//    TcPacketsInQueue 7 to 8
//    TcPacketsInQueue 8 to 9
//    DevicePacketsInQueue 1 to 0
//    TcPacketsInQueue 9 to 8
//
// plus some statistics collected at the network layer (by the flow monitor)
// and the application layer. Finally, the number of packets dropped by the
// queuing discipline, the number of packets dropped by the netdevice and
// the number of packets requeued by the queuing discipline are reported.
//
// If the size of the DropTail queue of the netdevice were increased from 1
// to a large number (e.g. 1000), one would observe that the number of dropped
// packets goes to zero, but the latency grows in an uncontrolled manner. This
// is the so-called bufferbloat problem, and illustrates the importance of
// having a small device queue, so that the standing queues build in the traffic
// control layer where they can be managed by advanced queue discs rather than
// in the device layer.

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TrafficControlExample");

double avgQueueDiscSize;
uint32_t checkTimes;
std::stringstream filePlotQueueDisc;
std::stringstream fPlotQueueDiscAvg;

void
CheckQueue (Ptr<QueueDisc> queue)
{

      StaticCast<DRRQueueDisc> (queue)->SetMaxSize(QueueSize(QueueSizeUnit::PACKETS, 50));

  // check queue disc size every 1/100 of a second
  Simulator::Schedule (Seconds (0.001), &CheckQueue, queue);

}

void
CheckQueueDiscSize (Ptr<QueueDisc> queue)
{
  uint32_t qSize = StaticCast<DRRQueueDisc> (queue)->GetNBytes ();

  avgQueueDiscSize += qSize;
  checkTimes++;

  QueueSize limit = StaticCast<DRRQueueDisc> (queue)->GetCurrentSize();
  // StaticCast<DRRQueueDisc> (queue)->GetAttribute ("MaxSize", limit);
  NS_LOG_INFO("Max Queue Size: "<<limit.GetValue());
  if(limit.GetValue() >= 45)
    {
      //StaticCast<DRRQueueDisc> (queue)->SetAttribute("MaxSize", StringValue("1200p"));
      StaticCast<DRRQueueDisc> (queue)->SetMaxSize(QueueSize(QueueSizeUnit::PACKETS, 80));
      NS_LOG_INFO("Queue Size Changed to 80. Cuurent value: "<<limit.GetValue());
    }
  else
    {
      StaticCast<DRRQueueDisc> (queue)->SetMaxSize(QueueSize(QueueSizeUnit::PACKETS, 50));
      NS_LOG_INFO("Queue Size Changed to 50. Current value: "<<limit.GetValue());

    }

  // check queue disc size every 1/100 of a second
  Simulator::Schedule (Seconds (0.01), &CheckQueueDiscSize, queue);

  //  std::ofstream fPlotQueueDisc (filePlotQueueDisc.str ().c_str (), std::ios::out | std::ios::app);
  //  fPlotQueueDisc << Simulator::Now ().GetSeconds () << " " << qSize << std::endl;
  //  fPlotQueueDisc.close ();
  //
  //  std::ofstream fPlotQueueDiscAvg (filePlotQueueDiscAvg.str ().c_str (), std::ios::out | std::ios::app);
  //  fPlotQueueDiscAvg << Simulator::Now ().GetSeconds () << " " << avgQueueDiscSize / checkTimes << std::endl;
  //  fPlotQueueDiscAvg.close ();
}

void
TcPacketsInQueueTrace (uint32_t oldValue, uint32_t newValue)
{
  std::cout << "TcPacketsInQueue " << oldValue << " to " << newValue << std::endl;
}

void
DevicePacketsInQueueTrace (uint32_t oldValue, uint32_t newValue)
{
  std::cout << "DevicePacketsInQueue " << oldValue << " to " << newValue << std::endl;
}

void
SojournTimeTrace (Time sojournTime)
{
  std::cout << "Sojourn time " << sojournTime.ToDouble (Time::MS) << "ms" << std::endl;
}

std::string
AddressToString (const Address &addr)
{
  std::stringstream addressStr;
  addressStr << InetSocketAddress::ConvertFrom (addr).GetIpv4 () << ":"
             << InetSocketAddress::ConvertFrom (addr).GetPort ();
  return addressStr.str ();
}

void
FragmentTx (Ptr<const Packet> fragment, const Address &from, const Address &to,
            const SeqTsSizeFragHeader &header)
{
  NS_LOG_INFO ("Sent fragment " << header.GetFragSeq () << "/" << header.GetFrags ()
                                << " of burst seq=" << header.GetSeq ()
                                << " of header.GetSize ()=" << header.GetSize ()
                                << " (fragment->GetSize ()=" << fragment->GetSize ()
                                << ") bytes from " << AddressToString (from) << " to "
                                << AddressToString (to) << " at " << header.GetTs ().As (Time::S));
}

void
FragmentRx (Ptr<const Packet> fragment, const Address &from, const Address &to,
            const SeqTsSizeFragHeader &header)
{
  NS_LOG_INFO ("Received fragment "
               << header.GetFragSeq () << "/" << header.GetFrags () << " of burst seq="
               << header.GetSeq () << " of header.GetSize ()=" << header.GetSize ()
               << " (fragment->GetSize ()=" << fragment->GetSize () << ") bytes from "
               << AddressToString (from) << " to " << AddressToString (to) << " at "
               << header.GetTs ().As (Time::S));
}

void
BurstTx (Ptr<const Packet> burst, const Address &from, const Address &to,
         const SeqTsSizeFragHeader &header)
{
  NS_LOG_INFO ("Sent burst seq=" << header.GetSeq () << " of header.GetSize ()="
                                 << header.GetSize () << " (burst->GetSize ()=" << burst->GetSize ()
                                 << ") bytes from " << AddressToString (from) << " to "
                                 << AddressToString (to) << " at " << header.GetTs ().As (Time::S));
}

void
BurstRx (Ptr<const Packet> burst, const Address &from, const Address &to,
         const SeqTsSizeFragHeader &header)
{
  NS_LOG_INFO ("Received burst seq="
               << header.GetSeq () << " of header.GetSize ()=" << header.GetSize ()
               << " (burst->GetSize ()=" << burst->GetSize () << ") bytes from "
               << AddressToString (from) << " to " << AddressToString (to) << " at "
               << header.GetTs ().As (Time::S));
}


int
main (int argc, char *argv[])
{
  double simulationTime = 10; //seconds
  std::string transportProt = "Udp";
  std::string socketType;

  Config::SetDefault ("ns3::QueueBase::MaxSize",
                      QueueSizeValue (QueueSize (QueueSizeUnit::PACKETS, 50)));

//  Config::SetDefault ("ns3::DRRQueueDisc::MaxSize",
//                      QueueSizeValue (QueueSize (QueueSizeUnit::PACKETS, 50)));

  LogComponentEnableAll (LOG_PREFIX_TIME);
  LogComponentEnable ("TrafficControlExample", LOG_INFO);

  CommandLine cmd;
  cmd.AddValue ("transportProt", "Transport protocol to use: Tcp, Udp", transportProt);
  cmd.Parse (argc, argv);

  if (transportProt.compare ("Tcp") == 0)
    {
      socketType = "ns3::TcpSocketFactory";
    }
  else
    {
      socketType = "ns3::UdpSocketFactory";
    }

  NodeContainer nodes;
  nodes.Create (2);



  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("1Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("20ms"));
//  pointToPoint.SetQueue ("ns3::DropTailQueue", "MaxSize", StringValue ("5000p"));

  NetDeviceContainer devices;
  devices = pointToPoint.Install (nodes);

  InternetStackHelper stack;
  stack.Install (nodes);

  TrafficControlHelper tch;
  tch.SetRootQueueDisc ("ns3::DRRQueueDisc");
  QueueDiscContainer qdiscs = tch.Install (devices);

  Ptr<QueueDisc> q = qdiscs.Get (1);
  q->TraceConnectWithoutContext ("PacketsInQueue", MakeCallback (&TcPacketsInQueueTrace));
  Config::ConnectWithoutContext ("/NodeList/1/$ns3::TrafficControlLayer/RootQueueDiscList/0/SojournTime",
                                 MakeCallback (&SojournTimeTrace));

  (q)->SetMaxSize(QueueSize(QueueSizeUnit::PACKETS, 50));

//  Ptr<NetDevice> nd = devices.Get (0);
//  Ptr<PointToPointNetDevice> ptpnd = DynamicCast<PointToPointNetDevice> (nd);
//  Ptr<Queue<Packet> > queue = ptpnd->GetQueue ();
//  queue->TraceConnectWithoutContext ("PacketsInQueue", MakeCallback (&DevicePacketsInQueueTrace));

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");

  Ipv4InterfaceContainer interfaces = address.Assign (devices);
  Ipv4Address serverAddress = interfaces.GetAddress (0);

  //Flow
  uint16_t port = 7;
  Address localAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper packetSinkHelper (socketType, localAddress);
  ApplicationContainer sinkApp = packetSinkHelper.Install (nodes.Get (0));

  sinkApp.Start (Seconds (0.0));
  sinkApp.Stop (Seconds (simulationTime + 10));
  //
  //  uint32_t payloadSize = 1448;
  //  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (payloadSize));

  //  OnOffHelper onoff (socketType, Ipv4Address::GetAny ());
  //  onoff.SetAttribute ("OnTime",  StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  //  onoff.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  //  onoff.SetAttribute ("PacketSize", UintegerValue (payloadSize));
  //  onoff.SetAttribute ("DataRate", StringValue ("50Mbps")); //bit/s
  //  ApplicationContainer apps;

  //  InetSocketAddress rmt (interfaces.GetAddress (0), port);
  //  rmt.SetTos (0xb8);
  //  AddressValue remoteAddress (rmt);
  //  onoff.SetAttribute ("Remote", remoteAddress);
  //  apps.Add (onoff.Install (nodes.Get (1)));
  //  apps.Start (Seconds (1.0));
  //  apps.Stop (Seconds (simulationTime + 0.1));



  // Create bursty application helper
  BurstyHelper burstyHelper ("ns3::UdpSocketFactory", InetSocketAddress (serverAddress, port));
  burstyHelper.SetAttribute ("FragmentSize", UintegerValue (1200));
  burstyHelper.SetBurstGenerator ("ns3::SimpleBurstGenerator",
                                  "PeriodRv", StringValue ("ns3::ConstantRandomVariable[Constant=10e-3]"),
                                  "BurstSizeRv", StringValue ("ns3::ConstantRandomVariable[Constant=5e3]"));

  // Install bursty application
  ApplicationContainer serverApps = burstyHelper.Install (nodes.Get (1));
  Ptr<BurstyApplication> burstyApp = serverApps.Get (0)->GetObject<BurstyApplication> ();

  // Example of connecting to the trace sources
  burstyApp->TraceConnectWithoutContext ("FragmentTx", MakeCallback (&FragmentTx));
  burstyApp->TraceConnectWithoutContext ("BurstTx", MakeCallback (&BurstTx));

  // Create burst sink helper
  BurstSinkHelper burstSinkHelper ("ns3::TcpSocketFactory", localAddress);

  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (simulationTime + 15));

  // Install burst sink
  //  ApplicationContainer clientApps = burstSinkHelper.Install (nodes.Get (0));
  //  Ptr<BurstSink> burstSink = clientApps.Get (0)->GetObject<BurstSink> ();
  //
  //  // Example of connecting to the trace sources
  //  burstSink->TraceConnectWithoutContext ("BurstRx", MakeCallback (&BurstRx));
  //  burstSink->TraceConnectWithoutContext ("FragmentRx", MakeCallback (&FragmentRx));
  //
  //  clientApps.Start (Seconds (0.0));
  //  clientApps.Stop (Seconds (simulationTime + 0));
  //
  //  // Stop bursty app after simTimeSec
  //  serverApps.Stop (Seconds (simTimeSec));

  //  Ptr<QueueDisc> queue = qdiscs.Get (0);
  Simulator::ScheduleNow (&CheckQueueDiscSize, q);
//  Simulator::ScheduleNow (&CheckQueue, q);

  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll();

  Simulator::Stop (Seconds (simulationTime + 20));

  AnimationInterface anim("traffic-control.xml");
  anim.AnimationInterface::EnablePacketMetadata(true);

  //  anim.SetConstantPosition (nodes.Get(0), 1.0,0.0);
  //  anim.SetConstantPosition (nodes.Get(1), 2.0 ,3.0);

  Simulator::Run ();

  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
  std::cout << std::endl << "*** Flow monitor statistics ***" << std::endl;
  std::cout << "  Tx Packets/Bytes:   " << stats[1].txPackets
            << " / " << stats[1].txBytes << std::endl;
  std::cout << "  Offered Load: " << stats[1].txBytes * 8.0 / (stats[1].timeLastTxPacket.GetSeconds () - stats[1].timeFirstTxPacket.GetSeconds ()) / 1000000 << " Mbps" << std::endl;
  std::cout << "  Rx Packets/Bytes:   " << stats[1].rxPackets
            << " / " << stats[1].rxBytes << std::endl;
  uint32_t packetsDroppedByQueueDisc = 0;
  uint64_t bytesDroppedByQueueDisc = 0;
  if (stats[1].packetsDropped.size () > Ipv4FlowProbe::DROP_QUEUE_DISC)
    {
      packetsDroppedByQueueDisc = stats[1].packetsDropped[Ipv4FlowProbe::DROP_QUEUE_DISC];
      bytesDroppedByQueueDisc = stats[1].bytesDropped[Ipv4FlowProbe::DROP_QUEUE_DISC];
    }
  std::cout << "  Packets/Bytes Dropped by Queue Disc:   " << packetsDroppedByQueueDisc
            << " / " << bytesDroppedByQueueDisc << std::endl;
  uint32_t packetsDroppedByNetDevice = 0;
  uint64_t bytesDroppedByNetDevice = 0;
  if (stats[1].packetsDropped.size () > Ipv4FlowProbe::DROP_QUEUE)
    {
      packetsDroppedByNetDevice = stats[1].packetsDropped[Ipv4FlowProbe::DROP_QUEUE];
      bytesDroppedByNetDevice = stats[1].bytesDropped[Ipv4FlowProbe::DROP_QUEUE];
    }
  std::cout << "  Packets/Bytes Dropped by NetDevice:   " << packetsDroppedByNetDevice
            << " / " << bytesDroppedByNetDevice << std::endl;
  std::cout << "  Throughput: " << stats[1].rxBytes * 8.0 / (stats[1].timeLastRxPacket.GetSeconds () - stats[1].timeFirstRxPacket.GetSeconds ()) / 1000000 << " Mbps" << std::endl;
  std::cout << "  Mean delay:   " << stats[1].delaySum.GetSeconds () / stats[1].rxPackets << std::endl;
  std::cout << "  Mean jitter:   " << stats[1].jitterSum.GetSeconds () / (stats[1].rxPackets - 1) << std::endl;
  auto dscpVec = classifier->GetDscpCounts (1);
  for (auto p : dscpVec)
    {
      std::cout << "  DSCP value:   0x" << std::hex << static_cast<uint32_t> (p.first) << std::dec
                << "  count:   "<< p.second << std::endl;
    }

  Simulator::Destroy ();

  std::cout << std::endl << "*** Application statistics ***" << std::endl;
  double thr = 0;
  uint64_t totalPacketsThr = DynamicCast<PacketSink> (sinkApp.Get (0))->GetTotalRx ();
  thr = totalPacketsThr * 8 / (simulationTime * 1000000.0); //Mbit/s
  std::cout << "  Rx Bytes: " << totalPacketsThr << std::endl;
  std::cout << "  Average Goodput: " << thr << " Mbit/s" << std::endl;
  std::cout << std::endl << "*** TC Layer statistics ***" << std::endl;
  std::cout << q->GetStats () << std::endl;
  return 0;
}
