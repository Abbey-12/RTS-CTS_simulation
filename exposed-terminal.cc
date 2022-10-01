#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/mobility-module.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/wifi-module.h"
#include "ns3/propagation-module.h"
#include "ns3/fd-net-device-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-flow-classifier.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE("exposed-terminal");

// read sent data
 static void 
Tx (Ptr<OutputStreamWrapper> stream,Ptr<const Packet> pkt)
{
 *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << pkt->GetSize() << std::endl;
  
}

// read received data
static void
Rx (Ptr<OutputStreamWrapper> stream, Ptr<const Packet> p, const Address &addr)
{
  *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << p->GetSize() << std::endl;
}

int main(int argc, char *argv[])
{
  bool verbose = true;
  bool rtscts = true;
  double datarate = 1.0;
  int packetsize = 1040;
  

   CommandLine cmd(__FILE__);

  cmd.AddValue("verbose", "Enable logging", verbose);
  cmd.AddValue("datarate", "Data Rate Mbps", datarate);
  cmd.AddValue("packetsize", "Packet size", packetsize);
  cmd.Parse(argc, argv);
  Time::SetResolution(Time::NS);

    if (verbose) {
        LogComponentEnable("OnOffApplication", LOG_LEVEL_INFO);
    }

    UintegerValue thr = rtscts ? UintegerValue(100) : UintegerValue(10000);

    Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold",thr);


// creating nodes 
    NodeContainer apNodes;
    apNodes.Create(2);
    NodeContainer staNodes;
    staNodes.Create(2);

// position of nodes
    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                    "MinX", DoubleValue(0.0),
                                    "MinY", DoubleValue(0.0),
                                    "DeltaX", DoubleValue(150.0),
                                    "DeltaY", DoubleValue(0.0),
                                    "GridWidth", UintegerValue(4),
                                    "LayoutType", StringValue("RowFirst"));

    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(apNodes.Get(0));
    mobility.Install(staNodes.Get(0));
    mobility.Install(staNodes.Get(1));
    mobility.Install(apNodes.Get(1));
// wifi network
    Ptr<MatrixPropagationLossModel> lossModel = CreateObject<MatrixPropagationLossModel>();
    lossModel->SetDefaultLoss(200);
    lossModel->SetLoss(apNodes.Get(0)->GetObject<MobilityModel>(), staNodes.Get(0)->GetObject<MobilityModel>(), 50);
    lossModel->SetLoss(apNodes.Get(1)->GetObject<MobilityModel>(), staNodes.Get(1)->GetObject<MobilityModel>(), 50);
    
    Ptr<YansWifiChannel> channel = CreateObject <YansWifiChannel> ();
    channel->SetPropagationDelayModel (CreateObject <ConstantSpeedPropagationDelayModel> ());
    channel->SetPropagationLossModel(lossModel);

    YansWifiPhyHelper phy;
    phy.SetChannel(channel);
    
    WifiHelper wifi;
    wifi.SetStandard (WIFI_STANDARD_80211a);
    std::string phyMode("OfdmRate54Mbps");
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode", StringValue(phyMode), "ControlMode", StringValue(phyMode));

    WifiMacHelper mac;
    Ssid ssid = Ssid("spe-ssid");
    
    mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
    NetDeviceContainer apDevices;
    apDevices = wifi.Install(phy, mac, apNodes);

    mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid));
    NetDeviceContainer staDevices;
    staDevices = wifi.Install(phy, mac, staNodes);

