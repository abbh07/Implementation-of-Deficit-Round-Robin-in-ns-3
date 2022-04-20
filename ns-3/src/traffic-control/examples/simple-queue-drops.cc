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

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("FirstScriptExample");

int
main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);

  // Enable logs
  LogComponentEnableAll (LOG_PREFIX_TIME);
  LogComponentEnable ("FirstScriptExample", LOG_INFO);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  Time::SetResolution (Time::NS);

  // Create nodes
  NodeContainer nodes;
  nodes.Create (3);
  NodeContainer n0n1 = NodeContainer (nodes.Get (0), nodes.Get (1));
  NodeContainer n1n2 = NodeContainer (nodes.Get (1), nodes.Get (2));

//  TrafficControlHelper tch;
//  uint16_t handle = tch.SetRootQueueDisc ("ns3::PfifoFastQueueDisc");
//  tch.AddInternalQueues (handle, 3, "ns3::DropTailQueue", "MaxSize", StringValue ("10000p"));

//  QueueDiscContainer queueDiscs;

  // Setup channel
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));
//  pointToPoint.SetQueue ("ns3::DropTailQueue");

  // Setup NIC
  NetDeviceContainer devn0n1 = pointToPoint.Install (n0n1);
//  pointToPoint.SetQueue ("ns3::DropTailQueue");
  NetDeviceContainer devn1n2 = pointToPoint.Install (n1n2);
//  tch.Install (devn1n2);
//  queueDiscs = tch.Install (devn1n2);

  // Setup IP
  InternetStackHelper stack;
  stack.Install (nodes);

  Ipv4AddressHelper ipv4;

  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i0i1 = ipv4.Assign (devn0n1);

  ipv4.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer i1i2 = ipv4.Assign (devn1n2);

  // Set up the routing
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();


  // Setup sink application
  uint16_t port = 50000;
  Address sinkLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper sinkHelper ("ns3::UdpSocketFactory", sinkLocalAddress);
  ApplicationContainer sinkApp = sinkHelper.Install (n1n2.Get (1));
  sinkApp.Start (Seconds (0.0));
  sinkApp.Stop (Seconds (10.0));

//  UdpEchoServerHelper echoServer (9);
//
//  ApplicationContainer serverApps = echoServer.Install (nodes.Get (1));
//  serverApps.Start (Seconds (1.0));
//  serverApps.Stop (Seconds (10.0));
//
//  UdpEchoClientHelper echoClient (interfaces.GetAddress (1), 9);
//  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
//  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
//  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));
//
//  ApplicationContainer clientApps = echoClient.Install (nodes.Get (0));
//  clientApps.Start (Seconds (2.0));
//  clientApps.Stop (Seconds (10.0));

  // Setup source application
  UdpClientHelper echoClient (i1i2.GetAddress (1), port);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (100));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (0.2)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApp = echoClient.Install (n1n2.Get (1));
  AddressValue remoteAddress (InetSocketAddress (i1i2.GetAddress (1), port));
//  echoClient.SetAttribute ("Remote", remoteAddress);
  clientApp.Start (Seconds (2.0));
  clientApp.Stop (Seconds (10.0));

//  OnOffHelper clientHelper ("ns3::UdpSocketFactory", Address ());
//  clientHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
//  clientHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
//  clientHelper.SetAttribute ("PacketSize", UintegerValue (1000));
//  clientHelper.SetAttribute ("DataRate", DataRateValue (DataRate ("10Mb/s")));
//
//  ApplicationContainer clientApp;
//  AddressValue remoteAddress (InetSocketAddress (i0i1.GetAddress (1), port));
//  clientHelper.SetAttribute ("Remote", remoteAddress);
//  clientApp.Add (clientHelper.Install (n0n1.Get (0)));
//  clientApp.Start (Seconds (1.0));
//  clientApp.Stop (Seconds (10.0));

  // Setup monitors
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll();

//  QueueDisc::Stats st = queueDiscs.Get (0)->GetStats ();
  Simulator::Stop (Seconds (10.0));
  Simulator::Run ();

//  std::string pathOut;
//  pathOut = "."; // Current directory
//  std::stringstream stmp;
//  stmp << pathOut << "/drr.flowmon";
//  flowmon->SerializeToXmlFile (stmp.str ().c_str (), false, false);

//  std::cout << "*** DRR stats from Node 2 queue ***" << std::endl;
//  std::cout << "\t " << st.GetNDroppedPackets (DRRQueueDisc::UNCLASSIFIED_DROP)
//            << " drops because packet could not be classified by any filter" << std::endl;



  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
  std::cout << std::endl << "*** Flow monitor statistics ***" << std::endl;
  std::cout << "  Tx Packets/Bytes:   " << stats[1].txPackets
            << " / " << stats[1].txBytes << std::endl;
  std::cout << "  Offered Load: " << stats[1].txBytes * 8.0 / (stats[1].timeLastTxPacket.GetSeconds () - stats[1].timeFirstTxPacket.GetSeconds ()) / 1000000 << " Mbps" << std::endl;
  std::cout << "  Rx Packets/Bytes:   " << stats[1].rxPackets
            << " / " << stats[1].rxBytes << std::endl;

//  uint32_t packetsDroppedByQueueDisc = 0;
//  uint64_t bytesDroppedByQueueDisc = 0;
//  if (stats[1].packetsDropped.size () > Ipv4FlowProbe::DROP_QUEUE_DISC)
//    {
//      packetsDroppedByQueueDisc = stats[1].packetsDropped[Ipv4FlowProbe::DROP_QUEUE_DISC];
//      bytesDroppedByQueueDisc = stats[1].bytesDropped[Ipv4FlowProbe::DROP_QUEUE_DISC];
//    }
//  std::cout << "  Packets/Bytes Dropped by Queue Disc:   " << packetsDroppedByQueueDisc
//            << " / " << bytesDroppedByQueueDisc << std::endl;


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

  std::cout << std::endl << "*** Tx Application statistics ***" << std::endl;
  double thr1 = 0;
  uint64_t totalPacketsThr1 = DynamicCast<UdpClient> (clientApp.Get (0))->GetTotalTx ();
  thr1 = totalPacketsThr1 * 8 / (simulationTime * 1000000.0); //Mbit/s
  std::cout << "  Tx Bytes: " << totalPacketsThr1 << std::endl;
  std::cout << "  Average Goodput: " << thr1 << " Mbit/s" << std::endl;

//  std::cout << std::endl << "*** TC Layer statistics ***" << std::endl;
//  std::cout << q->GetStats () << std::endl;

//  Simulator::Destroy ();
  return 0;
}
