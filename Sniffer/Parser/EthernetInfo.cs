using System;
using System.IO;
using System.Net.NetworkInformation;
using Sniffer.Parser.Factories;
using Sniffer.Translator.Parser;

namespace Sniffer.Parser
{
    public class EthernetInfo : BaseLinkLayerInfo
    {
        private const int PhysicalAddressBytesCount = 6;
        private const uint VlanHeaderEtherType = 0x8100;
        private const string TypeDescription = "Ethernet";
        private const int HeaderLengthWithoutVlanHeader = 14;
        private const int VlanHeaderSize = sizeof(uint);
        private const int HeaderLengthWithVlanHeader = HeaderLengthWithoutVlanHeader + VlanHeaderSize;

        private const int HexBase = 16;

        public const int TypeID = 1;

        public PhysicalAddress SourceAddress { get; private set; }
        public PhysicalAddress DestinationAddress { get; private set; }
        public ushort EtherType { get; private set; }
        public string HexEtherType { get => $"{EtherType:X4}"; }
        public uint VlanHeader { get; private set; }
        private int PayloadOffset => (EtherType == VlanHeaderEtherType) ? HeaderLengthWithVlanHeader : HeaderLengthWithoutVlanHeader;

        public BaseNetworkLayerInfo NetworkLayerInfo { get; private set; }

        public override string Type => NetworkLayerInfo?.Type ?? TypeDescription;

        public override string Source => NetworkLayerInfo?.Source ?? SourceAddress.ToString();

        public override string Destination => NetworkLayerInfo?.Destination ?? DestinationAddress.ToString();

        public override BasePacketInfo Payload => NetworkLayerInfo;

        private PhysicalAddress GetPhysicalAddress(BinaryReader packetReader)
        {
            byte[] physicalAddressBytes = packetReader.ReadBytes(PhysicalAddressBytesCount);
            return new PhysicalAddress(physicalAddressBytes);
        }

        public EthernetInfo(byte[] packetData)
        {
            using (var memoryStream = new MemoryStream(packetData, 0, packetData.Length))
            {
                using (var reader = new NetworkEndianBinaryReader(memoryStream))
                {
                    DestinationAddress = GetPhysicalAddress(reader);
                    SourceAddress = GetPhysicalAddress(reader);
                    EtherType = reader.ReadUInt16();

                    if (EtherType == VlanHeaderEtherType)
                    {
                        VlanHeader = (uint)(EtherType << 16) & reader.ReadUInt16();
                        EtherType = reader.ReadUInt16();
                    }
                    var payloadLength = packetData.Length - PayloadOffset;
                    NetworkLayerInfo = NetworkLayerFactory.GetNetworkLayerInfo(EtherType, packetData, PayloadOffset, payloadLength);
                }
            }
        }

        public override string ToString()
        {
            return $"Ethernet\nSource: {SourceAddress}\nDestination: {DestinationAddress}\nEtherType: {EtherType:X4}";
        }

        [ParserFunction("Ethernet", 0)]
        public static bool IsEthernet(object argument)
        {
            return GetEthernetInfo(argument) is EthernetInfo;
        }

        private static EthernetInfo GetEthernetInfo(object argument)
        {
            var packet = argument as Packet;
            return packet?.LinkLayerInfo as EthernetInfo;
        }

        [ParserFunction("Ethernet_EtherType", 1)]
        public static bool MatchEtherType(object first, object second)
        {
            var ethernetInfo = GetEthernetInfo(first);
            if (ethernetInfo != null)
            {
                var etherTypeString = second as string ?? "";
                var etherType = Convert.ToInt32(etherTypeString, HexBase);
                return etherType == ethernetInfo.EtherType;
            }
            return false;
        }

        [ParserFunction("Ethernet_Destination", 1)]
        public static bool MatchDestinationMac(object first, object second)
        {
            var ethernetInfo = GetEthernetInfo(first);
            if (ethernetInfo != null)
            {
                var macAddressString = second as string;
                return ethernetInfo.DestinationAddress.ToString() == macAddressString;
            }
            return false;
        }

        [ParserFunction("Ethernet_Source", 1)]
        public static bool MatchSourceMac(object first, object second)
        {
            var ethernetInfo = GetEthernetInfo(first);
            if (ethernetInfo != null)
            {
                var macAddressString = second as string;
                return ethernetInfo.SourceAddress.ToString() == macAddressString;
            }
            return false;
        }
    }
}
