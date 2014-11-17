#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wimax-module.h"
#include "ns3/internet-module.h"
#include "ns3/global-route-manager.h"
#include "ns3/ipcs-classifier-record.h"
#include "ns3/service-flow.h"
#include "ns3/vector.h"
#include <iostream>

NS_LOG_COMPONENT_DEFINE ("Scenario3_AllWiMAX");

#define MAXSS 10

using namespace ns3;

void
AddServiceFlowAtBS(Ptr<BaseStationNetDevice> bs, ServiceFlow  sf, uint32_t sfid, enum WimaxPhy::ModulationType modulation)
{
  ServiceFlow * serviceFlow = new ServiceFlow ();
  serviceFlow->CopyParametersFrom (sf);
  Ptr<WimaxConnection> transportConnection = bs->GetConnectionManager ()->CreateConnection (Cid::MULTICAST);
  serviceFlow->SetConnection (transportConnection);
  serviceFlow->SetUnsolicitedGrantInterval (1);
  serviceFlow->SetUnsolicitedPollingInterval (1);
  serviceFlow->SetConvergenceSublayerParam (sf.GetConvergenceSublayerParam ());
  serviceFlow->SetIsEnabled (true);
  serviceFlow->SetType (ServiceFlow::SF_TYPE_ACTIVE);
  serviceFlow->SetIsMulticast (true);
  serviceFlow->SetModulation (modulation);
  serviceFlow->SetSfid(sfid);
  bs->GetServiceFlowManager()->AddServiceFlow (serviceFlow);
  NS_LOG_INFO ("BsServiceFlowManager: Creating a new Service flow: SFID = " << serviceFlow->GetSfid () << " CID = "
                                                                                << serviceFlow->GetCid ());
  
}


