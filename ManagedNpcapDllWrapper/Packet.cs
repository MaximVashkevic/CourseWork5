namespace ManagedSnifferDllWrapper
{
    public class Packet
    {
        public const Packet InvalidPacket = null;

        public byte[] PacketData;

        public Packet(byte[] packetData)
        {
            PacketData = packetData;
        }
    }
}
