using System.IO;
using System.Text;
using Sniffer.Translator.Parser;

namespace Sniffer.Parser
{
    public class ArpInfo : BaseNetworkLayerInfo
    {
        private const string TypeDescription = "ARP";
        private const int RequestOperation = 1;
        private const char HardwareAddressDelimiter = ':';
        private const char DefaultDelimiter = ' ';
        private const char IPAddressDelimiter = '.';
        public const int TypeID = 0x0806;

        public ArpInfo(byte[] packetData, int offset, int length)
        {
            using (var memoryStream = new MemoryStream(packetData, offset, length))
            {
                using (var reader = new NetworkEndianBinaryReader(memoryStream))
                {
                    HardwareType = reader.ReadUInt16();
                    ProtocolType = reader.ReadUInt16();

                    HardwareAddressLength = reader.ReadByte();
                    ProtocolAddressLength = reader.ReadByte();
                    Operation = reader.ReadUInt16();

                    SenderHardwareAddress = reader.ReadBytes(HardwareAddressLength);
                    SenderProtocolAddress = reader.ReadBytes(ProtocolAddressLength);

                    TargetHardwareAddress = reader.ReadBytes(HardwareAddressLength);
                    TargetProtocolAddress = reader.ReadBytes(ProtocolAddressLength);
                }
            }
        }

        public ushort HardwareType { get; private set; }
        public ushort ProtocolType { get; private set; }
        public byte HardwareAddressLength { get; private set; }
        public byte ProtocolAddressLength { get; private set; }
        public ushort Operation { get; private set; }
        public byte[] SenderHardwareAddress { get; private set; }
        public byte[] SenderProtocolAddress { get; private set; }
        public byte[] TargetHardwareAddress { get; private set; }
        public byte[] TargetProtocolAddress { get; private set; }

        public string GetProtocolAddress(byte[] protocolAddressBytes)
        {
            if (protocolAddressBytes != null)
            {
                switch (ProtocolType)
                {

                    case IPv4Info.TypeID:
                        {
                            string result = "";
                            foreach (var protocolAddressByte in protocolAddressBytes)
                            {
                                result += protocolAddressByte.ToString() + IPAddressDelimiter;
                            }
                            return result.TrimEnd(IPAddressDelimiter);
                        }
                    default:
                        {
                            return ByteArrayToHexString(protocolAddressBytes, DefaultDelimiter);
                        }
                }
            }
            return "";
        }

        public override string Source => GetProtocolAddress(SenderProtocolAddress);

        public override string Destination => GetProtocolAddress(TargetProtocolAddress);

        public override string Type => TypeDescription;

        public override BasePacketInfo Payload => null;

        private static string ByteArrayToHexString(byte[] bytes, char delimiter)
        {
            var stringBuilder = new StringBuilder();
            for (int i = 0; i < bytes.Length; i++)
            {
                stringBuilder.Append(bytes[i].ToString("X2"));
                stringBuilder.Append(delimiter);
            }
            var result = stringBuilder.ToString().TrimEnd(delimiter);
            return result;
        }

        public override string ToString()
        {
            return $"ARP\nHardware type: {HardwareType:X4}"
                + $"\nProtocol type: {ProtocolType:X4}"
                + $"\nHardware address length: {HardwareAddressLength}"
                + $"\nProtocol address length: {ProtocolAddressLength}"
                + $"\nOperation: {Operation} ({(Operation == RequestOperation ? "Request" : "Reply")})"
                + "\nSender hardware address: " + ByteArrayToHexString(SenderHardwareAddress, HardwareAddressDelimiter)
                + "\nSender protocol address" + GetProtocolAddress(SenderProtocolAddress)
                + "\nTarget hardware address" + ByteArrayToHexString(TargetHardwareAddress, HardwareAddressDelimiter)
                + "\nTarget protocol address" + GetProtocolAddress(TargetProtocolAddress);
        }

        [ParserFunction("ARP", 0)]
        public static bool IsArp(object argument)
        {
            return GetArpInfo(argument) is ArpInfo;
        }

        private static ArpInfo GetArpInfo(object argument)
        {
            var packet = argument as Packet;
            var linkLayerInfo = packet?.Payload as BasePacketInfo;
            return linkLayerInfo?.Payload as ArpInfo;
        }

        [ParserFunction("ARP_Request", 0)]
        public static bool IsRequest(object argument)
        {
            var arpInfo = GetArpInfo(argument);
            if (arpInfo != null)
            {
                return arpInfo.Operation == RequestOperation;
            }
            return false;
        }

        [ParserFunction("ARP_Response", 0)]
        public static bool IsResponse(object argument)
        {
            var arpInfo = GetArpInfo(argument);
            if (arpInfo != null)
            {
                return arpInfo.Operation != RequestOperation;
            }
            return false;
        }
    }
}