int main (int argc, char *argv[])
{
  LogComponentEnable("Scenario3_AllWiMAX", LOG_LEVEL_INFO);
  LogComponentEnable("IpcsClassifierRecord", LOG_LEVEL_INFO);
  //LogComponentEnable("SubscriberStationNetDevice", LOG_LEVEL_INFO);
  LogComponentEnable("BaseStationNetDevice", LOG_LEVEL_INFO);
  LogComponentEnable("BsServiceFlowManager", LOG_LEVEL_INFO);
 

  int schedType = 0;
  int nNodes = 3;
  double distance = 20.0;
  WimaxHelper::SchedulerType scheduler = WimaxHelper::SCHED_TYPE_RTPS;

  CommandLine cmd;
  cmd.AddValue ("scheduler", "type of scheduler to use with the network devices", schedType);
  cmd.AddValue ("nNodes", "number of subscriber station to create", nNodes);
  cmd.Parse (argc, argv);
 
  switch (schedType)
    {
    case 0:
      scheduler = WimaxHelper::SCHED_TYPE_SIMPLE;
      break;
    case 1:
      scheduler = WimaxHelper::SCHED_TYPE_MBQOS;
      break;
    case 2:
      scheduler = WimaxHelper::SCHED_TYPE_RTPS;
      break;
    default:
      scheduler = WimaxHelper::SCHED_TYPE_SIMPLE;
    }

//Access Network 1
  NodeContainer ssNodes1;
  NodeContainer bsNodes1;
  NodeContainer ASNGW_Node1;

  ssNodes1.Create (nNodes);
  bsNodes1.Create (1);
  ASNGW_Node1.Create(1);

  Ptr<SimpleOfdmWimaxChannel> channel1;
  channel1 = CreateObject<SimpleOfdmWimaxChannel> ();
  channel1->SetPropagationModel (SimpleOfdmWimaxChannel::COST231_PROPAGATION);

  Ptr<SubscriberStationNetDevice> ss1[MAXSS];
  NetDeviceContainer ssDevs1;
  Ipv4InterfaceContainer SSinterfaces1;

  Ptr<BaseStationNetDevice> bs1;
  NetDeviceContainer bsDevs1, bsDevsOne1;
  Ipv4InterfaceContainer BSinterfaces1;

  WimaxHelper wimax1;

  ssDevs1 = wimax1.Install (ssNodes1,
                          WimaxHelper::DEVICE_TYPE_SUBSCRIBER_STATION,
                          WimaxHelper::SIMPLE_PHY_TYPE_OFDM,
                          channel1,
                          scheduler);
 
  bsDevs1 = wimax1.Install (bsNodes1,
                          WimaxHelper::DEVICE_TYPE_BASE_STATION,
                          WimaxHelper::SIMPLE_PHY_TYPE_OFDM,
                          channel1,
                          scheduler);
 

  for (int i = 0; i < nNodes; i++)
    {
      ss1[i] = ssDevs1.Get (i)->GetObject<SubscriberStationNetDevice> ();
      ss1[i]->SetModulationType (WimaxPhy::MODULATION_TYPE_QAM16_12);
    }

  bs1 = bsDevs1.Get (0)->GetObject<BaseStationNetDevice> ();

  NodeContainer positionedNodes1;
  positionedNodes1.Add(ssNodes1.Get(0));
  positionedNodes1.Add(bsNodes1.Get(0));
  positionedNodes1.Add(ssNodes1.Get(1));
  positionedNodes1.Add(ssNodes1.Get(2));

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
 

  InternetStackHelper stackWiMAX1;
  stackWiMAX1.Install (bsNodes1);
  stackWiMAX1.Install (ssNodes1);
  stackWiMAX1.Install (ASNGW_Node1);
 

  NodeContainer LAN_ASN_BS1;
  LAN_ASN_BS1.Add (bsNodes1.Get (0));
  LAN_ASN_BS1.Add (ASNGW_Node1.Get (0));

  CsmaHelper csmaASN_BS1;
  csmaASN_BS1.SetChannelAttribute ("DataRate", DataRateValue (DataRate (10000000)));
  csmaASN_BS1.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));
  csmaASN_BS1.SetDeviceAttribute ("Mtu", UintegerValue (1500));

  NetDeviceContainer LAN_ASN_BS_Devs1 = csmaASN_BS1.Install (LAN_ASN_BS1);
 
  NetDeviceContainer BS_CSMADevs1;
  BS_CSMADevs1.Add (LAN_ASN_BS_Devs1.Get (0));
  NetDeviceContainer ASN_Devs11;
  ASN_Devs11.Add (LAN_ASN_BS_Devs1.Get (1));


  Ipv4AddressHelper address1;
  address1.SetBase ("20.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer BSCSMAInterfaces1 = address1.Assign (BS_CSMADevs1);
  Ipv4InterfaceContainer ASNCSMAInterfaces1 = address1.Assign (ASN_Devs11);

  address1.SetBase ("20.1.2.0", "255.255.255.0");
  SSinterfaces1 = address1.Assign (ssDevs1);
  BSinterfaces1 = address1.Assign (bsDevs1);

  wimax1.EnablePcap ("wimax-simple-ss10", ssNodes1.Get (0)->GetId (), ss1[0]->GetIfIndex ());
  wimax1.EnablePcap ("wimax-simple-ss11", ssNodes1.Get (1)->GetId (), ss1[1]->GetIfIndex ());
  wimax1.EnablePcap ("wimax-simple-ss12", ssNodes1.Get (2)->GetId (), ss1[2]->GetIfIndex ());
  wimax1.EnablePcap ("wimax-simple-bs10", bsNodes1.Get (0)->GetId (), bs1->GetIfIndex ());
 
 

  IpcsClassifierRecord DlClassifierUgs10 (Ipv4Address ("0.0.0.0"),
                                        Ipv4Mask ("0.0.0.0"),
                                        SSinterfaces1.GetAddress (0),
                                        Ipv4Mask ("255.255.255.255"),
                                        0,
                                        65000,
                                        100,
                                        100,
                                        17,
                                        1);
  ServiceFlow DlServiceFlowUgs10 = wimax1.CreateServiceFlow (ServiceFlow::SF_DIRECTION_DOWN,
                                                          ServiceFlow::SF_TYPE_UGS,
                                                          DlClassifierUgs10);

 
  IpcsClassifierRecord DlClassifierRtps11 (Ipv4Address ("0.0.0.0"),
                                        Ipv4Mask ("0.0.0.0"),
                                        SSinterfaces1.GetAddress (1),
                                        Ipv4Mask ("255.255.255.255"),
                                        0,
                                        65000,
                                        200,
                                        200,
                                        17,
                                        1);
  ServiceFlow DlServiceFlowRtps11 = wimax1.CreateServiceFlow (ServiceFlow::SF_DIRECTION_DOWN,
                                                          ServiceFlow::SF_TYPE_RTPS,
                                                          DlClassifierRtps11);

  IpcsClassifierRecord DlClassifierBe12 (Ipv4Address ("0.0.0.0"),
                                        Ipv4Mask ("0.0.0.0"),
                                        SSinterfaces1.GetAddress (2),
                                        Ipv4Mask ("255.255.255.255"),
                                        0,
                                        65000,
                                        300,
                                        300,
                                        17,
                                        1);
  ServiceFlow DlServiceFlowBe12 = wimax1.CreateServiceFlow (ServiceFlow::SF_DIRECTION_DOWN,
                                                          ServiceFlow::SF_TYPE_BE,
                                                          DlClassifierBe12);

  DlServiceFlowUgs10.SetSfid(1000);
  DlServiceFlowRtps11.SetSfid(1001);
  DlServiceFlowBe12.SetSfid(1002);

  ss1[0]->AddServiceFlow (DlServiceFlowUgs10);
  ss1[1]->AddServiceFlow (DlServiceFlowRtps11);
  ss1[2]->AddServiceFlow (DlServiceFlowBe12);

  
  AddServiceFlowAtBS(bs1, DlServiceFlowUgs10, 1000, WimaxPhy::MODULATION_TYPE_QAM16_12);
  AddServiceFlowAtBS(bs1, DlServiceFlowRtps11, 1001, WimaxPhy::MODULATION_TYPE_QAM16_12);
  AddServiceFlowAtBS(bs1, DlServiceFlowBe12, 1002, WimaxPhy::MODULATION_TYPE_QAM16_12);

//Access Network 2
  NodeContainer ssNodes2;
  NodeContainer bsNodes2;
  NodeContainer ASNGW_Node2;

  ssNodes2.Create (nNodes);
  bsNodes2.Create (1);
  ASNGW_Node2.Create(1);

  Ptr<SimpleOfdmWimaxChannel> channel2;
  channel2 = CreateObject<SimpleOfdmWimaxChannel> ();
  channel2->SetPropagationModel (SimpleOfdmWimaxChannel::COST231_PROPAGATION);

  Ptr<SubscriberStationNetDevice> ss2[MAXSS];
  NetDeviceContainer ssDevs2;
  Ipv4InterfaceContainer SSinterfaces2;

  Ptr<BaseStationNetDevice> bs2;
  NetDeviceContainer bsDevs2, bsDevsOne2;
  Ipv4InterfaceContainer BSinterfaces2;

  WimaxHelper wimax2;

  ssDevs2 = wimax2.Install (ssNodes2,
                          WimaxHelper::DEVICE_TYPE_SUBSCRIBER_STATION,
                          WimaxHelper::SIMPLE_PHY_TYPE_OFDM,
                          channel2,
                          scheduler);
 
  bsDevs2 = wimax2.Install (bsNodes2,
                          WimaxHelper::DEVICE_TYPE_BASE_STATION,
                          WimaxHelper::SIMPLE_PHY_TYPE_OFDM,
                          channel2,
                          scheduler);
 

  for (int i = 0; i < nNodes; i++)
    {
      ss2[i] = ssDevs2.Get (i)->GetObject<SubscriberStationNetDevice> ();
      ss2[i]->SetModulationType (WimaxPhy::MODULATION_TYPE_QAM16_12);
    }

  bs2 = bsDevs2.Get (0)->GetObject<BaseStationNetDevice> ();

  NodeContainer positionedNodes2;
  positionedNodes2.Add(ssNodes2.Get(0));
  positionedNodes2.Add(bsNodes2.Get(0));
  positionedNodes2.Add(ssNodes2.Get(1));
  positionedNodes2.Add(ssNodes2.Get(2));

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
  

  InternetStackHelper stackWiMAX2;
  stackWiMAX2.Install (bsNodes2);
  stackWiMAX2.Install (ssNodes2);
  stackWiMAX2.Install (ASNGW_Node2);
 

  NodeContainer LAN_ASN_BS2;
  LAN_ASN_BS2.Add (bsNodes2.Get (0));
  LAN_ASN_BS2.Add (ASNGW_Node2.Get (0));

  CsmaHelper csmaASN_BS2;
  csmaASN_BS2.SetChannelAttribute ("DataRate", DataRateValue (DataRate (10000000)));
  csmaASN_BS2.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));
  csmaASN_BS2.SetDeviceAttribute ("Mtu", UintegerValue (1500));

  NetDeviceContainer LAN_ASN_BS_Devs2 = csmaASN_BS2.Install (LAN_ASN_BS2);
 
  NetDeviceContainer BS_CSMADevs2;
  BS_CSMADevs2.Add (LAN_ASN_BS_Devs2.Get (0));
  NetDeviceContainer ASN_Devs22;
  ASN_Devs22.Add (LAN_ASN_BS_Devs2.Get (1));


  Ipv4AddressHelper address2;
  address2.SetBase ("20.2.1.0", "255.255.255.0");
  Ipv4InterfaceContainer BSCSMAInterfaces2 = address2.Assign (BS_CSMADevs2);
  Ipv4InterfaceContainer ASNCSMAInterfaces2 = address2.Assign (ASN_Devs22);

  address2.SetBase ("20.2.2.0", "255.255.255.0");
  SSinterfaces2 = address2.Assign (ssDevs2);
  BSinterfaces2 = address2.Assign (bsDevs2);

  wimax2.EnablePcap ("wimax-simple-ss20", ssNodes2.Get (0)->GetId (), ss2[0]->GetIfIndex ());
  wimax2.EnablePcap ("wimax-simple-ss21", ssNodes2.Get (1)->GetId (), ss2[1]->GetIfIndex ());
  wimax2.EnablePcap ("wimax-simple-ss22", ssNodes2.Get (2)->GetId (), ss2[2]->GetIfIndex ());
  wimax2.EnablePcap ("wimax-simple-bs20", bsNodes2.Get (0)->GetId (), bs2->GetIfIndex ());
 
 

  IpcsClassifierRecord DlClassifierUgs20 (Ipv4Address ("0.0.0.0"),
                                        Ipv4Mask ("0.0.0.0"),
                                        SSinterfaces2.GetAddress (0),
                                        Ipv4Mask ("255.255.255.255"),
                                        0,
                                        65000,
                                        100,
                                        100,
                                        17,
                                        1);
  ServiceFlow DlServiceFlowUgs20 = wimax2.CreateServiceFlow (ServiceFlow::SF_DIRECTION_DOWN,
                                                          ServiceFlow::SF_TYPE_UGS,
                                                          DlClassifierUgs20);

 
  IpcsClassifierRecord DlClassifierRtps21 (Ipv4Address ("0.0.0.0"),
                                        Ipv4Mask ("0.0.0.0"),
                                        SSinterfaces2.GetAddress (1),
                                        Ipv4Mask ("255.255.255.255"),
                                        0,
                                        65000,
                                        200,
                                        200,
                                        17,
                                        1);
  ServiceFlow DlServiceFlowRtps21 = wimax2.CreateServiceFlow (ServiceFlow::SF_DIRECTION_DOWN,
                                                          ServiceFlow::SF_TYPE_RTPS,
                                                          DlClassifierRtps21);

  IpcsClassifierRecord DlClassifierBe22 (Ipv4Address ("0.0.0.0"),
                                        Ipv4Mask ("0.0.0.0"),
                                        SSinterfaces2.GetAddress (2),
                                        Ipv4Mask ("255.255.255.255"),
                                        0,
                                        65000,
                                        300,
                                        300,
                                        17,
                                        1);
  ServiceFlow DlServiceFlowBe22 = wimax2.CreateServiceFlow (ServiceFlow::SF_DIRECTION_DOWN,
                                                          ServiceFlow::SF_TYPE_BE,
                                                          DlClassifierBe22);
  DlServiceFlowUgs20.SetSfid(2000);
  DlServiceFlowRtps21.SetSfid(2001);
  DlServiceFlowBe22.SetSfid(2002);

  ss2[0]->AddServiceFlow (DlServiceFlowUgs20);
  ss2[1]->AddServiceFlow (DlServiceFlowRtps21);
  ss2[2]->AddServiceFlow (DlServiceFlowBe22);


  AddServiceFlowAtBS(bs2, DlServiceFlowUgs20, 2000, WimaxPhy::MODULATION_TYPE_QAM16_12);
  AddServiceFlowAtBS(bs2, DlServiceFlowRtps21, 2001, WimaxPhy::MODULATION_TYPE_QAM16_12);
  AddServiceFlowAtBS(bs2, DlServiceFlowBe22, 2002, WimaxPhy::MODULATION_TYPE_QAM16_12);

