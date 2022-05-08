/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/traffic-control-helper.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/global-router-interface.h"
#include "ns3/ipv4-flow-probe.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/bfdrr-queue-disc.h"
#include "ns3/log.h"
#include <vector>
#include <stdlib.h>


using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("FirstScriptExample");


int
main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);

  // Enable logs
  LogComponentEnableAll (LOG_PREFIX_TIME);
  LogComponentEnable ("DRRQueueDisc", LOG_DEBUG);
  LogComponentEnable ("DRRQueueDisc", LOG_INFO);
  LogComponentEnable ("BFDRRQueueDisc", LOG_DEBUG);
  LogComponentEnable ("BFDRRQueueDisc", LOG_INFO);
//  LogComponentEnable ("BFDRRFlow", LOG_INFO);
  LogComponentEnable ("Ipv4PacketFilter", LOG_DEBUG);
  LogComponentEnable ("Ipv4PacketFilter", LOG_INFO);
  LogComponentEnable ("FlowTag", LOG_DEBUG);
  LogComponentEnable ("FlowTag", LOG_INFO);
  LogComponentEnable ("FirstScriptExample", LOG_DEBUG);
  LogComponentEnable ("FirstScriptExample", LOG_INFO);
  LogComponentEnable ("QueueDisc", LOG_DEBUG);
  LogComponentEnable ("QueueDisc", LOG_INFO);
//  LogComponentEnable ("QueueDisc", LOG_FUNCTION);
  LogComponentEnable ("FlowApplication", LOG_DEBUG);
  LogComponentEnable ("FlowApplication", LOG_INFO);
//  LogComponentEnable ("PfifoFastQueueDisc", LOG_LEVEL_INFO);
//  LogComponentEnable ("DropTailQueue", LOG_LEVEL_INFO);



  Time::SetResolution (Time::NS);

  // Create nodes
  NodeContainer nodes;
//   nodes.Create (3);
//   NodeContainer n0n1 = NodeContainer (nodes.Get (0), nodes.Get (1));
//   NodeContainer n1n2 = NodeContainer (nodes.Get (1), nodes.Get (2));

  //changed
  nodes.Create (7);
  NodeContainer n0n5 = NodeContainer (nodes.Get (0), nodes.Get (5));
  NodeContainer n1n5 = NodeContainer (nodes.Get (1), nodes.Get (5));
  NodeContainer n2n5 = NodeContainer (nodes.Get (2), nodes.Get (5));
  NodeContainer n3n5 = NodeContainer (nodes.Get (3), nodes.Get (5));
  NodeContainer n4n5 = NodeContainer (nodes.Get (4), nodes.Get (5));
  NodeContainer n5n6 = NodeContainer (nodes.Get (5), nodes.Get (6));

  TrafficControlHelper tch;
  tch.SetRootQueueDisc ("ns3::BFDRRQueueDisc");
  // Pfifo q have min size 1000 with min 3 internal q
