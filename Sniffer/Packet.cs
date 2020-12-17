using System.Text;
using Sniffer.Parser;

namespace Sniffer
{
    public class Packet : BasePacketInfo
    {
        public int ID { get; private set; }
        internal byte[] Data { get; private set; }

        public int Length { get => Data.Length; }

        public override string Type => LinkLayerInfo?.Type ?? "Unknown";

        public string Source => LinkLayerInfo?.Source ?? "Unknown";

        public string Destination => LinkLayerInfo?.Destination ?? "Unknown";

        public BaseLinkLayerInfo LinkLayerInfo { get; private set; }

        public override BasePacketInfo Payload => LinkLayerInfo;

        public Packet(byte[] packetData)
        {
            Data = packetData;
            LinkLayerInfo = LinkLayerFactory.GetLinkLayerInfo(0, Data);
        }

        public Packet(int id, byte[] packetData) : this(packetData)
        {
            ID = id;
        }

        public override string ToString()
        {
            var stringBuilder = new StringBuilder();
            stringBuilder.Append("Length: " + Data.Length + " ");
            return stringBuilder.ToString();
        }
    }
}