//Access Network 3
  NodeContainer ssNodes3;
  NodeContainer bsNodes3;
  NodeContainer ASNGW_Node3;

  ssNodes3.Create (nNodes);
  bsNodes3.Create (1);
  ASNGW_Node3.Create(1);

  Ptr<SimpleOfdmWimaxChannel> channel3;
  channel3 = CreateObject<SimpleOfdmWimaxChannel> ();
  channel3->SetPropagationModel (SimpleOfdmWimaxChannel::COST231_PROPAGATION);

  Ptr<SubscriberStationNetDevice> ss3[MAXSS];
  NetDeviceContainer ssDevs3;
  Ipv4InterfaceContainer SSinterfaces3;

  Ptr<BaseStationNetDevice> bs3;
  NetDeviceContainer bsDevs3, bsDevsOne3;
  Ipv4InterfaceContainer BSinterfaces3;

  WimaxHelper wimax3;

  ssDevs3 = wimax3.Install (ssNodes3,
                          WimaxHelper::DEVICE_TYPE_SUBSCRIBER_STATION,
                          WimaxHelper::SIMPLE_PHY_TYPE_OFDM,
                          channel3,
                          scheduler);
 
  bsDevs3 = wimax3.Install (bsNodes3,
                          WimaxHelper::DEVICE_TYPE_BASE_STATION,
                          WimaxHelper::SIMPLE_PHY_TYPE_OFDM,
                          channel3,
                          scheduler);
 

  for (int i = 0; i < nNodes; i++)
    {
      ss3[i] = ssDevs3.Get (i)->GetObject<SubscriberStationNetDevice> ();
      ss3[i]->SetModulationType (WimaxPhy::MODULATION_TYPE_QAM16_12);
    }

  bs3 = bsDevs3.Get (0)->GetObject<BaseStationNetDevice> ();

  NodeContainer positionedNodes3;
  positionedNodes3.Add(ssNodes3.Get(0));
  positionedNodes3.Add(bsNodes3.Get(0));
  positionedNodes3.Add(ssNodes3.Get(1));
  positionedNodes3.Add(ssNodes3.Get(2));

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
  

  InternetStackHelper stackWiMAX3;
  stackWiMAX3.Install (bsNodes3);
  stackWiMAX3.Install (ssNodes3);
  stackWiMAX3.Install (ASNGW_Node3);
 

  NodeContainer LAN_ASN_BS3;
  LAN_ASN_BS3.Add (bsNodes3.Get (0));
  LAN_ASN_BS3.Add (ASNGW_Node3.Get (0));

  CsmaHelper csmaASN_BS3;
  csmaASN_BS3.SetChannelAttribute ("DataRate", DataRateValue (DataRate (10000000)));
  csmaASN_BS3.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));
  csmaASN_BS3.SetDeviceAttribute ("Mtu", UintegerValue (1500));

  NetDeviceContainer LAN_ASN_BS_Devs3 = csmaASN_BS3.Install (LAN_ASN_BS3);
 
  NetDeviceContainer BS_CSMADevs3;
  BS_CSMADevs3.Add (LAN_ASN_BS_Devs3.Get (0));
  NetDeviceContainer ASN_Devs33;
  ASN_Devs33.Add (LAN_ASN_BS_Devs3.Get (1));


  Ipv4AddressHelper address3;
  address3.SetBase ("20.3.1.0", "255.255.255.0");
  Ipv4InterfaceContainer BSCSMAInterfaces3 = address3.Assign (BS_CSMADevs3);
  Ipv4InterfaceContainer ASNCSMAInterfaces3 = address3.Assign (ASN_Devs33);

  address3.SetBase ("20.3.2.0", "255.255.255.0");
  SSinterfaces3 = address3.Assign (ssDevs3);
  BSinterfaces3 = address3.Assign (bsDevs3);

  wimax3.EnablePcap ("wimax-simple-ss30", ssNodes3.Get (0)->GetId (), ss3[0]->GetIfIndex ());
  wimax3.EnablePcap ("wimax-simple-ss31", ssNodes3.Get (1)->GetId (), ss3[1]->GetIfIndex ());
  wimax3.EnablePcap ("wimax-simple-ss33", ssNodes3.Get (2)->GetId (), ss3[2]->GetIfIndex ());
  wimax3.EnablePcap ("wimax-simple-bs30", bsNodes3.Get (0)->GetId (), bs3->GetIfIndex ());
 
 
