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
//              AP 10.1.1.1
//             |    |     |
//      10.1.1.2   |   10.1.1.4
//           |  10.1.1.3  |        <------------10.1.1.0
//           |      |     |
//           n1-->n2(AP)<--n3      <------------10.1.2.0
//    10.1.2.2   10.1.2.1  10.1.2.3


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("USER COOPERATION");

/*
static void
SetPosition (Ptr<Node> node, Vector position)
{
  Ptr<MobilityModel> mobility = node->GetObject<MobilityModel> ();
  mobility->SetPosition (position);
}*/


int main (int argc, char *argv[])
{
//
// Enable logging for UdpClient and
//
  LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);

  NS_LOG_UNCOND ("The Simulation for User Cooperation");

  //read the parser
  CommandLine cmd;
  cmd.Parse (argc, argv);

  NS_LOG_INFO ("Create nodes.");

  NodeContainer cellularNode;
  cellularNode.Create(3);

  NodeContainer cellularApNode;
  cellularApNode.Create(1);


  NodeContainer wifiNode;
  wifiNode.Add(cellularNode.Get(0));
  wifiNode.Add(cellularNode.Get(2));

  NodeContainer wifiAPNode;
  wifiAPNode.Add(cellularNode.Get(1));

  // Wifi Channel Model
  NS_LOG_INFO ("Create channels.");

  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  wifiPhy.SetChannel (wifiChannel.Create ());

 // NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();


  WifiHelper wifi = WifiHelper::Default ();
  wifi.SetStandard (WIFI_PHY_STANDARD_80211n_2_4GHZ);
  HtWifiMacHelper mac = HtWifiMacHelper::Default ();
  StringValue DataRate;
  DataRate = StringValue("OfdmRate65MbpsBW20MHz");

  Ssid ssid = Ssid ("celluar");
  //wifi.SetRemoteStationManager ("ns3::ArfWifiManager");

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


  HtWifiMacHelper maccp = HtWifiMacHelper::Default ();

  DataRate = StringValue("OfdmRate65MbpsBW20MHz");
  ssid = Ssid ("user_cp");

 //  setup stas.
  maccp.SetType ("ns3::StaWifiMac",
                   "Ssid", SsidValue (ssid),
                   "ActiveProbing", BooleanValue (false));

  NetDeviceContainer wifiNodeDevice;
  wifiNodeDevice = wifi.Install(wifiPhycp, maccp, wifiNode);
 // wifiNodeDevice.Add(CellularstaDevs.Get(0));
 // wifiNodeDevice.Add(CellularstaDevs.Get(2));

  // setup ap.
  maccp.SetType ("ns3::ApWifiMac",
                   "Ssid", SsidValue (ssid));


  // master user as an AP
  NetDeviceContainer wifiAPNodeDevice;
  wifiAPNodeDevice = wifi.Install (wifiPhycp, maccp, wifiAPNode);
 // wifiAPNodeDevice = wifi.Install(wifiPhy, mac, wifiAPNode);
 // wifiAPNodeDevice.Add(CellularstaDevs.Get(1));


  //mobility
  MobilityHelper mobility;


  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));
/*
  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));
*/
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  mobility.Install (cellularNode);
  mobility.Install (cellularApNode);

  // user cooperation

  mobility.Install (wifiNode);
  mobility.Install (wifiAPNode);
//
//  SetPosition (cellularNode.Get(0), Vector (1.0,0.0,0.0));
//  SetPosition (cellularApNode.Get(0), Vector (0.0,0.0,0.0));
//
//


  /* Internet stack*/
  InternetStackHelper stack;
  stack.Install (cellularApNode);
  stack.Install (cellularNode);

  // cooperation part

//  InternetStackHelper stackcp;
 // stackcp.Install(wifiAPNode);
//  stackcp.Install(wifiNode);



  Ipv4AddressHelper address;
  Ipv4Address addr;



   //cellular IP address

  //cellular node ip allocation
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer cellularNodesInterfaces;
  Ipv4InterfaceContainer cellularAPInterface;

  cellularAPInterface = address.Assign (CellularapDevice);
  cellularNodesInterfaces = address.Assign (CellularstaDevs);

  for(int i = 0; i < 3; i++)
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
   for(int i = 0; i < 2; i++)
   {
    addr = wifiNodeInterfaces.GetAddress(i);
    std::cout << " NO" << i << "\t"<< "wifiNodeDevice   "<<addr << std::endl;
   }

    addr=wifiAPNodeInterface.GetAddress(0);
    std::cout << "wifiAPNodeDevice   "<<addr << std::endl;
   //application

// cellular received sta as server


  ApplicationContainer cellular_server_App;
  UdpServerHelper cellular_sta(8);
  cellular_server_App.Add(cellular_sta.Install (cellularNode.Get (0)));

  UdpServerHelper cellular_sta1(8);
  cellular_server_App.Add(cellular_sta1.Install (cellularNode.Get (1)));

  UdpServerHelper cellular_sta2(8);
  cellular_server_App.Add(cellular_sta2.Install (cellularNode.Get (2)));


  cellular_server_App.Start (Seconds (0.0));
  cellular_server_App.Stop (Seconds (0.1));



