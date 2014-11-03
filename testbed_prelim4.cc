#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/sim-tags.h"
#include "ns3/flow-monitor-module.h"
#include <iostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Testbed_Prelim4");

Ptr<Ipv4FlowClassifier> flow_classifier = new Ipv4FlowClassifier();

double t_on = 0.8;
double t_off = 0.2;
double p_shape = 1.2;

class TimeQueueSizePkts{
  public:
	TimeQueueSizePkts(Time t, uint32_t s): time(t), size(s) {};
	TimeQueueSizePkts(const TimeQueueSizePkts& c): time(c.time), size(c.size) {}
  public:
	Time time;
	uint32_t size;
};

typedef std::vector<TimeQueueSizePkts> TimeQueueSizePkts_t;



class TimeDelay{
  public:
	TimeDelay(Time t, Time in, Time out, uint32_t s ): time(t), timeIn(in), timeOut(out), seqno(s) {};
	TimeDelay(const TimeDelay& c): time(c.time), timeIn(c.timeIn), timeOut(c.timeOut), seqno(c.seqno) {}
  public:
	Time time;
        Time timeIn;
        Time timeOut;
	uint32_t seqno;
};

typedef std::vector<TimeDelay> TimeE2EDelay_t;
typedef std::vector<TimeDelay> TimeQueueDelay_t;
typedef std::map<FlowId, TimeE2EDelay_t*> TimeE2EDelayByFlow_t;
typedef std::map<FlowId, TimeQueueDelay_t*> TimeQueueDelayByFlow_t;

class TimeDrop{
  public:
	TimeDrop(Time t, uint32_t d): time(t), num_drop(d) {};
	TimeDrop(const TimeDrop& c): time(c.time), num_drop(c.num_drop) {}
  public:
	Time time;
	uint32_t num_drop;
};

typedef std::vector<TimeDrop> TimeQueueDrop_t;
typedef std::map<FlowId, TimeQueueDrop_t*> TimeQueueDropByFlow_t;


TimeQueueSizePkts_t* timeQueueSizePkts = new TimeQueueSizePkts_t;
TimeE2EDelay_t* timeE2EDelay = new TimeE2EDelay_t;
TimeQueueDelay_t* timeQueueDelay = new TimeQueueDelay_t;
TimeQueueDrop_t* timeQueueDrop = new TimeQueueDrop_t;
TimeE2EDelayByFlow_t* timeE2EDelayByFlow = new TimeE2EDelayByFlow_t;
TimeQueueDelayByFlow_t* timeQueueDelayByFlow = new TimeQueueDelayByFlow_t;
TimeQueueDropByFlow_t* timeQueueDropByFlow = new TimeQueueDropByFlow_t;

Queue* mQueue;
uint32_t seqNo=0;

void SetQueue(Queue* q)
{
  mQueue = q;
}




void LogQueueSize(TimeQueueSizePkts_t* ts, std::ostream& os)
{
   for (TimeQueueSizePkts_t::size_type i = 0; i < ts->size(); i++)
        {
          os << (*ts)[i].time << "\t";
          uint32_t sz = (*ts)[i].size;
          os << sz << endl;
        }
}


void LogE2EDelay(TimeE2EDelay_t* e2e, std::ostream& os)
{
   for (TimeE2EDelay_t::size_type i = 0; i < e2e->size(); i++)
        {
          os << (*e2e)[i].time << "\t";
          Time tIn = (*e2e)[i].timeIn;
          Time tOut = (*e2e)[i].timeOut;
          uint32_t s = (*e2e)[i].seqno;
          os << tIn << "\t" << tOut << "\t" << s <<endl;
        }
}

void LogQueueDelay(TimeQueueDelay_t* e2e, std::ostream& os)
{
   for (TimeQueueDelay_t::size_type i = 0; i < e2e->size(); i++)
        {
          os << (*e2e)[i].time << "\t";
          Time tIn = (*e2e)[i].timeIn;
          Time tOut = (*e2e)[i].timeOut;
          uint32_t s = (*e2e)[i].seqno;
          os << tIn << "\t" << tOut << "\t" << s <<endl;
        }
}