//  tch.AddInternalQueues (handle, 1, "ns3::DropTailQueue", "MaxSize", StringValue ("1000p"));


  // Setup channel
  // Setup NIC
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));
//   NetDeviceContainer devn0n1 = pointToPoint.Install (n0n1);
  NetDeviceContainer devn0n5 = pointToPoint.Install (n0n5);



  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));
  NetDeviceContainer devn1n5 = pointToPoint.Install (n1n5);

  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));
  NetDeviceContainer devn2n5 = pointToPoint.Install (n2n5);

  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));
  NetDeviceContainer devn3n5 = pointToPoint.Install (n3n5);

  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));
  NetDeviceContainer devn4n5 = pointToPoint.Install (n4n5);



  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("1Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("1ms"));
//   NetDeviceContainer devn1n2 = pointToPoint.Install (n1n2);
  NetDeviceContainer devn5n6 = pointToPoint.Install (n5n6);

  // Setup IP
  InternetStackHelper stack;
  stack.Install (nodes);

  // Should be placed after internet is installed on the nodes
  QueueDiscContainer queueDiscs;
//   queueDiscs = tch.Install(devn1n2);
  queueDiscs = tch.Install(devn5n6);

  Ipv4AddressHelper ipv4;

  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
//   Ipv4InterfaceContainer i0i1 = ipv4.Assign (devn0n1);
  Ipv4InterfaceContainer i0i5 = ipv4.Assign (devn0n5);

  ipv4.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer i1i5 = ipv4.Assign (devn1n5);

  ipv4.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer i2i5 = ipv4.Assign (devn2n5);

  ipv4.SetBase ("10.1.4.0", "255.255.255.0");
  Ipv4InterfaceContainer i3i5 = ipv4.Assign (devn3n5);

  ipv4.SetBase ("10.1.5.0", "255.255.255.0");
  Ipv4InterfaceContainer i4i5 = ipv4.Assign (devn4n5);

  
  
  ipv4.SetBase ("10.1.6.0", "255.255.255.0");
  Ipv4InterfaceContainer i5i6 = ipv4.Assign (devn5n6);

  // Set up the routing
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();


  // Setup sink application
  uint16_t port = 50000;
  Address sinkLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper sinkHelper ("ns3::UdpSocketFactory", sinkLocalAddress);
  
  //changed
  ApplicationContainer sinkApp = sinkHelper.Install (n5n6.Get (1));
  sinkApp.Start (Seconds (0.0));
  sinkApp.Stop (Seconds (10.0));

  // Setup source application
  // Udp client doesn't work with packet sink
//  UdpClientHelper echoClient (i0i1.GetAddress (1), port);
//  echoClient.SetAttribute ("MaxPackets", UintegerValue (10000));
//  echoClient.SetAttribute ("Interval", TimeValue (Seconds (0.1)));
//  echoClient.SetAttribute ("PacketSize", UintegerValue (10240));
//
//  ApplicationContainer clientApp = echoClient.Install (n0n1.Get (0));
//  AddressValue remoteAddress (InetSocketAddress (i1i2.GetAddress (1), port));
//  echoClient.SetAttribute ("Remote", remoteAddress);
//  clientApp.Start (Seconds (2.0));
//  clientApp.Stop (Seconds (10.0));

  OnOffHelper clientHelper ("ns3::UdpSocketFactory", Address ());
  clientHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  clientHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  clientHelper.SetAttribute ("PacketSize", UintegerValue (100));
  clientHelper.SetAttribute ("DataRate", DataRateValue (DataRate ("10Mb/s")));

  Ptr<FlowApplication> flowApplication1 = CreateObject<FlowApplication> ();
  // flowApplication1->SetFlowType (FlowType::LIGHT);
//   flowApplication->SetRemote (i1i2.GetAddress (1), port);
  flowApplication1->SetRemote (i5i6.GetAddress (1), port);
  flowApplication1->SetPacketSize (100);
  flowApplication1->SetMaxPackets (100);
  flowApplication1->SetProtocol (UdpSocketFactory::GetTypeId());
  //  flowApplication.SetAttribute ("FlowType", "Bursty");
//  flowApplication.SetAttribute ("RemoteAddress", i1i2.GetAddress (1));
//  flowApplication.SetAttribute ("RemotePort", port);

//   n0n1.Get (0)->AddApplication (flowApplication);
  n0n5.Get (0)->AddApplication (flowApplication1);

  Ptr<FlowApplication> flowApplication2 = CreateObject<FlowApplication> ();
  // flowApplication2->SetFlowType (FlowType::BURSTY);
//   flowApplication->SetRemote (i1i2.GetAddress (1), port);
  flowApplication2->SetRemote (i5i6.GetAddress (1), port);
  flowApplication2->SetPacketSize (100);
  flowApplication2->SetMaxPackets (100);
  flowApplication2->SetProtocol (UdpSocketFactory::GetTypeId());

  n1n5.Get (0)->AddApplication (flowApplication2);

  Ptr<FlowApplication> flowApplication3 = CreateObject<FlowApplication> ();
  // flowApplication3->SetFlowType (FlowType::HEAVY);
//   flowApplication->SetRemote (i1i2.GetAddress (1), port);
  flowApplication3->SetRemote (i5i6.GetAddress (1), port);
  flowApplication3->SetPacketSize (100);
  flowApplication3->SetMaxPackets (100);
  flowApplication3->SetProtocol (UdpSocketFactory::GetTypeId());

  n2n5.Get (0)->AddApplication (flowApplication3);

  Ptr<FlowApplication> flowApplication4 = CreateObject<FlowApplication> ();
  // flowApplication4->SetFlowType (FlowType::HEAVY);
//   flowApplication->SetRemote (i1i2.GetAddress (1), port);
  flowApplication4->SetRemote (i5i6.GetAddress (1), port);
  flowApplication4->SetPacketSize (100);
  flowApplication4->SetMaxPackets (100);
  flowApplication4->SetProtocol (UdpSocketFactory::GetTypeId());

  n3n5.Get (0)->AddApplication (flowApplication4);


  Ptr<FlowApplication> flowApplication5 = CreateObject<FlowApplication> ();
  // flowApplication5->SetFlowType (FlowType::HEAVY);
//   flowApplication->SetRemote (i1i2.GetAddress (1), port);
  flowApplication5->SetRemote (i5i6.GetAddress (1), port);
  flowApplication5->SetPacketSize (100);
  flowApplication5->SetMaxPackets (100);
  flowApplication5->SetProtocol (UdpSocketFactory::GetTypeId());

  n4n5.Get (0)->AddApplication (flowApplication5);

  vector<FlowType> applicationTypes{
  FlowType::BURSTY,
  FlowType::HEAVY,
  FlowType::LIGHT
  };

  Ptr<NormalRandomVariable> rand = CreateObject<NormalRandomVariable> ();
  
  uint32_t randApplicationIndex = rand->GetInteger(static_cast<uint32_t>(3/2), static_cast<uint32_t>(3/2), 3);
  flowApplication1->SetFlowType (applicationTypes[randApplicationIndex]);
  NS_LOG_INFO("FlowApplication1 Type: " << randApplicationIndex);

  randApplicationIndex = rand->GetInteger(static_cast<uint32_t>(3/2), static_cast<uint32_t>(3/2), 3);
  flowApplication2->SetFlowType (applicationTypes[randApplicationIndex]);
  NS_LOG_INFO("FlowApplication2 Type: " << randApplicationIndex);

  randApplicationIndex = rand->GetInteger(static_cast<uint32_t>(3), static_cast<uint32_t>(3), 3);
  flowApplication3->SetFlowType (applicationTypes[randApplicationIndex]);
  NS_LOG_INFO("FlowApplication3 Type: " << randApplicationIndex);

  randApplicationIndex = rand->GetInteger(static_cast<uint32_t>(3/2), static_cast<uint32_t>(3/2), 3);
  flowApplication4->SetFlowType (applicationTypes[randApplicationIndex]);
  NS_LOG_INFO("FlowApplication4 Type: " << randApplicationIndex);

  randApplicationIndex = rand->GetInteger(static_cast<uint32_t>(2/3), static_cast<uint32_t>(2/3), 3);
  flowApplication5->SetFlowType (applicationTypes[randApplicationIndex]);
  NS_LOG_INFO("FlowApplication5 Type: " << randApplicationIndex);

  // FlowType flowType = applicationTypes[rand()%3+1];
  // flowApplication1->SetFlowType (flowType);
  // NS_LOG_INFO("FlowApplication1 Type: " << flowType);

  // flowType = applicationTypes[rand()%3+1];
  // flowApplication2->SetFlowType (flowType);
  // NS_LOG_INFO("FlowApplication2 Type: " << flowType);

  // flowType = applicationTypes[rand()%3+1];
  // flowApplication3->SetFlowType (flowType);
  // NS_LOG_INFO("FlowApplication3 Type: " << flowType);

  // flowType = applicationTypes[rand()%3+1];
  // flowApplication4->SetFlowType (flowType);
  // NS_LOG_INFO("FlowApplication4 Type: " << flowType);

  // flowType = applicationTypes[rand()%3+1];
  // flowApplication5->SetFlowType (flowType);
  // NS_LOG_INFO("FlowApplication5 Type: " << flowType);

  ApplicationContainer clientApp;
  clientApp.Add (flowApplication1);
  clientApp.Add (flowApplication2);
  clientApp.Add (flowApplication3);
  clientApp.Add (flowApplication4);
  clientApp.Add (flowApplication5);
  clientApp.Start (Seconds (1.0));
  clientApp.Stop (Seconds (8.0));

//  ApplicationContainer clientApp;
//  AddressValue remoteAddress (InetSocketAddress (i1i2.GetAddress (1), port));
//  clientHelper.SetAttribute ("Remote", remoteAddress);
//  clientApp.Add (clientHelper.Install (n0n1.Get (0)));
//  clientApp.Start (Seconds (1.0));
//  clientApp.Stop (Seconds (8.0));

  Ptr<QueueDisc> q = queueDiscs.Get (0);

  // Setup monitors
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll();

  Simulator::Stop (Seconds (10.0));
  Simulator::Run ();

  // Logging
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

  double simulationTime = 10; //seconds

  std::cout << std::endl << "*** Rx Application statistics ***" << std::endl;
  double thr = 0;
  uint64_t totalPacketsThr = DynamicCast<PacketSink> (sinkApp.Get (0))->GetTotalRx ();
  thr = totalPacketsThr * 8 / (simulationTime * 1000000.0); //Mbit/s
  std::cout << "  Rx Bytes: " << totalPacketsThr << std::endl;
  std::cout << "  Average Goodput: " << thr << " Mbit/s" << std::endl;

//  std::cout << std::endl << "*** Tx Application statistics ***" << std::endl;
//  double thr1 = 0;
//  uint64_t totalPacketsThr1 = DynamicCast<UdpClient> (clientApp.Get (0))->GetTotalTx ();
//  thr1 = totalPacketsThr1 * 8 / (simulationTime * 1000000.0); //Mbit/s
//  std::cout << "  Tx Bytes: " << totalPacketsThr1 << std::endl;
//  std::cout << "  Average Goodput: " << thr1 << " Mbit/s" << std::endl;

  std::cout << std::endl << "*** TC Layer statistics ***" << std::endl;
  std::cout << q->GetStats () << std::endl;

  return 0;
}
