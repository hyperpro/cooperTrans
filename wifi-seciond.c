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
  //uint32_t maxBytes[4] = {196608,196608,196608,196608};
  //uint32_t maxBytes1[4] = {196608,196608,196608,196608};

  NS_LOG_INFO ("Create nodes.");

  NodeContainer cellularNode;
  cellularNode.Create(n_nodes);

  NodeContainer cellularApNode;
  cellularApNode.Create(1);


  NodeContainer wifiNode;
  wifiNode.Add(cellularNode.Get(0));
  wifiNode.Add(cellularNode.Get(1));
  NodeContainer wifiAPNode;
  wifiAPNode.Add(cellularNode.Get(2));

  // Wifi Channel Model
  NS_LOG_INFO ("Create channels.");

  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  wifiPhy.SetChannel (wifiChannel.Create ());

  //mac1:cellular
  WifiHelper wifi = WifiHelper::Default ();
  QosWifiMacHelper mac = QosWifiMacHelper::Default();
  StringValue DataRate;
  DataRate = StringValue("OfdmRate27MbpsBW10MHz");

  Ssid ssid = Ssid ("cellular");


  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager","DataMode", DataRate, 
                                  "ControlMode", DataRate);
 //  setup stas.
  mac.SetType ("ns3::StaWifiMac",
                   "Ssid", SsidValue (ssid),
                   "ActiveProbing", BooleanValue (false));

  //cellular sta dev
  NetDeviceContainer CellularstaDevs;
  CellularstaDevs = wifi.Install (wifiPhy, mac, cellularNode);

  // setup ap.
  mac.SetType ("ns3::ApWifiMac",
                   "Ssid", SsidValue (ssid));


  //celluar ap dev
  NetDeviceContainer CellularapDevice;
  CellularapDevice = wifi.Install (wifiPhy, mac, cellularApNode);