void LogQueueDrop(TimeQueueDrop_t* ts, std::ostream& os)
{
   for (TimeQueueDrop_t::size_type i = 0; i < ts->size(); i++)
        {
          os << (*ts)[i].time << "\t";
          uint32_t nd = (*ts)[i].num_drop;
          os << nd << endl;
        }
}


int tx = 0;
static void
Transmit(Ptr<const Packet> packet)
{
  tx++;
  //std::cout << "Transmit at Phy function called. Item " << tx << ".\n";
   NS_LOG_INFO("Transmit at Phy function called. Item " << tx);
   Time t = Simulator::Now();
   Time::Unit tu = Time::US;
   double dtime = t.ToDouble(tu);
    
   ++seqNo;
  

   TimeTxTag timetx;
   timetx.SetTimeTx(dtime);

   SeqNoTag sTag;
   sTag.SetSeqNo(seqNo);

   packet->AddPacketTag(timetx);
   packet->AddPacketTag(sTag);
}

int rx = 0;
static void
Receive(Ptr<const Packet> packet)
{
  rx++;
  //std::cout << "Receive at Phy function called. Item " << rx << ".\n";
  NS_LOG_INFO("Receive at Phy function called. Item " << rx);
  Time t = Simulator::Now();
  
  TimeTxTag txTag;
  packet->PeekPacketTag (txTag);

  SeqNoTag s;
  packet->PeekPacketTag (s);

  Ipv4FlowTag ftag; 
  packet->PeekPacketTag (ftag);
  
  FlowId flowid = ftag.GetFlowId();

  Time::Unit tu = Time::US;
  Time rtime = Time::FromDouble(txTag.GetTimeTx(), tu);
  timeE2EDelay->push_back(TimeDelay(t, rtime, t,  s.GetSeqNo()));

  TimeE2EDelayByFlow_t::iterator iter;
  iter=timeE2EDelayByFlow->find(flowid);
  if (iter == timeE2EDelayByFlow->end())
    {
      TimeE2EDelay_t* new_entry = new TimeE2EDelay_t;
      std::pair<TimeE2EDelayByFlow_t::iterator, bool> srch = timeE2EDelayByFlow->insert(std::pair<FlowId, TimeE2EDelay_t*>(flowid, new_entry));
      if(srch.second)
        {
          srch.first->second->push_back(TimeDelay(t, rtime, t,  s.GetSeqNo()));
        }
      else
        {
          cout<<"There is a problem with insert.\n";
        }
    }
  else
    {
      iter->second->push_back(TimeDelay(t, rtime, t,  s.GetSeqNo()));
    }

}



int enqu = 0;
static void
Enqueue(Ptr<const Packet> packet)
{
  enqu++;
  Time t = Simulator::Now();
  Time::Unit tu = Time::US;
  double dtime = t.ToDouble(tu);

  TimeInQTag tinQ;
  tinQ.SetTimeInQ(dtime);
  packet->AddPacketTag(tinQ);


  std::cout << "Enqueue function called. Item " << enqu << ".\n";
  timeQueueSizePkts->push_back(TimeQueueSizePkts(Simulator::Now(), mQueue->GetNPackets()));
}

int dequ=0;
static void
Dequeue(Ptr<const Packet> packet)
{
  dequ++;
  Time t = Simulator::Now();
  
  TimeInQTag tinQ;
  packet->PeekPacketTag (tinQ);

  SeqNoTag s;
  packet->PeekPacketTag (s);

  Ipv4FlowTag ftag; 
  packet->PeekPacketTag (ftag);
  
  FlowId flowid = ftag.GetFlowId();

  Time::Unit tu = Time::US;
  Time rtime = Time::FromDouble(tinQ.GetTimeInQ(), tu);
  timeQueueDelay->push_back(TimeDelay(t, rtime, t,  s.GetSeqNo()));

  TimeQueueDelayByFlow_t::iterator iter;
  iter=timeQueueDelayByFlow->find(flowid);
  if (iter == timeQueueDelayByFlow->end())
    {
      TimeQueueDelay_t* new_entry = new TimeQueueDelay_t;
      std::pair<TimeQueueDelayByFlow_t::iterator, bool> srch = timeQueueDelayByFlow->insert(std::pair<FlowId, TimeQueueDelay_t*>(flowid, new_entry));
      if(srch.second)
        {
          srch.first->second->push_back(TimeDelay(t, rtime, t,  s.GetSeqNo()));
        }
      else
        {
          cout<<"There is a problem with insert.\n";
        }
    }
  else
    {
      iter->second->push_back(TimeDelay(t, rtime, t,  s.GetSeqNo()));
    }
  
  std::cout << "Dequeue function called. Item " << dequ << ".\n";
  timeQueueSizePkts->push_back(TimeQueueSizePkts(Simulator::Now(), mQueue->GetNPackets()));
}

