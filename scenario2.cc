#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include <iostream>

// Default Network Topology
//
//   Wifi 20.1.3.0
//                 AP
//  *    *    *    *
//  |    |    |    |    20.1.1.0
// n5   n6   n7   n0 -------------- n1  
//                   point-to-point  

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Scenario2_AllWiFi");

int 
main (int argc, char *argv[])
{
  LogComponentEnable("Scenario2_AllWiFi", LOG_LEVEL_INFO);

  uint32_t nNodes = 3;
  double distance = 20.0;

  CommandLine cmd;
  cmd.AddValue ("nNodes", "Number of wifi STA devices", nNodes);
  
  cmd.Parse (argc,argv);

  
/*Link between AP and gateway router */
//Access Network 1
  NS_LOG_INFO ("Creation of WiFi-Access Network 1");
  NodeContainer p2pNodes1;
  p2pNodes1.Create (2);
  PointToPointHelper pointToPoint1;
  pointToPoint1.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint1.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer p2pDevices1;
  p2pDevices1 = pointToPoint1.Install (p2pNodes1);

  NodeContainer wifiStaNodes1;
  wifiStaNodes1.Create (nNodes);
  NodeContainer wifiApNode1 = p2pNodes1.Get (0);
  

  YansWifiChannelHelper channel1 = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy1 = YansWifiPhyHelper::Default ();
  phy1.SetChannel (channel1.Create ());

  WifiHelper wifi1 = WifiHelper::Default ();
  wifi1.SetRemoteStationManager ("ns3::AarfWifiManager");

  QosWifiMacHelper mac1 = QosWifiMacHelper::Default ();

  Ssid ssid1 = Ssid ("ns-3-ssid1");
  mac1.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid1),
               "ActiveProbing", BooleanValue (false));

  NetDeviceContainer staDevices1;
  staDevices1 = wifi1.Install (phy1, mac1, wifiStaNodes1);

  mac1.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid1));

  NetDeviceContainer apDevices1;
  apDevices1 = wifi1.Install (phy1, mac1, wifiApNode1);

  NodeContainer positionedNodes1;
  positionedNodes1.Add(wifiStaNodes1.Get(0));
  positionedNodes1.Add(wifiApNode1.Get(0));
  positionedNodes1.Add(wifiStaNodes1.Get(1));
  positionedNodes1.Add(wifiStaNodes1.Get(2));

  // Install Mobility Model
  Ptr<ListPositionAllocator> positionAlloc1 = CreateObject<ListPositionAllocator> ();
   for (uint16_t i = 0; i < nNodes ; i++)
    {
      positionAlloc1->Add (Vector(2*distance, distance * (i+1), 0));
    }
  positionAlloc1->Add (Vector(distance, distance * 2, 0));
  MobilityHelper mobility1;
  mobility1.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility1.SetPositionAllocator(positionAlloc1);
  mobility1.Install(positionedNodes1);
 
  Ptr<Node> gw1 = p2pNodes1.Get (1);

  InternetStackHelper stackWiFi1;
  stackWiFi1.Install (wifiApNode1);
  stackWiFi1.Install (wifiStaNodes1);
  stackWiFi1.Install (gw1);

  Ipv4AddressHelper address1;
  address1.SetBase ("20.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces1;
  p2pInterfaces1 = address1.Assign (p2pDevices1);

  address1.SetBase ("20.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces_ue1_an1 = address1.Assign (staDevices1);
  address1.Assign (apDevices1);

  pointToPoint1.EnablePcapAll ("scenario2_accessnetwork1");
  phy1.EnablePcap ("scenario2_accessnetwork1", apDevices1.Get (0));

//Access Network 2
  NS_LOG_INFO ("Creation of WiFi-Access Network 2");
  NodeContainer p2pNodes2;
  p2pNodes2.Create (2);
  PointToPointHelper pointToPoint2;
  pointToPoint2.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint2.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer p2pDevices2;
  p2pDevices2 = pointToPoint2.Install (p2pNodes2);

  NodeContainer wifiStaNodes2;
  wifiStaNodes2.Create (nNodes);
  NodeContainer wifiApNode2 = p2pNodes2.Get (0);

  YansWifiChannelHelper channel2 = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy2 = YansWifiPhyHelper::Default ();
  phy2.SetChannel (channel2.Create ());

  WifiHelper wifi2 = WifiHelper::Default ();
  wifi2.SetRemoteStationManager ("ns3::AarfWifiManager");

  QosWifiMacHelper mac2 = QosWifiMacHelper::Default ();

  Ssid ssid2 = Ssid ("ns-3-ssid2");
  mac2.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid2),
               "ActiveProbing", BooleanValue (false));

  NetDeviceContainer staDevices2;
  staDevices2 = wifi2.Install (phy2, mac2, wifiStaNodes2);

  mac2.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid2));

  NetDeviceContainer apDevices2;
  apDevices2 = wifi2.Install (phy2, mac2, wifiApNode2);

  NodeContainer positionedNodes2;
  positionedNodes2.Add(wifiStaNodes2.Get(0));
  positionedNodes2.Add(wifiApNode2.Get(0));
  positionedNodes2.Add(wifiStaNodes2.Get(1));
  positionedNodes2.Add(wifiStaNodes2.Get(2));

  // Install Mobility Model
  Ptr<ListPositionAllocator> positionAlloc2 = CreateObject<ListPositionAllocator> ();
   for (uint16_t i = 0; i < nNodes ; i++)
    {
      positionAlloc2->Add (Vector(2*distance, distance * (i+1+nNodes), 0));
    }
  positionAlloc2->Add (Vector(distance, distance * (2+nNodes), 0));
  MobilityHelper mobility2;
  mobility2.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility2.SetPositionAllocator(positionAlloc2);
  mobility2.Install(positionedNodes2);

  Ptr<Node> gw2 = p2pNodes2.Get (1);

  InternetStackHelper stackWiFi2;
  stackWiFi2.Install (wifiApNode2);
  stackWiFi2.Install (wifiStaNodes2);
  stackWiFi2.Install (gw2);

  Ipv4AddressHelper address2;
  address2.SetBase ("20.2.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces2;
  p2pInterfaces2 = address2.Assign (p2pDevices2);

  address2.SetBase ("20.2.2.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces_ue2_an2 = address2.Assign (staDevices2);
  address2.Assign (apDevices2);

  pointToPoint2.EnablePcapAll ("scenario2_accessnetwork2");
  phy2.EnablePcap ("scenario2_accessnetwork2", apDevices2.Get (0));

//Access Network 3
  NS_LOG_INFO ("Creation of WiFi-Access Network 3");
  NodeContainer p2pNodes3;
  p2pNodes3.Create (2);
  PointToPointHelper pointToPoint3;
  pointToPoint3.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint3.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer p2pDevices3;
  p2pDevices3 = pointToPoint3.Install (p2pNodes3);

  NodeContainer wifiStaNodes3;
  wifiStaNodes3.Create (nNodes);
  NodeContainer wifiApNode3 = p2pNodes3.Get (0);

  YansWifiChannelHelper channel3 = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy3 = YansWifiPhyHelper::Default ();
  phy3.SetChannel (channel3.Create ());

  WifiHelper wifi3 = WifiHelper::Default ();
  wifi3.SetRemoteStationManager ("ns3::AarfWifiManager");

  QosWifiMacHelper mac3 = QosWifiMacHelper::Default ();

  Ssid ssid3 = Ssid ("ns-3-ssid3");
  mac3.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid3),
               "ActiveProbing", BooleanValue (false));

  NetDeviceContainer staDevices3;
  staDevices3 = wifi3.Install (phy3, mac3, wifiStaNodes3);

  mac3.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid3));

  NetDeviceContainer apDevices3;
  apDevices3 = wifi3.Install (phy3, mac3, wifiApNode3);

  NodeContainer positionedNodes3;
  positionedNodes3.Add(wifiStaNodes3.Get(0));
  positionedNodes3.Add(wifiApNode3.Get(0));
  positionedNodes3.Add(wifiStaNodes3.Get(1));
  positionedNodes3.Add(wifiStaNodes3.Get(2));

  // Install Mobility Model
  Ptr<ListPositionAllocator> positionAlloc3 = CreateObject<ListPositionAllocator> ();
   for (uint16_t i = 0; i < nNodes ; i++)
    {
      positionAlloc3->Add (Vector(2*distance, distance * (i+1+(2*nNodes)), 0));
    }
  positionAlloc3->Add (Vector(distance, distance * (2+(2*nNodes)), 0));
  MobilityHelper mobility3;
  mobility3.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility3.SetPositionAllocator(positionAlloc3);
  mobility3.Install(positionedNodes3);

  Ptr<Node> gw3 = p2pNodes3.Get (1);

  InternetStackHelper stackWiFi3;
  stackWiFi3.Install (wifiApNode3);
  stackWiFi3.Install (wifiStaNodes3);
  stackWiFi3.Install (gw3);
 
  Ipv4AddressHelper address3;
  address3.SetBase ("20.3.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces3;
  p2pInterfaces3 = address3.Assign (p2pDevices3);

  address3.SetBase ("20.3.2.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces_ue3_an3 = address3.Assign (staDevices3);
  address3.Assign (apDevices3);

  pointToPoint3.EnablePcapAll ("scenario2_accessnetwork3");
  phy3.EnablePcap ("scenario2_accessnetwork3", apDevices3.Get (0));


 //Rest of network
  NS_LOG_INFO ("Creation of Rest of Network");
  NodeContainer nodes;
  nodes.Create (16);

  
  Ptr<Node> edge2 = nodes.Get(0);
  Ptr<Node> core2 = nodes.Get(1);
  Ptr<Node> core1 = nodes.Get(2);
  Ptr<Node> edge1 = nodes.Get(3);
  Ptr<Node> r1 = nodes.Get(4);
  Ptr<Node> r2 = nodes.Get(5);
  Ptr<Node> r3 = nodes.Get(6);
  Ptr<Node> s11 = nodes.Get(7);
  Ptr<Node> s12 = nodes.Get(8);
  Ptr<Node> s13 = nodes.Get(9);
  Ptr<Node> s21 = nodes.Get(10);
  Ptr<Node> s22 = nodes.Get(11);
  Ptr<Node> s23 = nodes.Get(12);
  Ptr<Node> s31 = nodes.Get(13);
  Ptr<Node> s32 = nodes.Get(14);
  Ptr<Node> s33 = nodes.Get(15);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));


  NetDeviceContainer devices_gw1_e2;
  NetDeviceContainer devices_gw2_e2;
  NetDeviceContainer devices_gw3_e2;
  NetDeviceContainer devices_e2_c2;
  NetDeviceContainer devices_c2_c1;
  NetDeviceContainer devices_c1_e1;  
  NetDeviceContainer devices_e1_r1;
  NetDeviceContainer devices_e1_r2;
  NetDeviceContainer devices_e1_r3;
  NetDeviceContainer devices_r1_s11;
  NetDeviceContainer devices_r1_s12;
  NetDeviceContainer devices_r1_s13;
  NetDeviceContainer devices_r2_s21;
  NetDeviceContainer devices_r2_s22;
  NetDeviceContainer devices_r2_s23;
  NetDeviceContainer devices_r3_s31;
  NetDeviceContainer devices_r3_s32;
  NetDeviceContainer devices_r3_s33;

  
