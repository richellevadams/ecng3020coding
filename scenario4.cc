 #include "ns3/lte-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/virtual-net-device.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/config-store.h"
//#include "ns3/gtk-config-store.h"

#include <iostream>

using namespace ns3;


NS_LOG_COMPONENT_DEFINE ("Scenario4_AllLTE");
int
main (int argc, char *argv[])
{
  //LogComponentEnable ("Ipv4L3Protocol", LOG_LEVEL_ALL);
  //LogComponentEnable ("Ipv4StaticRouting", LOG_LEVEL_ALL);
  uint16_t nNodes = 3;
  //double interPacketInterval = 100;
  double distance = 20.0;

  // Command line arguments
  CommandLine cmd;
  cmd.AddValue("nNodes", "Number of UEs", nNodes);
  cmd.Parse(argc, argv);

// Access Network 1
  Ptr<LteHelper> lteHelper1 = CreateObject<LteHelper> ();
  Ptr<EpcHelper>  epcHelper1 = CreateObject<EpcHelper> ();
  lteHelper1->SetEpcHelper (epcHelper1);
  lteHelper1->SetSchedulerType("ns3::PfFfMacScheduler");

  Ptr<Node> pgw1 = epcHelper1->GetPgwNode ();

  Ptr<NetDevice> tunDevice1 = pgw1->GetDevice(1);
  //tunDevice1->SetNode(pgw1);
  NetDeviceContainer tunDeviceContainer1;
  tunDeviceContainer1.Add(tunDevice1);
  Ipv4AddressHelper ueAddress1;
  ueAddress1.SetBase("20.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer tunDeviceIpv4IfContainer1 = ueAddress1.Assign(tunDeviceContainer1);

  epcHelper1->m_s1uIpv4AddressHelper.SetBase ("10.1.0.0", "255.255.255.252");

  NodeContainer ueNodes1;
  NodeContainer enbNode1;
  enbNode1.Create(1);
  ueNodes1.Create(nNodes);
  
  NodeContainer positionedNodes1;
  positionedNodes1.Add(ueNodes1.Get(0));
  positionedNodes1.Add(enbNode1.Get(0));
  positionedNodes1.Add(ueNodes1.Get(1));
  positionedNodes1.Add(ueNodes1.Get(2));

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
 

  // Install LTE Devices to the nodes
  NetDeviceContainer enbLteDevs1 = lteHelper1->InstallEnbDevice (enbNode1);
  NetDeviceContainer ueLteDevs1 = lteHelper1->InstallUeDevice (ueNodes1);

  // Attach UEs to eNodeB
  for (uint16_t i = 0; i < nNodes; i++)
      {
        lteHelper1->Attach (ueLteDevs1.Get(i), enbLteDevs1.Get(0));
      }

  // Install the IP stack on the UEs
  InternetStackHelper stackLTE1;
  stackLTE1.Install (ueNodes1);
  Ipv4InterfaceContainer ueIpIface1;
  //ueIpIface1 = epcHelper1->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs1));
  ueIpIface1 = ueAddress1.Assign(ueLteDevs1);
  Ipv4StaticRoutingHelper ipv4RoutingHelper1;
  // Assign IP address to UEs, and install applications
  for (uint32_t u = 0; u < ueNodes1.GetN (); ++u)
    {
      Ptr<Node> ueNode1 = ueNodes1.Get (u);
      // Set the default gateway for the UE
      Ptr<Ipv4StaticRouting> ueStaticRouting1 = ipv4RoutingHelper1.GetStaticRouting (ueNode1->GetObject<Ipv4> ());
      ueStaticRouting1->SetDefaultRoute (epcHelper1->GetUeDefaultGatewayAddress (), 1);
    }

 lteHelper1->ActivateEpsBearer (ueLteDevs1.Get(0), EpsBearer (EpsBearer::GBR_CONV_VOICE), EpcTft::Default ());
 lteHelper1->ActivateEpsBearer (ueLteDevs1.Get(1), EpsBearer (EpsBearer::GBR_NON_CONV_VIDEO), EpcTft::Default ());
 lteHelper1->ActivateEpsBearer (ueLteDevs1.Get(2), EpsBearer (EpsBearer::NGBR_VIDEO_TCP_DEFAULT), EpcTft::Default ());

  lteHelper1->EnableTraces ();
  // Uncomment to enable PCAP tracing
  //p2ph.EnablePcapAll("lena-epc-first"); 

// Access Network 2

  Ptr<LteHelper> lteHelper2 = CreateObject<LteHelper> ();
  Ptr<EpcHelper>  epcHelper2 = CreateObject<EpcHelper> ();
  lteHelper2->SetEpcHelper (epcHelper2);
  lteHelper2->SetSchedulerType("ns3::PfFfMacScheduler");

  Ptr<Node> pgw2 = epcHelper2->GetPgwNode ();

  Ptr<NetDevice> tunDevice2 = pgw2->GetDevice(1);
  //tunDevice2->SetNode(pgw2);
  NetDeviceContainer tunDeviceContainer2;
  tunDeviceContainer2.Add(tunDevice2);
  Ipv4AddressHelper ueAddress2;
  ueAddress2.SetBase("20.2.1.0", "255.255.255.0");
  Ipv4InterfaceContainer tunDeviceIpv4IfContainer2 = ueAddress2.Assign(tunDeviceContainer2);

  epcHelper2->m_s1uIpv4AddressHelper.SetBase ("10.2.0.0", "255.255.255.252");

  NodeContainer ueNodes2;
  NodeContainer enbNode2;
  enbNode2.Create(1);
  ueNodes2.Create(nNodes);

  NodeContainer positionedNodes2;
  positionedNodes2.Add(ueNodes2.Get(0));
  positionedNodes2.Add(enbNode2.Get(0));
  positionedNodes2.Add(ueNodes2.Get(1));
  positionedNodes2.Add(ueNodes2.Get(2));

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

  // Install LTE Devices to the nodes
  NetDeviceContainer enbLteDevs2 = lteHelper2->InstallEnbDevice (enbNode2);
  NetDeviceContainer ueLteDevs2 = lteHelper2->InstallUeDevice (ueNodes2);

  // Attach one UE per eNodeB
  for (uint16_t i = 0; i < nNodes; i++)
      {
        lteHelper2->Attach (ueLteDevs2.Get(i), enbLteDevs2.Get(0));
      }

  // Install the IP stack on the UEs
  InternetStackHelper stackLTE2;
  stackLTE2.Install (ueNodes2);
  Ipv4InterfaceContainer ueIpIface2;
  //ueIpIface2 = epcHelper2->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs2));
  ueIpIface2 = ueAddress2.Assign(ueLteDevs2);
  Ipv4StaticRoutingHelper ipv4RoutingHelper2;
  // Assign IP address to UEs, and install applications
  for (uint32_t u = 0; u < ueNodes2.GetN (); ++u)
    {
      Ptr<Node> ueNode2 = ueNodes2.Get (u);
      // Set the default gateway for the UE
      Ptr<Ipv4StaticRouting> ueStaticRouting2 = ipv4RoutingHelper2.GetStaticRouting (ueNode2->GetObject<Ipv4> ());
      ueStaticRouting2->SetDefaultRoute (epcHelper2->GetUeDefaultGatewayAddress (), 1);
    }
  lteHelper2->ActivateEpsBearer (ueLteDevs2.Get(0), EpsBearer (EpsBearer::GBR_CONV_VOICE), EpcTft::Default ());
  lteHelper2->ActivateEpsBearer (ueLteDevs2.Get(1), EpsBearer (EpsBearer::GBR_NON_CONV_VIDEO), EpcTft::Default ());
  lteHelper2->ActivateEpsBearer (ueLteDevs2.Get(2), EpsBearer (EpsBearer::NGBR_VIDEO_TCP_DEFAULT), EpcTft::Default ());

  lteHelper2->EnableTraces ();
  // Uncomment to enable PCAP tracing
  //p2ph.EnablePcapAll("lena-epc-first")

//Access Network 3
 Ptr<LteHelper> lteHelper3 = CreateObject<LteHelper> ();
  Ptr<EpcHelper>  epcHelper3 = CreateObject<EpcHelper> ();
  lteHelper3->SetEpcHelper (epcHelper3);
  lteHelper3->SetSchedulerType("ns3::PfFfMacScheduler");

  Ptr<Node> pgw3 = epcHelper3->GetPgwNode ();

  Ptr<NetDevice> tunDevice3 = pgw3->GetDevice(1);
  //tunDevice3->SetNode(pgw3);
  NetDeviceContainer tunDeviceContainer3;
  tunDeviceContainer3.Add(tunDevice3);
  Ipv4AddressHelper ueAddress3;
  ueAddress3.SetBase("20.3.1.0", "255.255.255.0");
  Ipv4InterfaceContainer tunDeviceIpv4IfContainer3 = ueAddress3.Assign(tunDeviceContainer3);

  epcHelper3->m_s1uIpv4AddressHelper.SetBase ("10.3.0.0", "255.255.255.252");

  NodeContainer ueNodes3;
  NodeContainer enbNode3;
  enbNode3.Create(1);
  ueNodes3.Create(nNodes);

  NodeContainer positionedNodes3;
  positionedNodes3.Add(ueNodes3.Get(0));
  positionedNodes3.Add(enbNode3.Get(0));
  positionedNodes3.Add(ueNodes3.Get(1));
  positionedNodes3.Add(ueNodes3.Get(2));

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

  // Install LTE Devices to the nodes
  NetDeviceContainer enbLteDevs3 = lteHelper3->InstallEnbDevice (enbNode3);
  NetDeviceContainer ueLteDevs3 = lteHelper3->InstallUeDevice (ueNodes3);

  // Attach one UE per eNodeB
  for (uint16_t i = 0; i < nNodes; i++)
      {
        lteHelper3->Attach (ueLteDevs3.Get(i), enbLteDevs3.Get(0));
      }

  // Install the IP stack on the UEs
  InternetStackHelper stackLTE3;
  stackLTE3.Install (ueNodes3);
  Ipv4InterfaceContainer ueIpIface3;
  //ueIpIface3 = epcHelper3->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs3));
  ueIpIface3 = ueAddress3.Assign(ueLteDevs3);
  Ipv4StaticRoutingHelper ipv4RoutingHelper3;
  // Assign IP address to UEs, and install applications
  for (uint32_t u = 0; u < ueNodes3.GetN (); ++u)
    {
      Ptr<Node> ueNode3 = ueNodes3.Get (u);
      // Set the default gateway for the UE
      Ptr<Ipv4StaticRouting> ueStaticRouting3 = ipv4RoutingHelper3.GetStaticRouting (ueNode3->GetObject<Ipv4> ());
      ueStaticRouting3->SetDefaultRoute (epcHelper3->GetUeDefaultGatewayAddress (), 1);
    }
  lteHelper3->ActivateEpsBearer (ueLteDevs3.Get(0), EpsBearer (EpsBearer::GBR_CONV_VOICE), EpcTft::Default ());
  lteHelper3->ActivateEpsBearer (ueLteDevs3.Get(1), EpsBearer (EpsBearer::GBR_NON_CONV_VIDEO), EpcTft::Default ());
  lteHelper3->ActivateEpsBearer (ueLteDevs3.Get(2), EpsBearer (EpsBearer::NGBR_VIDEO_TCP_DEFAULT), EpcTft::Default ());

  lteHelper3->EnableTraces ();
  // Uncomment to enable PCAP tracing
  //p3ph.EnablePcapAll("lena-epc-first");


  NodeContainer nodes;
  nodes.Create (16);

  Ptr<Node> gw1 = pgw1;
  Ptr<Node> gw2 = pgw2;
  Ptr<Node> gw3	= pgw3;
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


uint32_t ifIndx_4 = devices_c1_e1.Get(1)->GetIfIndex();
Ipv4Address ipAddress_4 = interfaces_c1_e1.GetAddress(1);
uint32_t ifIndx_3 = devices_e1_r3.Get(0)->GetIfIndex();
Ipv4Address ipAddress_3 = interfaces_e1_r3.GetAddress(0);
uint32_t ifIndx_2 = devices_e1_r2.Get(0)->GetIfIndex();
Ipv4Address ipAddress_2 = interfaces_e1_r2.GetAddress(0);
uint32_t ifIndx_1 = devices_e1_r1.Get(0)->GetIfIndex();
Ipv4Address ipAddress_1 = interfaces_e1_r1.GetAddress(0);
 cout<<"e1_4 = " << ifIndx_4 <<" with address = " << ipAddress_4 <<"; e1_3 = " << ifIndx_3 <<" with address = "<<ipAddress_3<<"; e1_2 = " << ifIndx_2 <<" with address = "<<ipAddress_2<<"; e1_1 = " << ifIndx_1 <<" with address = "<<ipAddress_1<< endl;

//voice applications
uint16_t sinkPort_ue11 = 100;
Address sinkAddress_ue11 (InetSocketAddress (ueIpIface1.GetAddress (0), sinkPort_ue11));
VoiceTrafficHelper helper_s11_ue11 ("ns3::UdpSocketFactory",  sinkAddress_ue11);
ApplicationContainer clientApps_s11 = helper_s11_ue11.Install(s11);
clientApps_s11.Start (Seconds (0.2));
clientApps_s11.Stop (Seconds (20));

uint16_t sinkPort_ue21 = 100;
Address sinkAddress_ue21 (InetSocketAddress (ueIpIface2.GetAddress (0), sinkPort_ue21));
VoiceTrafficHelper helper_s21_ue21 ("ns3::UdpSocketFactory",  sinkAddress_ue21);
ApplicationContainer clientApps_s21 = helper_s21_ue21.Install(s21);
clientApps_s21.Start (Seconds (0.2));
clientApps_s21.Stop (Seconds (20));

uint16_t sinkPort_ue31 = 100;
Address sinkAddress_ue31 (InetSocketAddress (ueIpIface3.GetAddress (0), sinkPort_ue31));
VoiceTrafficHelper helper_s31_ue31 ("ns3::UdpSocketFactory",  sinkAddress_ue31);
ApplicationContainer clientApps_s31 = helper_s31_ue31.Install(s31);
clientApps_s31.Start (Seconds (0.2));
clientApps_s31.Stop (Seconds (20));

//AR2video applications
Ar2VideoHelper helper_s12_ue12 (ueIpIface1.GetAddress(1),  200);
ApplicationContainer clientApps_s12 = helper_s12_ue12.Install(s12);
clientApps_s12.Start (Seconds (0.2));
clientApps_s12.Stop (Seconds (20));

Ar2VideoHelper helper_s22_ue22 (ueIpIface2.GetAddress(1),  200);
ApplicationContainer clientApps_s22 = helper_s22_ue22.Install(s22);
clientApps_s22.Start (Seconds (0.2));
clientApps_s22.Stop (Seconds (20));

Ar2VideoHelper helper_s32_ue32 (ueIpIface3.GetAddress(1),  200);
ApplicationContainer clientApps_s32 = helper_s32_ue32.Install(s32);
clientApps_s32.Start (Seconds (0.2));
clientApps_s32.Stop (Seconds (20));


  Ipv4StaticRoutingHelper ipv4RoutingHelper;

  //Left-to-Right Direction
  Ptr<Ipv4StaticRouting> s11StaticRouting = ipv4RoutingHelper.GetStaticRouting (s11->GetObject<Ipv4> ());
  s11StaticRouting->AddNetworkRouteTo (Ipv4Address ("20.1.1.0"), Ipv4Mask ("255.255.255.0"), Ipv4Address ("30.0.10.1"), 1);

  Ptr<Ipv4StaticRouting> s12StaticRouting = ipv4RoutingHelper.GetStaticRouting (s12->GetObject<Ipv4> ());
  s12StaticRouting->AddNetworkRouteTo (Ipv4Address ("20.1.1.0"), Ipv4Mask ("255.255.255.0"), Ipv4Address ("30.0.10.3"), 1);

  Ptr<Ipv4StaticRouting> s13StaticRouting = ipv4RoutingHelper.GetStaticRouting (s13->GetObject<Ipv4> ());
  s13StaticRouting->AddNetworkRouteTo (Ipv4Address ("20.1.1.0"), Ipv4Mask ("255.255.255.0"), Ipv4Address ("30.0.10.5"), 1);

  Ptr<Ipv4StaticRouting> r1StaticRouting = ipv4RoutingHelper.GetStaticRouting (r1->GetObject<Ipv4> ());
  r1StaticRouting->AddNetworkRouteTo (Ipv4Address ("20.1.1.0"), Ipv4Mask ("255.255.255.0"), Ipv4Address ("30.0.7.1"), 1);

  Ptr<Ipv4StaticRouting> s21StaticRouting = ipv4RoutingHelper.GetStaticRouting (s21->GetObject<Ipv4> ());
  s21StaticRouting->AddNetworkRouteTo (Ipv4Address ("20.2.1.0"), Ipv4Mask ("255.255.255.0"), Ipv4Address ("30.0.11.1"), 1);

  Ptr<Ipv4StaticRouting> s22StaticRouting = ipv4RoutingHelper.GetStaticRouting (s22->GetObject<Ipv4> ());
  s22StaticRouting->AddNetworkRouteTo (Ipv4Address ("20.2.1.0"), Ipv4Mask ("255.255.255.0"), Ipv4Address ("30.0.11.3"), 1);

  Ptr<Ipv4StaticRouting> s23StaticRouting = ipv4RoutingHelper.GetStaticRouting (s23->GetObject<Ipv4> ());
  s23StaticRouting->AddNetworkRouteTo (Ipv4Address ("20.2.1.0"), Ipv4Mask ("255.255.255.0"), Ipv4Address ("30.0.11.5"), 1);

  Ptr<Ipv4StaticRouting> r2StaticRouting = ipv4RoutingHelper.GetStaticRouting (r2->GetObject<Ipv4> ());
  r2StaticRouting->AddNetworkRouteTo (Ipv4Address ("20.2.1.0"), Ipv4Mask ("255.255.255.0"), Ipv4Address ("30.0.8.1"), 1);

  Ptr<Ipv4StaticRouting> s31StaticRouting = ipv4RoutingHelper.GetStaticRouting (s31->GetObject<Ipv4> ());
  s31StaticRouting->AddNetworkRouteTo (Ipv4Address ("20.3.1.0"), Ipv4Mask ("255.255.255.0"), Ipv4Address ("30.0.12.1"), 1);

  Ptr<Ipv4StaticRouting> s32StaticRouting = ipv4RoutingHelper.GetStaticRouting (s32->GetObject<Ipv4> ());
  s32StaticRouting->AddNetworkRouteTo (Ipv4Address ("20.3.1.0"), Ipv4Mask ("255.255.255.0"), Ipv4Address ("30.0.12.3"), 1);

  Ptr<Ipv4StaticRouting> s33StaticRouting = ipv4RoutingHelper.GetStaticRouting (s33->GetObject<Ipv4> ());
  s33StaticRouting->AddNetworkRouteTo (Ipv4Address ("20.3.1.0"), Ipv4Mask ("255.255.255.0"), Ipv4Address ("30.0.12.5"), 1);

  Ptr<Ipv4StaticRouting> r3StaticRouting = ipv4RoutingHelper.GetStaticRouting (r3->GetObject<Ipv4> ());
  r3StaticRouting->AddNetworkRouteTo (Ipv4Address ("20.3.1.0"), Ipv4Mask ("255.255.255.0"), Ipv4Address ("30.0.9.1"), 1);

  Ptr<Ipv4StaticRouting> e1StaticRouting = ipv4RoutingHelper.GetStaticRouting (edge1->GetObject<Ipv4> ());
  e1StaticRouting->AddNetworkRouteTo (Ipv4Address ("20.1.1.0"), Ipv4Mask ("255.255.255.0"), Ipv4Address ("30.0.6.1"), 1);
  e1StaticRouting->AddNetworkRouteTo (Ipv4Address ("20.2.1.0"), Ipv4Mask ("255.255.255.0"), Ipv4Address ("30.0.6.1"), 1);
  e1StaticRouting->AddNetworkRouteTo (Ipv4Address ("20.3.1.0"), Ipv4Mask ("255.255.255.0"), Ipv4Address ("30.0.6.1"), 1);
  e1StaticRouting->AddNetworkRouteTo (Ipv4Address ("30.0.10.0"), Ipv4Mask ("255.255.255.0"), Ipv4Address ("30.0.7.2"), 2);
  e1StaticRouting->AddNetworkRouteTo (Ipv4Address ("30.0.11.0"), Ipv4Mask ("255.255.255.0"), Ipv4Address ("30.0.8.2"), 3);
  e1StaticRouting->AddNetworkRouteTo (Ipv4Address ("30.0.12.0"), Ipv4Mask ("255.255.255.0"), Ipv4Address ("30.0.9.2"), 4);

  Ptr<Ipv4StaticRouting> c1StaticRouting = ipv4RoutingHelper.GetStaticRouting (core1->GetObject<Ipv4> ());
  c1StaticRouting->AddNetworkRouteTo (Ipv4Address ("20.1.1.0"), Ipv4Mask ("255.255.255.0"), Ipv4Address ("30.0.5.1"), 1);
  c1StaticRouting->AddNetworkRouteTo (Ipv4Address ("20.2.1.0"), Ipv4Mask ("255.255.255.0"), Ipv4Address ("30.0.5.1"), 1);
  c1StaticRouting->AddNetworkRouteTo (Ipv4Address ("20.3.1.0"), Ipv4Mask ("255.255.255.0"), Ipv4Address ("30.0.5.1"), 1);
  c1StaticRouting->AddNetworkRouteTo (Ipv4Address ("30.0.10.0"), Ipv4Mask ("255.255.255.0"), Ipv4Address ("30.0.6.2"), 2);
  c1StaticRouting->AddNetworkRouteTo (Ipv4Address ("30.0.11.0"), Ipv4Mask ("255.255.255.0"), Ipv4Address ("30.0.6.2"), 2);
  c1StaticRouting->AddNetworkRouteTo (Ipv4Address ("30.0.12.0"), Ipv4Mask ("255.255.255.0"), Ipv4Address ("30.0.6.2"), 2);

  Ptr<Ipv4StaticRouting> c2StaticRouting = ipv4RoutingHelper.GetStaticRouting (core2->GetObject<Ipv4> ());
  c2StaticRouting->AddNetworkRouteTo (Ipv4Address ("20.1.1.0"), Ipv4Mask ("255.255.255.0"), Ipv4Address ("30.0.4.1"), 1);
  c2StaticRouting->AddNetworkRouteTo (Ipv4Address ("20.2.1.0"), Ipv4Mask ("255.255.255.0"), Ipv4Address ("30.0.4.1"), 1);
  c2StaticRouting->AddNetworkRouteTo (Ipv4Address ("20.3.1.0"), Ipv4Mask ("255.255.255.0"), Ipv4Address ("30.0.4.1"), 1);
  c2StaticRouting->AddNetworkRouteTo (Ipv4Address ("30.0.10.0"), Ipv4Mask ("255.255.255.0"), Ipv4Address ("30.0.5.2"), 2);
  c2StaticRouting->AddNetworkRouteTo (Ipv4Address ("30.0.11.0"), Ipv4Mask ("255.255.255.0"), Ipv4Address ("30.0.5.2"), 2);
  c2StaticRouting->AddNetworkRouteTo (Ipv4Address ("30.0.12.0"), Ipv4Mask ("255.255.255.0"), Ipv4Address ("30.0.5.2"), 2);

  Ptr<Ipv4StaticRouting> e2StaticRouting = ipv4RoutingHelper.GetStaticRouting (edge2->GetObject<Ipv4> ());
  e2StaticRouting->AddNetworkRouteTo (Ipv4Address ("20.1.1.0"), Ipv4Mask ("255.255.255.0"), Ipv4Address ("30.0.1.1"), 1);
  e2StaticRouting->AddNetworkRouteTo (Ipv4Address ("20.2.1.0"), Ipv4Mask ("255.255.255.0"), Ipv4Address ("30.0.2.1"), 2);
  e2StaticRouting->AddNetworkRouteTo (Ipv4Address ("20.3.1.0"), Ipv4Mask ("255.255.255.0"), Ipv4Address ("30.0.3.1"), 3);
  e2StaticRouting->AddNetworkRouteTo (Ipv4Address ("30.0.10.0"), Ipv4Mask ("255.255.255.0"), Ipv4Address ("30.0.4.2"), 4);
  e2StaticRouting->AddNetworkRouteTo (Ipv4Address ("30.0.11.0"), Ipv4Mask ("255.255.255.0"), Ipv4Address ("30.0.4.2"), 4);
  e2StaticRouting->AddNetworkRouteTo (Ipv4Address ("30.0.12.0"), Ipv4Mask ("255.255.255.0"), Ipv4Address ("30.0.4.2"), 4);

 
  Ptr<Ipv4StaticRouting> gw1StaticRouting = ipv4RoutingHelper.GetStaticRouting (gw1->GetObject<Ipv4> ());
  gw1StaticRouting->AddHostRouteTo (Ipv4Address ("30.0.10.2"), Ipv4Address ("30.0.1.2"), 3);
  gw1StaticRouting->AddHostRouteTo (Ipv4Address ("30.0.10.4"), Ipv4Address ("30.0.1.2"), 3);
  gw1StaticRouting->AddHostRouteTo (Ipv4Address ("30.0.10.6"), Ipv4Address ("30.0.1.2"), 3);

  Ptr<Ipv4StaticRouting> gw2StaticRouting = ipv4RoutingHelper.GetStaticRouting (gw2->GetObject<Ipv4> ());
  gw2StaticRouting->AddHostRouteTo (Ipv4Address ("30.0.11.2"), Ipv4Address ("30.0.2.2"), 3);
  gw2StaticRouting->AddHostRouteTo (Ipv4Address ("30.0.11.4"), Ipv4Address ("30.0.2.2"), 3);
  gw2StaticRouting->AddHostRouteTo (Ipv4Address ("30.0.11.6"), Ipv4Address ("30.0.2.2"), 3);

  Ptr<Ipv4StaticRouting> gw3StaticRouting = ipv4RoutingHelper.GetStaticRouting (gw3->GetObject<Ipv4> ());
  gw3StaticRouting->AddHostRouteTo (Ipv4Address ("30.0.12.2"), Ipv4Address ("30.0.3.2"), 3);
  gw3StaticRouting->AddHostRouteTo (Ipv4Address ("30.0.12.4"), Ipv4Address ("30.0.3.2"), 3);
  gw3StaticRouting->AddHostRouteTo (Ipv4Address ("30.0.12.6"), Ipv4Address ("30.0.3.2"), 3);
  
  
  //Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  // Print the routing table
  
  OutputStreamWrapper ows("/home/richelle/Documents/fluidqueue/tarballs/Routing_TableLTE.txt", std::ios::out);
  Ptr<OutputStreamWrapper> w(&ows);

  
  s11StaticRouting->PrintRoutingTable(w);
  s12StaticRouting->PrintRoutingTable(w);
  s13StaticRouting->PrintRoutingTable(w);
  s21StaticRouting->PrintRoutingTable(w);
  s22StaticRouting->PrintRoutingTable(w);
  s23StaticRouting->PrintRoutingTable(w);
  s31StaticRouting->PrintRoutingTable(w);
  s32StaticRouting->PrintRoutingTable(w);
  s33StaticRouting->PrintRoutingTable(w);
  r1StaticRouting->PrintRoutingTable(w);
  r2StaticRouting->PrintRoutingTable(w);
  r3StaticRouting->PrintRoutingTable(w);
  e1StaticRouting->PrintRoutingTable(w);
  e2StaticRouting->PrintRoutingTable(w);
  c1StaticRouting->PrintRoutingTable(w);
  c2StaticRouting->PrintRoutingTable(w);
  gw1StaticRouting->PrintRoutingTable(w);
  gw2StaticRouting->PrintRoutingTable(w);
  gw3StaticRouting->PrintRoutingTable(w);
  
  Simulator::Stop(Seconds(20.0));

  pointToPoint.EnablePcapAll("lte-epc-wired");
  Simulator::Run();

  /*GtkConfigStore config;
  config.ConfigureAttributes();*/

  Simulator::Destroy();
  return 0;

}