// cellular ap app

  UdpClientHelper cellular_ap[3] = {UdpClientHelper(cellularNodesInterfaces.GetAddress(0), 8),\
		                            UdpClientHelper(cellularNodesInterfaces.GetAddress(1), 8),\
		                            UdpClientHelper(cellularNodesInterfaces.GetAddress(2), 8)};


  for(int i = 0; i < 3; i++)
  {
	 cellular_ap[i].SetAttribute ("MaxPackets", UintegerValue (64707202));
	 cellular_ap[i].SetAttribute ("Interval", TimeValue (Time ("0.00002")));
	 cellular_ap[i].SetAttribute ("PacketSize", UintegerValue (1500));
  }
/*
  UdpClientHelper cellular_ap (cellularNodesInterfaces.GetAddress (0), 8);
  cellular_ap.SetAttribute ("MaxPackets", UintegerValue (64707202));
  cellular_ap.SetAttribute ("Interval", TimeValue (Time ("0.00002")));
  cellular_ap.SetAttribute ("PacketSize", UintegerValue (1500));

  UdpClientHelper cellular_ap1 (cellularNodesInterfaces.GetAddress (1), 8);
  cellular_ap1.SetAttribute ("MaxPackets", UintegerValue (64707202));
  cellular_ap1.SetAttribute ("Interval", TimeValue (Time ("0.00002")));
  cellular_ap1.SetAttribute ("PacketSize", UintegerValue (1500));

  UdpClientHelper cellular_ap2 (cellularNodesInterfaces.GetAddress (2), 8);
  cellular_ap2.SetAttribute ("MaxPackets", UintegerValue (64707202));
  cellular_ap2.SetAttribute ("Interval", TimeValue (Time ("0.00002")));
  cellular_ap2.SetAttribute ("PacketSize", UintegerValue (1500));

  ApplicationContainer clientApps;
  clientApps.Add(cellular_ap.Install (cellularApNode.Get (0)));
  clientApps.Add(cellular_ap1.Install (cellularApNode.Get (0)));
  clientApps.Add(cellular_ap2.Install (cellularApNode.Get (0)));
*/

  ApplicationContainer clientApps;
  for(int i = 0; i < 3; i++)
  {
	 clientApps.Add(cellular_ap[i].Install (cellularApNode.Get (0)));
  }

  clientApps.Start (Seconds (0.0));
  clientApps.Stop (Seconds (0.1));


/*
// wifi share
  ApplicationContainer wifiAP_server_App;
  UdpServerHelper wifiAP_server(10);
  wifiAP_server_App.Add(wifiAP_server.Install (wifiAPNode.Get (0)));


  wifiAP_server_App.Start (Seconds (0.0));
  wifiAP_server_App.Stop (Seconds (0.2));



  UdpClientHelper wifiNodeCP[2]={UdpClientHelper (wifiAPNodeInterface.GetAddress (0), 10), \
		                         UdpClientHelper (wifiAPNodeInterface.GetAddress (0), 10)};

  for(int i = 0; i < 2; i++)
  {
   wifiNodeCP[i].SetAttribute ("MaxPackets", UintegerValue (64707202));
   wifiNodeCP[i].SetAttribute ("Interval", TimeValue (Time ("0.00002")));
   wifiNodeCP[i].SetAttribute ("PacketSize", UintegerValue (1500));
  }






  ApplicationContainer wifiNodeCPApps;
  wifiNodeCPApps.Add(wifiNodeCP[0].Install (wifiNode.Get (0)));
  wifiNodeCPApps.Add(wifiNodeCP[1].Install (wifiNode.Get (1)));


  wifiNodeCPApps.Start (Seconds (0.0));
  wifiNodeCPApps.Stop (Seconds (0.2));
*/


  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Simulator::Stop (Seconds (0.5));
  Simulator::Run ();
  Simulator::Destroy ();

  uint32_t totalPacketsThrough = 0;
  totalPacketsThrough = DynamicCast<UdpServer>(cellular_server_App.Get (0))->GetReceived ()\
		  + DynamicCast<UdpServer>(cellular_server_App.Get (1))->GetReceived ()  \
		  + DynamicCast<UdpServer>(cellular_server_App.Get (2))->GetReceived ();
  double throughput=totalPacketsThrough*1500*8/(0.2*1000000.0);
  std::cout<< "  totalPacketsThrough :  " << totalPacketsThrough << std::endl;
  double datarate = 65;

  std::cout << "DataRate" <<"  " << "Throughput" << '\n';
  std::cout << datarate <<"     \t " << throughput << '\n';


  totalPacketsThrough = DynamicCast<UdpServer>(cellular_server_App.Get (0))->GetReceived ();
  std::cout<< "user B  totalPacketsThrough :  " << totalPacketsThrough << std::endl;

  totalPacketsThrough = DynamicCast<UdpServer>(cellular_server_App.Get (1))->GetReceived ();
  std::cout<< "user A  totalPacketsThrough :  " << totalPacketsThrough << std::endl;

  totalPacketsThrough = DynamicCast<UdpServer>(cellular_server_App.Get (2))->GetReceived ();
  std::cout<< "user C  totalPacketsThrough :  " << totalPacketsThrough << std::endl;
/*
  uint32_t total_WIFIAP_PacketsThrough = DynamicCast<UdpServer>(wifiAP_server_App.Get (0))->GetReceived ();

  std::cout<< "Master user  totalPacketsThrough :  " << total_WIFIAP_PacketsThrough << std::endl;


  double wifiap_throughput=total_WIFIAP_PacketsThrough*1500*8/(0.2*1000000.0);

  std::cout << "DataRate" <<"  " << "Master user Throughput" << '\n';
  std::cout << "   65 " <<"     \t " << wifiap_throughput << '\n';
*/
  return 0;
}
