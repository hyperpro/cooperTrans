#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"


//Network topology:
//
//
//
//                 AP              
//             |    |     |     
//           |      |     |  --------------wireless
//           |      |     |
//           n1    n2     n3     
//            |     |     |    
//              |   |    | -------------point To point
//                | |   |
//                 server

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("USER COOPERATION");

void handler(ApplicationContainer server_app, ApplicationContainer wifi_Server_App);

int main (int argc, char *argv[])
{

  NS_LOG_INFO ("The Simulation for User Cooperation");
  //LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
  //LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);
  //read the parser
  CommandLine cmd;
  cmd.Parse (argc, argv);
  int n_nodes = 3;
  double SinkStartTime = 0.0;
  double SinkStopTime = 16.0;


  NS_LOG_INFO ("Create nodes.");

  NodeContainer cellularNode;
  cellularNode.Create(n_nodes);
  NodeContainer cellularApNode;
  cellularApNode.Create(1);


  NodeContainer p2pNodes1;
  p2pNodes1.Add(cellularNode.Get(0));
  p2pNodes1.Create(1);

  NodeContainer p2pNodes2;
  p2pNodes2.Add(cellularNode.Get(1));
  p2pNodes2.Add(p2pNodes1.Get(1));

  NodeContainer p2pNodes3;
  p2pNodes3.Add(cellularNode.Get(2));
  p2pNodes3.Add(p2pNodes1.Get(1));
  
  PointToPointHelper pointToPoint1;
  pointToPoint1.SetDeviceAttribute ("DataRate", StringValue ("25Mbps"));
  pointToPoint1.SetChannelAttribute ("Delay", StringValue ("2ms"));
  NetDeviceContainer p2pDevices1;
  p2pDevices1 = pointToPoint1.Install (p2pNodes1);


  PointToPointHelper pointToPoint2;
  pointToPoint2.SetDeviceAttribute ("DataRate", StringValue ("25Mbps"));
  pointToPoint2.SetChannelAttribute ("Delay", StringValue ("2ms"));
  NetDeviceContainer p2pDevices2;
  p2pDevices2 = pointToPoint2.Install (p2pNodes2);

  PointToPointHelper pointToPoint3;
  pointToPoint3.SetDeviceAttribute ("DataRate", StringValue ("25Mbps"));
  pointToPoint3.SetChannelAttribute ("Delay", StringValue ("2ms"));
  NetDeviceContainer p2pDevices3;
  p2pDevices3 = pointToPoint3.Install (p2pNodes3);



  
  



  // Wifi Channel Model
  NS_LOG_INFO ("Create channels.");

  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  wifiPhy.SetChannel (wifiChannel.Create ());
  WifiHelper wifi = WifiHelper::Default ();
  QosWifiMacHelper mac = QosWifiMacHelper::Default();
  StringValue DataRate;
  DataRate = StringValue("OfdmRate27MbpsBW10MHz");
  Ssid ssid = Ssid ("cellular");
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager","DataMode", DataRate, 
                                  "ControlMode", DataRate);
  mac.SetType ("ns3::StaWifiMac",
                   "Ssid", SsidValue (ssid),
                   "ActiveProbing", BooleanValue (false));
  NetDeviceContainer CellularstaDevs;
  CellularstaDevs = wifi.Install (wifiPhy, mac, cellularNode);
  mac.SetType ("ns3::ApWifiMac",
                   "Ssid", SsidValue (ssid));
  NetDeviceContainer CellularapDevice;
  CellularapDevice = wifi.Install (wifiPhy, mac, cellularApNode);


  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (cellularNode);
  mobility.Install (cellularApNode);


  /* Internet stack*/
  InternetStackHelper stack;


  stack.Install (cellularApNode);
    std::cout<<"fffffffff" <<std::endl;
  stack.Install (cellularNode);
    stack.Install (p2pNodes1.Get(1));
  
  

  // cooperation part

  Ipv4AddressHelper address;
  Ipv4Address addr;



   //cellular IP address

  //cellular node ip allocation
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer cellularNodesInterfaces;
  Ipv4InterfaceContainer cellularAPInterface;
  cellularAPInterface = address.Assign (CellularapDevice);
  cellularNodesInterfaces = address.Assign (CellularstaDevs);

  for(int i = 0; i < n_nodes-1; i++)
  {
   addr = cellularNodesInterfaces.GetAddress(i);
   std::cout << " NO" << i << "\t"<< "CellularstaDevs   "<<addr << std::endl;
  }

   addr=cellularAPInterface.GetAddress(0);
   std::cout << "CellularapDevice   "<<addr << std::endl;


   // cooperate user ip allocation
   address.SetBase ("10.1.2.0", "255.255.255.0");
   Ipv4InterfaceContainer p2pNodes1Interface;
   p2pNodes1Interface = address.Assign(p2pDevices1);
   address.SetBase ("10.1.3.0", "255.255.255.0");
   Ipv4InterfaceContainer p2pNodes2Interface;
   p2pNodes2Interface = address.Assign(p2pDevices2);
   address.SetBase ("10.1.4.0", "255.255.255.0");
   Ipv4InterfaceContainer p2pNodes3Interface;
   p2pNodes3Interface = address.Assign(p2pDevices3);

    //exit(1);
   //application

  NS_LOG_INFO("Set Application!");
  /*Setting TCP applications*/
  
  uint16_t port1 = 4000;
  UdpServerHelper server1 (port1);
  UdpServerHelper server2 (port1);
  UdpServerHelper server3 (port1);
  ApplicationContainer server_app; 
  server_app.Add(server1.Install (p2pNodes1.Get (0)));
  server_app.Add(server2.Install (p2pNodes2.Get (0)));
  server_app.Add(server3.Install (p2pNodes3.Get (0)));
  server_app.Start(Seconds(SinkStartTime));
  server_app.Stop(Seconds(SinkStopTime));

  UdpClientHelper cellular_ap (p2pNodes1Interface.GetAddress (0), port1);
  cellular_ap.SetAttribute ("MaxPackets", UintegerValue (64707202));
  cellular_ap.SetAttribute ("Interval", TimeValue (Time ("0.004")));
  cellular_ap.SetAttribute ("PacketSize", UintegerValue (1500));
  UdpClientHelper cellular_ap1 (p2pNodes2Interface.GetAddress (0), port1);
  cellular_ap1.SetAttribute ("MaxPackets", UintegerValue (64707202));
  cellular_ap1.SetAttribute ("Interval", TimeValue (Time ("0.005")));
  cellular_ap1.SetAttribute ("PacketSize", UintegerValue (1500));
  UdpClientHelper cellular_ap2 (p2pNodes3Interface.GetAddress (0), port1);
  cellular_ap2.SetAttribute ("MaxPackets", UintegerValue (64707202));
  cellular_ap2.SetAttribute ("Interval", TimeValue (Time ("0.002")));
  cellular_ap2.SetAttribute ("PacketSize", UintegerValue (1500));
  ApplicationContainer clientApps;
  clientApps.Add(cellular_ap.Install (p2pNodes1.Get (1)));
  clientApps.Add(cellular_ap1.Install (p2pNodes2.Get (1)));
  clientApps.Add(cellular_ap2.Install (p2pNodes3.Get (1)));
  clientApps.Start (Seconds (SinkStartTime));
  clientApps.Stop (Seconds (SinkStopTime));


  uint16_t port2 = 4001;
  ApplicationContainer wifi_Server_App;
  UdpServerHelper wifi_Server(port2);
  wifi_Server_App.Add(wifi_Server.Install (cellularNode.Get(0)));
  wifi_Server_App.Start (Seconds (SinkStartTime+0.00001));
  wifi_Server_App.Stop (Seconds (SinkStopTime));

  UdpClientHelper wifiNodeCP1(cellularNodesInterfaces.GetAddress (0), port2);
  wifiNodeCP1.SetAttribute ("MaxPackets", UintegerValue (64707202));
  wifiNodeCP1.SetAttribute ("Interval", TimeValue (Time ("0.003")));
  wifiNodeCP1.SetAttribute ("PacketSize", UintegerValue (1500));
  UdpClientHelper wifiNodeCP2(cellularNodesInterfaces.GetAddress (0), port2);
  wifiNodeCP2.SetAttribute ("MaxPackets", UintegerValue (64707202));
  wifiNodeCP2.SetAttribute ("Interval", TimeValue (Time ("0.002")));
  wifiNodeCP2.SetAttribute ("PacketSize", UintegerValue (1500));
  ApplicationContainer wifiNodeCPApps;
  wifiNodeCPApps.Add(wifiNodeCP1.Install (cellularNode.Get (1)));
  wifiNodeCPApps.Add(wifiNodeCP2.Install (cellularNode.Get (2)));

  wifiNodeCPApps.Start (Seconds (SinkStartTime+0.00001));
  wifiNodeCPApps.Stop (Seconds (SinkStopTime));



  
  //Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  double setCallBack = 0.001;
  while (setCallBack<=2)
  {
      Simulator::Schedule(Seconds(setCallBack), &handler, server_app, wifi_Server_App);
      setCallBack += 0.05;
  }
  std::cout<<"fuck!!" <<std::endl;
  Simulator::Stop (Seconds (1.0));
  Simulator::Run ();
  Simulator::Destroy ();
  double totalPacketsThrough = DynamicCast<UdpServer>(server_app.Get (0))->GetReceived ()\
      + DynamicCast<UdpServer>(server_app.Get (1))->GetReceived ()  \
      + DynamicCast<UdpServer>(server_app.Get (2))->GetReceived ();
  std::cout << "totalPacketsThrough: " << totalPacketsThrough <<std::endl;
  std::cout << "recieveThrough:      " << DynamicCast<UdpServer>(server_app.Get (0))->GetReceived () + DynamicCast<UdpServer>(wifi_Server_App.Get (0))->GetReceived ()<< std::endl;
  std::cout << "cellular1:           " << DynamicCast<UdpServer>(server_app.Get (0))->GetReceived ()<<std::endl;
  std::cout << "cellular2:           " << DynamicCast<UdpServer>(server_app.Get (1))->GetReceived ()<<std::endl;
  std::cout << "cellular3:           " << DynamicCast<UdpServer>(server_app.Get (2))->GetReceived ()<<std::endl;
  std::cout << "wifi:                " << DynamicCast<UdpServer>(wifi_Server_App.Get (0))->GetReceived ()<<std::endl;
  std::cout << std::endl;

  return 0;
}