//This is for voice application (ugs). For video (rtps) change port number to 330, keep protocol number = 37, change ServiceFlow::SF_TYPE_UGS to
//ServiceFlow::SF_TYPE_RTPS.
  IpcsClassifierRecord DlClassifierUgs30 (Ipv4Address ("0.0.0.0"),
                                        Ipv4Mask ("0.0.0.0"),
                                        SSinterfaces3.GetAddress (0),
                                        Ipv4Mask ("255.255.255.255"),
                                        0,
                                        65000,
                                        100,
                                        100,
                                        17,
                                        1);
  ServiceFlow DlServiceFlowUgs30 = wimax3.CreateServiceFlow (ServiceFlow::SF_DIRECTION_DOWN,
                                                          ServiceFlow::SF_TYPE_UGS,
                                                          DlClassifierUgs30);

 
  IpcsClassifierRecord DlClassifierRtps31 (Ipv4Address ("0.0.0.0"),
                                        Ipv4Mask ("0.0.0.0"),
                                        SSinterfaces3.GetAddress (1),
                                        Ipv4Mask ("255.255.255.255"),
                                        0,
                                        65000,
                                        200,
                                        200,
                                        17,
                                        1);
  ServiceFlow DlServiceFlowRtps31 = wimax3.CreateServiceFlow (ServiceFlow::SF_DIRECTION_DOWN,
                                                          ServiceFlow::SF_TYPE_RTPS,
                                                          DlClassifierRtps31);

  IpcsClassifierRecord DlClassifierBe32 (Ipv4Address ("0.0.0.0"),
                                        Ipv4Mask ("0.0.0.0"),
                                        SSinterfaces3.GetAddress (2),
                                        Ipv4Mask ("255.255.255.255"),
                                        0,
                                        65000,
                                        300,
                                        300,
                                        17,
                                        1);
  ServiceFlow DlServiceFlowBe32 = wimax3.CreateServiceFlow (ServiceFlow::SF_DIRECTION_DOWN,
                                                          ServiceFlow::SF_TYPE_BE,
                                                          DlClassifierBe32);
  
  DlServiceFlowUgs30.SetSfid(3000);
  DlServiceFlowRtps31.SetSfid(3001);
  DlServiceFlowBe32.SetSfid(3002);

  ss3[0]->AddServiceFlow (DlServiceFlowUgs30);
  ss3[1]->AddServiceFlow (DlServiceFlowRtps31);
  ss3[2]->AddServiceFlow (DlServiceFlowBe32);
  
  AddServiceFlowAtBS(bs3, DlServiceFlowUgs30, 3000, WimaxPhy::MODULATION_TYPE_QAM16_12);
  AddServiceFlowAtBS(bs3, DlServiceFlowRtps31, 3001, WimaxPhy::MODULATION_TYPE_QAM16_12);
  AddServiceFlowAtBS(bs3, DlServiceFlowBe32, 3002, WimaxPhy::MODULATION_TYPE_QAM16_12);
  


  NodeContainer nodes;
  nodes.Create (16);

  Ptr<Node> gw1 = ASNGW_Node1.Get(0);
  Ptr<Node> gw2 = ASNGW_Node2.Get(0);
  Ptr<Node> gw3 = ASNGW_Node3.Get(0);
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

