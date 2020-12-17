using System;

namespace Sniffer
{
    public class PacketEventArgs : EventArgs
    {
        public Packet Packet { get; private set; }

        public PacketEventArgs(Packet packet)
        {
            Packet = packet;
        }
    }
}
