/*

*/

#include <iostream>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT("WIFI-APPLICATION-CODE");

int main(int agrc, char *agrv[])
{

	CommandLine cmd;
	cmd.Parse(agrc, agrv);

  NS_LOG_INFO("Start Topology!");
	NodeContainer wifiNodes;
	wifiNodes.create(6);
	NodeContainer wifiApNodes;
	wifiApNodes.create(1);

  NS_LOG_INFO("Set channel!");
	//set channel and phy layer
	YansWifiChannelHelper channel = YansWiFiChannelHelper::Deafult();
	//channel.AddPropagationLossModel();
	YansWifiPhyHelper phy = YansWiFiPhyHelper::Deafult();
	phy.setChannel = channel.create();

  NS_LOG_INFO("Set MAC!");
	//set MAC
	WifiHelper wifi = WifiHelpr::Deafult();
	QosWifiMacHelper = QosWifiMacHelper::Deafult();
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

    for (int i = 1; i <= 5; i++)
    {
      OnOffHelper onoff ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAddress(0), port)); // traffic flows from node[i] to node[0]
      onoff.setConstantRate("1Mbps");
      onoff.SetAttribute();
      ApplicationContainer  apps = onoff.Install(node.GetId(i));
      //set the Max Bytes of data = 3MB
      apps.SetAttribute ("MaxBytes", UintegerValue (262144));
      apps.start(Seconds(0.0));
      apps.end(Seconds(16.0));
    }

    NS_LOG_INFO("Run Simulator!");
  	Simulator::Stop (Seconds (16.0));
  	Simulator::Run ();
  	Simulator::Destroy ();
    NS_LOG_INFO("Done!");

	return 0;
}