//Ptr<Node> ue11 = ssNodes1.Get(0);
Ptr<Node> ue12 = ssNodes1.Get(1);
//Ptr<Node> ue13 = ssNodes1.Get(2);
//Ptr<Node> ue21 = ssNodes2.Get(0);
Ptr<Node> ue22 = ssNodes2.Get(1);
//Ptr<Node> ue23 = ssNodes2.Get(2);
//Ptr<Node> ue31 = ssNodes3.Get(0);
Ptr<Node> ue32 = ssNodes3.Get(1);
//Ptr<Node> ue33 = ssNodes3.Get(2);


//voice applications
uint16_t sinkPort_ue11 = 100;
Address sinkAddress_ue11 (InetSocketAddress (SSinterfaces1.GetAddress (0), sinkPort_ue11));
VoiceTrafficHelper helper_s11_ue11 ("ns3::UdpSocketFactory",  sinkAddress_ue11);
ApplicationContainer clientApps_s11 = helper_s11_ue11.Install(s11);
clientApps_s11.Start (Seconds (0.2));
clientApps_s11.Stop (Seconds (20));

uint16_t sinkPort_ue21 = 100;
Address sinkAddress_ue21 (InetSocketAddress (SSinterfaces2.GetAddress (0), sinkPort_ue21));
VoiceTrafficHelper helper_s21_ue21 ("ns3::UdpSocketFactory",  sinkAddress_ue21);
ApplicationContainer clientApps_s21 = helper_s21_ue21.Install(s21);
clientApps_s21.Start (Seconds (0.2));
clientApps_s21.Stop (Seconds (20));