void handler(ApplicationContainer server_app, ApplicationContainer wifi_Server_App)
{
    double totalPacketsThrough = DynamicCast<UdpServer>(server_app.Get (0))->GetReceived ()\
      + DynamicCast<UdpServer>(server_app.Get (1))->GetReceived ()  \
      + DynamicCast<UdpServer>(server_app.Get (2))->GetReceived ();
  std::cout << "totalPacketsThrough: " << totalPacketsThrough <<std::endl;
  std::cout << "recieveThrough:      " << DynamicCast<UdpServer>(server_app.Get (0))->GetReceived () + DynamicCast<UdpServer>(wifi_Server_App.Get (0))->GetReceived ()<< std::endl;
  std::cout << "cellular1:           " << DynamicCast<UdpServer>(server_app.Get (0))->GetReceived ()<<std::endl;
  std::cout << "cellular2:           " << DynamicCast<UdpServer>(server_app.Get (1))->GetReceived ()<<std::endl;
  std::cout << "cellular3:           " << DynamicCast<UdpServer>(server_app.Get (2))->GetReceived ()<<std::endl;
  std::cout << "wifi:                " << DynamicCast<UdpServer>(wifi_Server_App.Get (0))->GetReceived ()<<std::endl;
  std::cout << std::endl;


}