devices_e1_r1   = pointToPoint.Install (edge1,r1);
devices_e1_r2   = pointToPoint.Install (edge1,r2);
devices_e1_r3   = pointToPoint.Install (edge1,r3);
devices_r1_s11  = pointToPoint.Install (r1,s11);
devices_r1_s12  = pointToPoint.Install (r1,s12);
devices_r1_s13  = pointToPoint.Install (r1,s13);
devices_r2_s21  = pointToPoint.Install (r2,s21);
devices_r2_s22  = pointToPoint.Install (r2,s22);
devices_r2_s23  = pointToPoint.Install (r2,s23);
devices_r3_s31  = pointToPoint.Install (r3,s31);
devices_r3_s32  = pointToPoint.Install (r3,s32);
devices_r3_s33  = pointToPoint.Install (r3,s33);
devices_c1_e1  =  pointToPoint.Install(core1, edge1);
devices_c2_c1  =  pointToPoint.Install(core2, core1);
devices_e2_c2  =  pointToPoint.Install(edge2, core2);
devices_gw1_e2 =  pointToPoint.Install(gw1, edge2);
devices_gw2_e2 =  pointToPoint.Install(gw2, edge2);
devices_gw3_e2 =  pointToPoint.Install(gw3, edge2);


InternetStackHelper stack;
stack.Install (nodes);




