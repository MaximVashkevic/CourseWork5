using System;
using System.IO;
using System.Net;
using System.Text;
using Sniffer.Parser.Factories;
using Sniffer.Translator.Parser;

namespace Sniffer.Parser
{
    public class IPv4Info : BaseNetworkLayerInfo
    {
        private const int VersionOffset = 4;
        private const int IhlMask = 0x0F;
        private const int BytesPerIhlWord = 4;
        private const int DscpOffset = 2;
        private const int EcnMask = 0b0000_0011;
        private const int FlagsOffset = 13;
        private const int FragmentOffsetMask = 0b0001_1111__1111_1111;
        private const int IhlWithoutOptions = 5;
        private const int HeaderWithoutOptionsSize = IhlWithoutOptions * BytesPerIhlWord;
        private const int DFMask = 0b010;
        private const int MFMask = 0b001;
        private const string TypeDescription = "IPv4";

        public const int TypeID = 0x0800;

        private readonly uint _sourceIP;
        private readonly uint _destinationIP;

        public byte Version { get; private set; }
        public byte Ihl { get; private set; }
        public byte Dscp { get; private set; }
        public byte Ecn { get; private set; }
        public ushort TotalLength { get; private set; }
        public ushort Identification { get; private set; }
        public byte Flags { get; private set; }
        public ushort FragmentOffset { get; private set; }
        public byte TTL { get; private set; }
        public byte Protocol { get; private set; }
        public ushort Checksum { get; private set; }
        public IPAddress SourceIP { get; private set; }
        public IPAddress DestinationIP { get; private set; }

        public bool DontFragment { get => (Flags & DFMask) != 0; }
        public bool MoreFragments { get => (Flags & MFMask) != 0; }

        public override string Type => TransportLayerInfo?.Type ?? TypeDescription;

        public override string Source => SourceIP.ToString();

        public override string Destination => DestinationIP.ToString();

        public byte[] Options { get; private set; }

        public BasePacketInfo TransportLayerInfo { get; private set; }

        public override BasePacketInfo Payload => TransportLayerInfo;

        public IPv4Info(byte[] packetData, int offset, int length)
        {
            using (var memoryStream = new MemoryStream(packetData, offset, length))
            {
                using (var reader = new NetworkEndianBinaryReader(memoryStream))
                {
                    var versionAndIhl = reader.ReadByte();
                    Version = (byte)(versionAndIhl >> VersionOffset);
                    Ihl = (byte)((versionAndIhl & IhlMask) * BytesPerIhlWord);

                    var dscpAndEcn = reader.ReadByte();
                    Dscp = (byte)(dscpAndEcn >> DscpOffset);
                    Ecn = (byte)(dscpAndEcn >> EcnMask);

                    TotalLength = reader.ReadUInt16();
                    Identification = reader.ReadUInt16();

                    var flagsAndFragmentOffset = reader.ReadUInt16();
                    Flags = (byte)(flagsAndFragmentOffset >> FlagsOffset);
                    FragmentOffset = (ushort)(flagsAndFragmentOffset & FragmentOffsetMask);

                    TTL = reader.ReadByte();
                    Protocol = reader.ReadByte();
                    Checksum = reader.ReadUInt16();

                    _sourceIP = reader.ReadUInt32();
                    SourceIP = IPAddress.Parse(_sourceIP.ToString());

                    _destinationIP = reader.ReadUInt32();
                    DestinationIP = IPAddress.Parse(_destinationIP.ToString());

                    if (Ihl > HeaderWithoutOptionsSize)
                    {
                        var optionsSize = Ihl - HeaderWithoutOptionsSize;
                        Options = new byte[optionsSize];
                        for (int i = 0; i < Options.Length; i++)
                        {
                            Options[i] = reader.ReadByte();
                        }
                    }

                }
            }
            var payloadOffset = offset + Ihl;
            var payloadLength = packetData.Length - payloadOffset;
            TransportLayerInfo = IPSubprotocolsFactory.GetIPSubprotocolInfo(Protocol, packetData, payloadOffset, payloadLength);
        }

        private string GetProtocol(byte protocol)
        {
            switch (protocol)
            {
                case 0:
                case 255: return "Reserved";
                case 1: return "ICMP";
                case 3: return "Gateway-to-Gateway";
                case 4: return "CMCC Gateway Monitoring Message";
                case 5: return "ST";
                case 6: return "TCP";
                case 7: return "UCL";
                case 9: return "Secure";
                case 10: return "BBN RCC Monitoring";
                case 11: return "NVP";
                case 12: return "PUP";
                case 13: return "Pluribus";
                case 14: return "Telenet";
                case 15: return "XNET";
                case 16: return "Chaos";
                case 17: return "UDP";
                case 18: return "Multiplexing";
                case 19: return "DCN";
                case 20: return "TAC Monitoring";
                case 63: return "any local network";
                case 64: return "SATNET and Backroom EXPAK";
                case 65: return "MIT Subnet Support";
                case 69: return "SATNET Monitoring";
                case 71: return "Internet Packet Core Utility";
                case 76: return "Backroom SATNET Monitoring";
                case 78: return "WIDEBAND Monitoring ";
                case 79: return "WIDEBAND EXPAK";
                default: return "Unassigned";
            }
        }

        public override string ToString()
        {
            var result = "IP";
            result += $"\nHeader length: {Ihl}";
            result += $"\nTotal length: {TotalLength}";
            result += $"\nIdentification: 0x{Identification:X4} ({Identification})";
            result += $"\nFlags: {(DontFragment ? "Don't fragment" : "")} {(MoreFragments ? "More fragments" : "")}";
            result += $"\nFragment offset: {FragmentOffset}";
            result += $"\nTime to live: {TTL}";
            result += $"\nProtocol: {GetProtocol(Protocol)}";
            result += $"\nHeader checksum: 0x{Checksum:X4}";
            result += $"\nSource address: {Source}";
            result += $"\nDestination address: {Destination}";
            return result;
        }

        [ParserFunction("IPv4", 0)]
        public static bool IsIPv4(object packet)
        {
            return GetIPv4Info(packet) is IPv4Info;
        }

        [ParserFunction("IPv4_SourceIP", 1)]
        public static bool MatchSourceIP(object packet, object sourceIP)
        {
            var ipv4Info = GetIPv4Info(packet);
            return MatchIP(ipv4Info?.SourceIP, sourceIP);
        }

        private static bool MatchIP(IPAddress firstIP, object secondIPObject)
        {
            string ipAddressString = secondIPObject as string;
            var secondIP = IPAddress.Parse(ipAddressString);
            return firstIP?.Equals(secondIP) ?? false;
        }

        [ParserFunction("IPv4_DestinationIP", 1)]
        public static bool MatchDestinationIP(object packet, object destinationIP)
        {
            var ipv4Info = GetIPv4Info(packet);
            return MatchIP(ipv4Info?.DestinationIP, destinationIP);
        }

        private static IPv4Info GetIPv4Info(object packetObject)
        {
            var packet = packetObject as Packet;
            var linkLayerInfo = packet?.Payload as BasePacketInfo;
            return linkLayerInfo?.Payload as IPv4Info;
        }
    }
}