// internet 
    InternetStackHelper stack;
    stack.Install(apNodes);
    stack.Install(staNodes);

    Ipv4AddressHelper address;
    address.SetBase("10.0.2.0", "255.255.255.0");
    Ipv4InterfaceContainer staWifiInterfaces;
    staWifiInterfaces = address.Assign(staDevices);
    Ipv4InterfaceContainer apWifiInterfaces;
    apWifiInterfaces = address.Assign(apDevices);

    uint16_t portB = 8000;
    Address serverAddressB = InetSocketAddress( apWifiInterfaces.GetAddress(0), portB);
    
    double simulationTime = 8.0;
    double sendStart = 1.1;
    double sendTime = 5.0;

    //serverB
    PacketSinkHelper sinkHelperB ("ns3::TcpSocketFactory", serverAddressB);
    ApplicationContainer serverAppB = sinkHelperB.Install(apNodes.Get(0));
    serverAppB.Start(Seconds(0));
    serverAppB.Stop(Seconds(simulationTime));
    //clientB
    OnOffHelper onOffHelperB ("ns3::TcpSocketFactory", serverAddressB);
    onOffHelperB.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
    onOffHelperB.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
    onOffHelperB.SetAttribute ("PacketSize", UintegerValue (packetsize));
    ApplicationContainer clientAppsB;
    onOffHelperB.SetAttribute ("DataRate", StringValue (std::to_string(datarate)+"Mbps"));
    onOffHelperB.SetAttribute ("StartTime", TimeValue (Seconds (sendStart)));
    onOffHelperB.SetAttribute ("StopTime", TimeValue (Seconds (sendStart + sendTime)));
    clientAppsB.Add(onOffHelperB.Install(staNodes.Get(0)));
    

    uint16_t portC = 8000;
    Address serverAddressC = InetSocketAddress(apWifiInterfaces.GetAddress(1), portC);

    //serverC
    PacketSinkHelper sinkHelperC ("ns3::TcpSocketFactory", serverAddressC);
    ApplicationContainer serverAppC = sinkHelperC.Install(apNodes.Get(1));
    serverAppB.Start(Seconds(0));
    serverAppB.Stop(Seconds(simulationTime));
    //clientC
    OnOffHelper onOffHelperC ("ns3::TcpSocketFactory", serverAddressC);
    onOffHelperC.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
    onOffHelperC.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
    onOffHelperC.SetAttribute ("PacketSize", UintegerValue (packetsize));
    ApplicationContainer clientAppsC;
    onOffHelperC.SetAttribute ("DataRate", StringValue (std::to_string(datarate)+"Mbps"));
    onOffHelperC.SetAttribute ("StartTime", TimeValue (Seconds (sendStart)));
    onOffHelperC.SetAttribute ("StopTime", TimeValue (Seconds (sendStart + sendTime)));
    clientAppsC.Add(onOffHelperC.Install(staNodes.Get(1)));

// enable sniffing
    phy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);
    phy.EnablePcap ("wifi-apNode0", apDevices.Get(0));
    phy.EnablePcap ("wifi-staNode1", staDevices.Get(0));
    phy.EnablePcap ("wifi-staNode2", staDevices.Get(1));
    phy.EnablePcap ("wifi-apNode1", apDevices.Get(1));

    AsciiTraceHelper asciiTraceHelper;
    Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream ("tx_trace.txt");
    clientAppsB.Get(0)->TraceConnectWithoutContext("Tx",MakeBoundCallback(&Tx,stream));
       
    Ptr<OutputStreamWrapper> stream2 = asciiTraceHelper.CreateFileStream ("rx_trace.txt");
    serverAppB.Get(0)->TraceConnectWithoutContext ("Rx", MakeBoundCallback(&Rx, stream2));

   
    phy.EnableAscii("wifi-st0", staDevices.Get(0));
    phy.EnableAscii("wifi-st1", staDevices.Get(1));

// flow monitor
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll ();

    Simulator::Stop(Seconds(simulationTime));
    Simulator::Run();
    
    monitor->CheckForLostPackets ();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
    FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();
      for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i) {
          Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
          std::cout << "Flow " << i->first << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
          std::cout << "  Tx Packets: " << i->second.txPackets << "\n";
          std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
          std::cout << "  TxOffered:  " << i->second.txBytes * 8.0 / sendTime / 1000 / 1000  << " Mbps\n";
          std::cout << "  Rx Packets: " << i->second.rxPackets << "\n";
          std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
          std::cout << "  Mean delay:  " << i->second.delaySum /i->second.rxPackets << "\n";
          std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / sendTime / 1000 / 1000  << " Mbps\n";
          std::cout << "  Packet Loss Ratio: " << (i->second.txPackets - i->second.rxPackets)*100/(double)i->second.txPackets << " %\n";
  }  
    Simulator::Destroy();
    return 0;

      }  
    
    

    