NS_LOG_INFO ("Assigning addresses to rest of network");
 Ipv4AddressHelper address_gw1_e2;
  address_gw1_e2.SetBase ("30.0.1.0", "255.255.255.0","0.0.0.1");
 Ipv4AddressHelper address_gw2_e2;
  address_gw2_e2.SetBase ("30.0.2.0", "255.255.255.0","0.0.0.1");
 Ipv4AddressHelper address_gw3_e2;
  address_gw3_e2.SetBase ("30.0.3.0", "255.255.255.0","0.0.0.1");
Ipv4AddressHelper address_e2_c2;
  address_e2_c2.SetBase ("30.0.4.0", "255.255.255.0","0.0.0.1");
Ipv4AddressHelper address_c2_c1;
  address_c2_c1.SetBase ("30.0.5.0", "255.255.255.0","0.0.0.1");
Ipv4AddressHelper address_c1_e1;
  address_c1_e1.SetBase ("30.0.6.0", "255.255.255.0","0.0.0.1");
Ipv4AddressHelper address_e1_r1;
  address_e1_r1.SetBase ("30.0.7.0", "255.255.255.0","0.0.0.1");
Ipv4AddressHelper address_e1_r2;
  address_e1_r2.SetBase ("30.0.8.0", "255.255.255.0","0.0.0.1");