int qDrops=0;
static void
Drop(Ptr<const Packet> packet)
{
  qDrops++;
  Time t = Simulator::Now();

  SeqNoTag s;
  packet->PeekPacketTag (s);

  Ipv4FlowTag ftag; 
  packet->PeekPacketTag (ftag);
  
  FlowId flowid = ftag.GetFlowId();

  TimeQueueDropByFlow_t::iterator iter;
  iter=timeQueueDropByFlow->find(flowid);
  if (iter == timeQueueDropByFlow->end())
    {
      TimeQueueDrop_t* new_entry = new TimeQueueDrop_t;
      std::pair<TimeQueueDropByFlow_t::iterator, bool> srch = timeQueueDropByFlow->insert(std::pair<FlowId, TimeQueueDrop_t*>(flowid, new_entry));
      if(srch.second)
        {
          srch.first->second->push_back(TimeDrop(t, s.GetSeqNo()));
        }
      else
        {
          cout<<"There is a problem with insert.\n";
        }
    }
  else
    {
      iter->second->push_back(TimeDrop(t, s.GetSeqNo()));
    }

  std::cout << "Drop function called. Item " << qDrops << ".\n";
  timeQueueDrop->push_back(TimeDrop(t, mQueue->GetTotalDroppedPackets()));
//uint32_t GetTotalDroppedPackets (void) const;
}

static void
Ipv4Transmit(const Ipv4Header& ipHeader, Ptr<const Packet> packet, uint32_t v)
{
  //The following piece of code was adopted from src/flow-monitor/model/ipv4-flow-probe.cc lines 195-209
  FlowId flowId;

  FlowPacketId packetId;
  
  if (flow_classifier->Classify (ipHeader, packet, &flowId, &packetId))
    {
      uint32_t size = (packet->GetSize () + ipHeader.GetSerializedSize ());
      
      Ipv4FlowTag fTag (flowId, packetId, size);
      packet->AddPacketTag (fTag);
      NS_LOG_INFO("Flow ID: " << flowId << ", packetId: " << packetId <<" with packet size: " << size);
    }
}