uint16_t sinkPort_ue31 = 100;
Address sinkAddress_ue31 (InetSocketAddress (SSinterfaces3.GetAddress (0), sinkPort_ue31));
VoiceTrafficHelper helper_s31_ue31 ("ns3::UdpSocketFactory",  sinkAddress_ue31);
ApplicationContainer clientApps_s31 = helper_s31_ue31.Install(s31);
clientApps_s31.Start (Seconds (0.2));
clientApps_s31.Stop (Seconds (20));

//AR2video applications
/*Ar2VideoHelper helper_s12_ue12 (SSinterfaces1.GetAddress(1),  200);
ApplicationContainer clientApps_s12 = helper_s12_ue12.Install(s12);
clientApps_s12.Start (Seconds (0.2));
clientApps_s12.Stop (Seconds (20));

Ar2VideoHelper helper_s22_ue22 (SSinterfaces2.GetAddress(1),  200);
ApplicationContainer clientApps_s22 = helper_s22_ue22.Install(s22);
clientApps_s22.Start (Seconds (0.2));
clientApps_s22.Stop (Seconds (20));

Ar2VideoHelper helper_s32_ue32 (SSinterfaces3.GetAddress(1),  200);
ApplicationContainer clientApps_s32 = helper_s32_ue32.Install(s32);
clientApps_s32.Start (Seconds (0.2));
clientApps_s32.Stop (Seconds (20));
*/