Ipv4AddressHelper address_e1_r3;
  address_e1_r3.SetBase ("30.0.9.0", "255.255.255.0","0.0.0.1");
Ipv4AddressHelper address_r1_s11;
  address_r1_s11.SetBase ("30.0.10.0", "255.255.255.0","0.0.0.1");
Ipv4AddressHelper address_r1_s12;
 address_r1_s12.SetBase ("30.0.11.0", "255.255.255.0","0.0.0.1");
Ipv4AddressHelper address_r1_s13;
 address_r1_s13.SetBase ("30.0.12.0", "255.255.255.0","0.0.0.1");
Ipv4AddressHelper address_r2_s21;
  address_r2_s21.SetBase ("30.0.13.0", "255.255.255.0","0.0.0.1");
Ipv4AddressHelper address_r2_s22;
address_r2_s22.SetBase ("30.0.14.0", "255.255.255.0","0.0.0.1");
Ipv4AddressHelper address_r2_s23;
address_r2_s23.SetBase ("30.0.15.0", "255.255.255.0","0.0.0.1");
Ipv4AddressHelper address_r3_s31;
  address_r3_s31.SetBase ("30.0.16.0", "255.255.255.0","0.0.0.1");
Ipv4AddressHelper address_r3_s32;
  address_r3_s32.SetBase ("30.0.17.0", "255.255.255.0","0.0.0.1");
Ipv4AddressHelper address_r3_s33;
  address_r3_s33.SetBase ("30.0.18.0", "255.255.255.0","0.0.0.1");