// cooperation user

  YansWifiPhyHelper wifiPhycp = YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannelcp = YansWifiChannelHelper::Default ();
  wifiPhycp.SetChannel (wifiChannelcp.Create ());
  QosWifiMacHelper maccp = QosWifiMacHelper::Default();

  DataRate = StringValue("OfdmRate65MbpsBW20MHz");
  ssid = Ssid ("user_cp");

 //  setup stas.
  maccp.SetType ("ns3::StaWifiMac",
                   "Ssid", SsidValue (ssid),
                   "ActiveProbing", BooleanValue (false));

  NetDeviceContainer wifiNodeDevice;
  wifiNodeDevice = wifi.Install(wifiPhycp, maccp, wifiNode);

  // setup ap.
  maccp.SetType ("ns3::ApWifiMac",
                   "Ssid", SsidValue (ssid));


  // master user as an AP
  NetDeviceContainer wifiAPNodeDevice;
  wifiAPNodeDevice = wifi.Install (wifiPhycp, maccp, wifiAPNode);
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  mobility.Install (cellularNode);
  mobility.Install (cellularApNode);

  // user cooperation

  mobility.Install (wifiNode);
  mobility.Install (wifiAPNode);


  /* Internet stack*/
  InternetStackHelper stack;
  stack.Install (cellularApNode);
  stack.Install (cellularNode);

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

  for(int i = 0; i < n_nodes; i++)
  {
   addr = cellularNodesInterfaces.GetAddress(i);
   std::cout << " NO" << i << "\t"<< "CellularstaDevs   "<<addr << std::endl;
  }

   addr=cellularAPInterface.GetAddress(0);
   std::cout << "CellularapDevice   "<<addr << std::endl;


   // cooperate user ip allocation
   address.SetBase ("10.1.2.0", "255.255.255.0");

   Ipv4InterfaceContainer wifiAPNodeInterface;
   Ipv4InterfaceContainer wifiNodeInterfaces;

   wifiAPNodeInterface = address.Assign(wifiAPNodeDevice);
   wifiNodeInterfaces = address.Assign(wifiNodeDevice);
   for(int i = 0; i < n_nodes-1; i++)
   {
    addr = wifiNodeInterfaces.GetAddress(i);
    std::cout << " NO" << i << "\t"<< "wifiNodeDevice   "<<addr << std::endl;
   }

    addr=wifiAPNodeInterface.GetAddress(0);
    std::cout << "wifiAPNodeDevice   "<<addr << std::endl;
    //exit(1);
   //application

  NS_LOG_INFO("Set Application!");
  /*Setting TCP applications*/
  
  uint16_t port1 = 4000;
  UdpServerHelper server1 (port1);
  UdpServerHelper server2 (port1);
  UdpServerHelper server3 (port1);
  ApplicationContainer server_app; 
  server_app.Add(server1.Install (cellularNode.Get (0)));
  server_app.Add(server2.Install (cellularNode.Get (1)));
  server_app.Add(server3.Install (cellularNode.Get (2)));
  server_app.Start(Seconds(SinkStartTime));
  server_app.Stop(Seconds(SinkStopTime));

  UdpClientHelper cellular_ap (cellularNodesInterfaces.GetAddress (0), port1);
  cellular_ap.SetAttribute ("MaxPackets", UintegerValue (64707202));
  cellular_ap.SetAttribute ("Interval", TimeValue (Time ("0.00004")));
  cellular_ap.SetAttribute ("PacketSize", UintegerValue (1500));
  UdpClientHelper cellular_ap1 (cellularNodesInterfaces.GetAddress (1), port1);
  cellular_ap1.SetAttribute ("MaxPackets", UintegerValue (64707202));
  cellular_ap1.SetAttribute ("Interval", TimeValue (Time ("0.00005")));
  cellular_ap1.SetAttribute ("PacketSize", UintegerValue (1500));
  UdpClientHelper cellular_ap2 (cellularNodesInterfaces.GetAddress (2), port1);
  cellular_ap2.SetAttribute ("MaxPackets", UintegerValue (64707202));
  cellular_ap2.SetAttribute ("Interval", TimeValue (Time ("0.00002")));
  cellular_ap2.SetAttribute ("PacketSize", UintegerValue (1500));
  ApplicationContainer clientApps;
  clientApps.Add(cellular_ap.Install (cellularApNode.Get (0)));
  clientApps.Add(cellular_ap1.Install (cellularApNode.Get (0)));
  clientApps.Add(cellular_ap2.Install (cellularApNode.Get (0)));
  clientApps.Start (Seconds (SinkStartTime));
  clientApps.Stop (Seconds (SinkStopTime));


  uint16_t port2 = 4001;
  ApplicationContainer wifi_Server_App;
  UdpServerHelper wifi_Server(port2);
  wifi_Server_App.Add(wifi_Server.Install (wifiAPNode.Get(0)));
  wifi_Server_App.Start (Seconds (SinkStartTime+0.00001));
  wifi_Server_App.Stop (Seconds (SinkStopTime));

  UdpClientHelper wifiNodeCP1(wifiAPNodeInterface.GetAddress (0), port2);
  wifiNodeCP1.SetAttribute ("MaxPackets", UintegerValue (64707202));
  wifiNodeCP1.SetAttribute ("Interval", TimeValue (Time ("0.00003")));
  wifiNodeCP1.SetAttribute ("PacketSize", UintegerValue (1500));
  UdpClientHelper wifiNodeCP2(wifiAPNodeInterface.GetAddress (0), port2);
  wifiNodeCP2.SetAttribute ("MaxPackets", UintegerValue (64707202));
  wifiNodeCP2.SetAttribute ("Interval", TimeValue (Time ("0.00002")));
  wifiNodeCP2.SetAttribute ("PacketSize", UintegerValue (1500));
  ApplicationContainer wifiNodeCPApps;
  wifiNodeCPApps.Add(wifiNodeCP1.Install (wifiNode.Get (1)));
  wifiNodeCPApps.Add(wifiNodeCP2.Install (wifiNode.Get (2)));

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