int
main (int argc, char *argv[])
{
  //LogComponentEnable ("OnOffApplication", LOG_LEVEL_INFO);
  //LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);
  //LogComponentEnable ("Ipv4GlobalRouting", LOG_LEVEL_INFO);
  LogComponentEnable("Testbed_Prelim3", LOG_LEVEL_INFO);

  DataRate dataRate = 10000000 ;

  CommandLine cmd;

  cmd.AddValue("DataRate", "Rate of Bottleneck Link", dataRate);

  cmd.Parse(argc, argv);

  NodeContainer nodes;
  nodes.Create (6);
  
  NS_LOG_INFO ("Creating nodes.");
  Ptr<Node> r1 = nodes.Get(0);
  Ptr<Node> r2 = nodes.Get(1);
  Ptr<Node> h1 = nodes.Get(2);
  Ptr<Node> h3 = nodes.Get(3);
  Ptr<Node> h2 = nodes.Get(4);
  Ptr<Node> h4 = nodes.Get(5);
  

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("200ms"));

  NetDeviceContainer devices_r1_r2;
  NetDeviceContainer devices_h1_r1;
  NetDeviceContainer devices_r2_h3;
  NetDeviceContainer devices_h2_r1;
  NetDeviceContainer devices_r2_h4;
  
  NS_LOG_INFO ("Creating point-to-poimt connections between network nodes.");
  devices_r1_r2 = pointToPoint.Install (r1,r2);
  devices_h1_r1 = pointToPoint.Install (h1,r1);
  devices_r2_h3 = pointToPoint.Install (r2,h3);
  devices_h2_r1 = pointToPoint.Install (h2,r1);
  devices_r2_h4 = pointToPoint.Install (r2,h4);
 
  NS_LOG_INFO ("Installing the Internet stack on the nodes.");
  InternetStackHelper stack;
  stack.Install (nodes);

  NS_LOG_INFO ("Assigning IPv4 network addresses to the netdevices.");
  Ipv4AddressHelper address;
  address.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces_r1_r2 = address.Assign (devices_r1_r2);
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces_h1_r1 = address.Assign (devices_h1_r1);
  address.SetBase ("10.1.4.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces_r2_h3 = address.Assign (devices_r2_h3);
  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces_h2_r1 = address.Assign (devices_h2_r1);
  address.SetBase ("10.1.5.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces_r2_h4 = address.Assign (devices_r2_h4);

  devices_r1_r2.Get(0)->GetObject<PointToPointNetDevice>()->SetDataRate (dataRate);
  NS_LOG_INFO ("Data rate of bottleneck link is " << dataRate);
  Ptr<Queue> que = devices_r1_r2.Get(0)->GetObject<PointToPointNetDevice>()->GetQueue()->GetObject<DropTailQueue>();
  Queue* ptpq = GetPointer<Queue>(que); 
  //ptpnd = dynamic_cast<Ptr<PointToPointNetDevice>>(nd);
  SetQueue(ptpq);

  NS_LOG_INFO ("Connecting trace sinks to trace sources.");
  //Config::Connect("/NodeList/3/DeviceList/1/$ns3::NetDevice/PhyTxBegin", MakeCallback(&Transmit));
  //Config::Connect("/NodeList/4/DeviceList/1/$ns3::NetDevice/PhyRxEnd", MakeCallback(&Receive));
  //Config::Connect("/NodeList/3/DeviceList/1/$ns3::NetDevice/MacTx", MakeCallback(&TagPacket));
  //Config::Connect("/NodeList/1/DeviceList/2/$ns3::PointToPointNetDevice/TxQueue/$ns3::DropTailQueue/Enqueue", MakeCallback(&Enqueue));
  //Config::Connect("/NodeList/1/DeviceList/2/$ns3::PointToPointNetDevice/TxQueue/$ns3::DropTailQueue/Dequeue", MakeCallback(&Dequeue));
  //Config::Connect("/NodeList/1/DeviceList/2/$ns3::PointToPointNetDevice/TxQueue/$ns3::DropTailQueue/Drop", MakeCallback(&Count));
  devices_h1_r1.Get(0)->TraceConnectWithoutContext ("PhyTxBegin", MakeCallback (&Transmit));
  devices_r2_h3.Get(1)->TraceConnectWithoutContext ("PhyRxEnd", MakeCallback (&Receive));
  devices_h2_r1.Get(0)->TraceConnectWithoutContext ("PhyTxBegin", MakeCallback (&Transmit));
  devices_r2_h4.Get(1)->TraceConnectWithoutContext ("PhyRxEnd", MakeCallback (&Receive));
  que->TraceConnectWithoutContext ("Enqueue", MakeCallback (&Enqueue));
  que->TraceConnectWithoutContext ("Dequeue", MakeCallback (&Dequeue));
  que->TraceConnectWithoutContext ("Drop", MakeCallback (&Drop));

  Ptr<Ipv4> ipv4_h1 = h1->GetObject<Ipv4>();
  ipv4_h1->TraceConnectWithoutContext ("SendOutgoing", MakeCallback (&Ipv4Transmit));
  Ptr<Ipv4> ipv4_h2 = h2->GetObject<Ipv4>();
  ipv4_h2->TraceConnectWithoutContext ("SendOutgoing", MakeCallback (&Ipv4Transmit));

  NS_LOG_INFO ("Creating the applications on the host.");
  UdpServerHelper server (5112);

  ApplicationContainer serverApp = server.Install (NodeContainer(h3));
  serverApp.Start (Seconds (0.0));
  serverApp.Stop (Seconds (10.0));

  UdpTraceClientHelper client (interfaces_r2_h3.GetAddress (1), 5112, "tracefile.dat");
  client.SetAttribute ("MaxPacketSize", UintegerValue (1024));


  ApplicationContainer clientApp = client.Install (NodeContainer(h1));
  clientApp.Start (Seconds (0.0));
  clientApp.Stop (Seconds (10.0));

  OnOffHelper onoff ("ns3::UdpSocketFactory", Address (InetSocketAddress (interfaces_r2_h4.GetAddress (1), 9)));
  onoff.SetAttribute ("OnTime", RandomVariableValue (ParetoVariable (t_on, p_shape)));
  onoff.SetAttribute ("OffTime", RandomVariableValue (ParetoVariable (t_off, p_shape)));
  onoff.SetAttribute ("PacketSize", StringValue ("1024"));
  onoff.SetAttribute ("DataRate", StringValue ("125Mb/s"));

  ApplicationContainer apps = onoff.Install (h2);
  apps.Start (Seconds (0.1));
  apps.Stop (Seconds (10.0));

  // Create a packet sink to receive these packets
  PacketSinkHelper sink ("ns3::UdpSocketFactory",InetSocketAddress (Ipv4Address::GetAny (), 9));
  apps = sink.Install (h4);
  apps.Start (Seconds (0.0));

  NS_LOG_INFO ("Routing being configured through the network.");
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  pointToPoint.EnablePcapAll("Testbed_prelim4");

  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Simulation Complete");

  string nm = string("Router1_queue_trajectory.txt");
  ofstream f(nm.c_str());
  LogQueueSize(timeQueueSizePkts, f);
  f.close();

  string nm1 = string("e2e_delay.txt");
  ofstream f1(nm1.c_str());
  LogE2EDelay(timeE2EDelay, f1);
  f1.close();

  string nm2 = string("Router1_queue_delay.txt");
  ofstream f2(nm2.c_str());
  LogQueueDelay(timeQueueDelay, f2);
  f2.close();

  string nm3 = string("Router1_queue_drops.txt");
  ofstream f3(nm3.c_str());
  LogQueueDrop(timeQueueDrop, f3);
  f3.close();
  
  for(TimeE2EDelayByFlow_t::const_iterator i=timeE2EDelayByFlow->begin(); i!=timeE2EDelayByFlow->end(); ++i)
    {
      Ipv4FlowClassifier::FiveTuple t = flow_classifier->FindFlow(i->first);
      cout<<"Flow " << i->first << " (" << t.sourceAddress << "(" <<t.sourcePort << ")"<< "->" << t.destinationAddress << "(" <<t.destinationPort << ")"<< ")\n";
      string flow_name = static_cast<ostringstream*>( &(ostringstream() << i->first) )->str();
      string nm4 = string("Flow")+flow_name+ string("_") + nm1;
      ofstream f4(nm4.c_str());
      LogE2EDelay(i->second, f4);
      f4.close();
    }

  for(TimeQueueDelayByFlow_t::const_iterator i=timeQueueDelayByFlow->begin(); i!=timeQueueDelayByFlow->end(); ++i)
    {
      string flow_name = static_cast<ostringstream*>( &(ostringstream() << i->first) )->str();
      string nm5 = string("Flow")+flow_name+ string("_") + nm2;
      ofstream f5(nm5.c_str());
      LogQueueDelay(i->second, f5);
      f5.close();
    }

   for(TimeQueueDropByFlow_t::const_iterator i=timeQueueDropByFlow->begin(); i!=timeQueueDropByFlow->end(); ++i)
    {
      string flow_name = static_cast<ostringstream*>( &(ostringstream() << i->first) )->str();
      string nm6 = string("Flow")+flow_name+ string("_") + nm3;
      ofstream f6(nm6.c_str());
      LogQueueDrop(i->second, f6);
      f6.close();
    }
 

  return 0;
}