Ipv4InterfaceContainer interfaces_gw1_e2 = address_gw1_e2.Assign (devices_gw1_e2);
Ipv4InterfaceContainer interfaces_gw2_e2 = address_gw2_e2.Assign (devices_gw2_e2);
Ipv4InterfaceContainer interfaces_gw3_e2 = address_gw3_e2.Assign (devices_gw3_e2);
Ipv4InterfaceContainer interfaces_e2_c2 = address_e2_c2.Assign (devices_e2_c2);
Ipv4InterfaceContainer interfaces_c2_c1 = address_c2_c1.Assign (devices_c2_c1);
Ipv4InterfaceContainer interfaces_c1_e1 = address_c1_e1.Assign (devices_c1_e1);
Ipv4InterfaceContainer interfaces_e1_r1 = address_e1_r1.Assign (devices_e1_r1);
Ipv4InterfaceContainer interfaces_e1_r2 = address_e1_r2.Assign (devices_e1_r2);
Ipv4InterfaceContainer interfaces_e1_r3 = address_e1_r3.Assign (devices_e1_r3);
Ipv4InterfaceContainer interfaces_r1_s11 = address_r1_s11.Assign (devices_r1_s11);
Ipv4InterfaceContainer interfaces_r1_s12 = address_r1_s12.Assign (devices_r1_s12);
Ipv4InterfaceContainer interfaces_r1_s13 = address_r1_s13.Assign (devices_r1_s13);
Ipv4InterfaceContainer interfaces_r2_s21 = address_r2_s21.Assign (devices_r2_s21);
Ipv4InterfaceContainer interfaces_r2_s22 = address_r2_s22.Assign (devices_r2_s22);
Ipv4InterfaceContainer interfaces_r2_s23 = address_r2_s23.Assign (devices_r2_s23);
Ipv4InterfaceContainer interfaces_r3_s31 = address_r3_s31.Assign (devices_r3_s31);
Ipv4InterfaceContainer interfaces_r3_s32 = address_r3_s32.Assign (devices_r3_s32);
Ipv4InterfaceContainer interfaces_r3_s33 = address_r3_s33.Assign (devices_r3_s33);

//Ptr<Node> ue11 = wifiStaNodes1.Get(0);
Ptr<Node> ue12 = wifiStaNodes1.Get(1);
//Ptr<Node> ue13 = wifiStaNodes1.Get(2);
//Ptr<Node> ue21 = wifiStaNodes2.Get(0);
Ptr<Node> ue22 = wifiStaNodes2.Get(1);
//Ptr<Node> ue23 = wifiStaNodes2.Get(2);
//Ptr<Node> ue31 = wifiStaNodes3.Get(0);
Ptr<Node> ue32 = wifiStaNodes3.Get(1);
//Ptr<Node> ue33 = wifiStaNodes3.Get(2);

 //voice applications
NS_LOG_INFO ("Adding voice traffic: s11->ue11");
uint16_t sinkPort_ue11 = 100;
Address sinkAddress_ue11 (InetSocketAddress (interfaces_ue1_an1.GetAddress (0), sinkPort_ue11));
VoiceTrafficHelper helper_s11_ue11 ("ns3::UdpSocketFactory",  sinkAddress_ue11);
ApplicationContainer clientApps_s11 = helper_s11_ue11.Install(s11);
clientApps_s11.Start (Seconds (0.2));
clientApps_s11.Stop (Seconds (20));

NS_LOG_INFO ("Adding voice traffic: s21->ue21");
uint16_t sinkPort_ue21 = 100;
Address sinkAddress_ue21 (InetSocketAddress (interfaces_ue2_an2.GetAddress (0), sinkPort_ue21));
VoiceTrafficHelper helper_s21_ue21 ("ns3::UdpSocketFactory",  sinkAddress_ue21);
ApplicationContainer clientApps_s21 = helper_s21_ue21.Install(s21);
clientApps_s21.Start (Seconds (0.2));
clientApps_s21.Stop (Seconds (20));

