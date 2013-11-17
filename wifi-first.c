/*
*/

#include <iostream>

#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/network-module.h"
#include "ns3/packet-sink.h"
#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE("WIFIAPPLICATIONCODE");

int main(int agrc, char *agrv[])
{

  LogComponentEnable ("OnOffApplication", LOG_LEVEL_INFO);
  uint32_t maxBytes = 131072;
  //std::string AppPacketRate ("0.5Mbps");//??what it mean??
  //Config::SetDefault ("ns3::OnOffApplication::DataRate",  StringValue (AppPacketRate));

  CommandLine cmd;
  cmd.Parse(agrc, agrv);

  NS_LOG_INFO("Start Topology!");
  NodeContainer wifiNodes;
  wifiNodes.Create(6);
  NodeContainer wifiApNode;
  wifiApNode.Create(1);

  NS_LOG_INFO("Set channel!");
  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetChannel (channel.Create ());


  NS_LOG_INFO("Set MAC!");
  //set MAC
  WifiHelper wifi = WifiHelper::Default();
  QosWifiMacHelper mac = QosWifiMacHelper::Default();
  wifi.SetRemoteStationManager ("ns3::AarfWifiManager", "FragmentationThreshold", UintegerValue (2500));

    Ssid ssid = Ssid ("ns-3-802.11n");
    mac.SetType ("ns3::StaWifiMac",
                 "Ssid", SsidValue (ssid),
                 "ActiveProbing", BooleanValue (false));
    mac.SetMsduAggregatorForAc (AC_BE, "ns3::MsduStandardAggregator", 
                                "MaxAmsduSize", UintegerValue (3839));

    NetDeviceContainer staDevices;
    staDevices = wifi.Install (phy, mac, wifiNodes);

    //set AP MAC
    mac.SetType ("ns3::ApWifiMac",
                 "Ssid", SsidValue (ssid));
    mac.SetMsduAggregatorForAc (AC_BE, "ns3::MsduStandardAggregator", 
                                "MaxAmsduSize", UintegerValue (7935));
    NetDeviceContainer apDevice;
    apDevice = wifi.Install (phy, mac, wifiApNode);
 
    NS_LOG_INFO("Set mobility!");
    /* Setting mobility model */
    MobilityHelper mobility;

    mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                   "MinX", DoubleValue (0.0),
                                   "MinY", DoubleValue (0.0),
                                   "DeltaX", DoubleValue (5.0),
                                   "DeltaY", DoubleValue (10.0),
                                   "GridWidth", UintegerValue (3),
                                   "LayoutType", StringValue ("RowFirst"));

    mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                               "Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));
    mobility.Install (wifiNodes);

    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (wifiApNode);

    NS_LOG_INFO("Set InternetStackHelper");
    /* Internet stack*/
    InternetStackHelper stack;
    stack.Install (wifiApNode);
    stack.Install (wifiNodes);

    Ipv4AddressHelper address;

    address.SetBase ("192.168.1.0", "255.255.255.0");
    Ipv4InterfaceContainer wifiNodesInterfaces;
    Ipv4InterfaceContainer apNodeInterface;

    wifiNodesInterfaces = address.Assign (staDevices);
    apNodeInterface = address.Assign (apDevice);


    NS_LOG_INFO("Set Application!");
    /*Setting TCP applications*/
    uint16_t servPort = 500;

    PacketSinkHelper sink ("ns3::TcpSocketFactory",
                         InetSocketAddress (Ipv4Address::GetAny (), servPort));

    ApplicationContainer apps_sink = sink.Install (wifiNodes.Get (0));
    apps_sink.Start (Seconds (0.0));
    apps_sink.Stop (Seconds (16.0));

    for (int i = 0; i <= 5; i++)
    if (i!=1)
    {
      OnOffHelper onoff ("ns3::TcpSocketFactory", InetSocketAddress (wifiNodesInterfaces.GetAddress(1), servPort)); // traffic flows from node[i] to node[0]
      //onoff.SetConstantRate(DataRate(AppPacketRate));
      onoff.SetAttribute ("MaxBytes", UintegerValue(maxBytes));
      ApplicationContainer  apps = onoff.Install(wifiNodes.Get(i));
      //set the Max Bytes of data
      apps.Start(Seconds(0.0));
      apps.Stop(Seconds(16.0));
    }

    phy.EnablePcapAll ("wifi-tcp-ffff", false);
    NS_LOG_INFO("Run Simulator!");
    Simulator::Stop (Seconds (16.0));
    Simulator::Run ();
    Simulator::Destroy ();
    NS_LOG_INFO("Done!");

  return 0;
}