NS_LOG_INFO ("Adding video traffic: s12->ue12");
 UdpServerHelper server_s12_ue12 (200);
 ApplicationContainer serverApp_s12 = server_s12_ue12.Install (NodeContainer(ue12));
 serverApp_s12.Start (Seconds (0.0));
 serverApp_s12.Stop (Seconds (20.0));
 UdpTraceClientHelper client_s12_ue12 (SSinterfaces1.GetAddress (1), 200, "tracefile.dat");
 client_s12_ue12.SetAttribute ("MaxPacketSize", UintegerValue (1024));
 ApplicationContainer clientApp_ue12 = client_s12_ue12.Install (NodeContainer(s12));
 clientApp_ue12.Start (Seconds (0.0));
 clientApp_ue12.Stop (Seconds (20.0));

NS_LOG_INFO ("Adding video traffic: s22->ue22");
 UdpServerHelper server_s22_ue22 (200);
 ApplicationContainer serverApp_s22 = server_s22_ue22.Install (NodeContainer(ue22));
 serverApp_s22.Start (Seconds (0.0));
 serverApp_s22.Stop (Seconds (20.0));
 UdpTraceClientHelper client_s22_ue22 (SSinterfaces2.GetAddress (1), 200, "tracefile.dat");
 client_s22_ue22.SetAttribute ("MaxPacketSize", UintegerValue (1024));
 ApplicationContainer clientApp_ue22 = client_s22_ue22.Install (NodeContainer(s22));
 clientApp_ue22.Start (Seconds (0.0));
 clientApp_ue22.Stop (Seconds (20.0));