NS_LOG_INFO ("Adding voice traffic: s31->ue31");
uint16_t sinkPort_ue31 = 100;
Address sinkAddress_ue31 (InetSocketAddress (interfaces_ue3_an3.GetAddress (0), sinkPort_ue31));
VoiceTrafficHelper helper_s31_ue31 ("ns3::UdpSocketFactory",  sinkAddress_ue31);
ApplicationContainer clientApps_s31 = helper_s31_ue31.Install(s31);
clientApps_s31.Start (Seconds (0.2));
clientApps_s31.Stop (Seconds (20)); 

//AR2video applications

/*Ar2VideoHelper helper_s12_ue12 (interfaces_ue1_an1.GetAddress(1),  200);
ApplicationContainer clientApps_s12 = helper_s12_ue12.Install(s12);
clientApps_s12.Start (Seconds (0.2));
clientApps_s12.Stop (Seconds (20));

Ar2VideoHelper helper_s22_ue22 (interfaces_ue2_an2.GetAddress(1),  200);
ApplicationContainer clientApps_s22 = helper_s22_ue22.Install(s22);
clientApps_s22.Start (Seconds (0.2));
clientApps_s22.Stop (Seconds (20));

Ar2VideoHelper helper_s32_ue32 (interfaces_ue3_an3.GetAddress(1),  200);
ApplicationContainer clientApps_s32 = helper_s32_ue32.Install(s32);
clientApps_s32.Start (Seconds (0.2));
clientApps_s32.Stop (Seconds (20));
*/

NS_LOG_INFO ("Adding video traffic: s12->ue12");
 UdpServerHelper server_s12_ue12 (200);
 ApplicationContainer serverApp_s12 = server_s12_ue12.Install (NodeContainer(s12));
 serverApp_s12.Start (Seconds (0.0));
 serverApp_s12.Stop (Seconds (20.0));
 UdpTraceClientHelper client_s12_ue12 (interfaces_r1_s12.GetAddress (1), 5112, "tracefile.dat");
 client_s12_ue12.SetAttribute ("MaxPacketSize", UintegerValue (1024));
 ApplicationContainer clientApp_ue12 = client_s12_ue12.Install (NodeContainer(ue12));
 clientApp_ue12.Start (Seconds (0.0));
 clientApp_ue12.Stop (Seconds (20.0));

NS_LOG_INFO ("Adding video traffic: s22->ue22");
 UdpServerHelper server_s22_ue22 (200);
 ApplicationContainer serverApp_s22 = server_s22_ue22.Install (NodeContainer(s22));
 serverApp_s22.Start (Seconds (0.0));
 serverApp_s22.Stop (Seconds (20.0));
 UdpTraceClientHelper client_s22_ue22 (interfaces_r2_s22.GetAddress (1), 5112, "tracefile.dat");
 client_s22_ue22.SetAttribute ("MaxPacketSize", UintegerValue (1024));
 ApplicationContainer clientApp_ue22 = client_s22_ue22.Install (NodeContainer(ue22));
 clientApp_ue22.Start (Seconds (0.0));
 clientApp_ue22.Stop (Seconds (20.0));

NS_LOG_INFO ("Adding video traffic: s32->ue32");
 UdpServerHelper server_s32_ue32 (200);
 ApplicationContainer serverApp_s32 = server_s32_ue32.Install (NodeContainer(s32));
 serverApp_s32.Start (Seconds (0.0));
 serverApp_s32.Stop (Seconds (20.0));
 UdpTraceClientHelper client_s32_ue32 (interfaces_r3_s32.GetAddress (1), 5112, "tracefile.dat");
 client_s32_ue32.SetAttribute ("MaxPacketSize", UintegerValue (1024));
 ApplicationContainer clientApp_ue32 = client_s32_ue32.Install (NodeContainer(ue32));
 clientApp_ue32.Start (Seconds (0.0));
 clientApp_ue32.Stop (Seconds (20.0));

Ipv4GlobalRoutingHelper::PopulateRoutingTables ();


Simulator::Stop (Seconds (20.0));

//pointToPoint.EnablePcapAll ("wifi_wired_core");

Simulator::Run ();
Simulator::Destroy ();
return 0;
}
