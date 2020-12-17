using System.IO;
using System.Net;

namespace Sniffer.Parser
{
    class NetworkEndianBinaryReader : BinaryReader
    {
        public NetworkEndianBinaryReader(Stream input) : base(input) { }

        public override ushort ReadUInt16()
        {
            return (ushort)ReadInt16();
        }
        public override uint ReadUInt32()
        {
            return (uint)ReadInt32();
        }
        public override ulong ReadUInt64()
        {
            return (ulong)ReadInt64();
        }
        public override short ReadInt16()
        {
            return IPAddress.NetworkToHostOrder(base.ReadInt16());
        }
        public override int ReadInt32()
        {
            return IPAddress.NetworkToHostOrder(base.ReadInt32());
        }
        public override long ReadInt64()
        {
            return IPAddress.NetworkToHostOrder(base.ReadInt64());
        }
    }
}