NS_LOG_INFO ("Adding video traffic: s32->ue32");
 UdpServerHelper server_s32_ue32 (200);
 ApplicationContainer serverApp_s32 = server_s32_ue32.Install (NodeContainer(ue32));
 serverApp_s32.Start (Seconds (0.0));
 serverApp_s32.Stop (Seconds (20.0));
 UdpTraceClientHelper client_s32_ue32 (SSinterfaces3.GetAddress (1), 200, "tracefile.dat");
 client_s32_ue32.SetAttribute ("MaxPacketSize", UintegerValue (1024));
 ApplicationContainer clientApp_ue32 = client_s32_ue32.Install (NodeContainer(s32));
 clientApp_ue32.Start (Seconds (0.0));
 clientApp_ue32.Stop (Seconds (20.0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  Simulator::Stop (Seconds (20.0));

  
  pointToPoint.EnablePcapAll("wimax-wired-core");

  NS_LOG_INFO ("Starting simulation.....");
  Simulator::Run ();

  ss1[0] = 0;
  ss1[1] = 0;
  ss1[2] = 0;
  bs1 = 0;

  ss2[0] = 0;
  ss2[1] = 0;
  ss2[2] = 0;
  bs2 = 0;

  ss3[0] = 0;
  ss3[1] = 0;
  ss3[2] = 0;
  bs3 = 0;

  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");

  return 0;
}
