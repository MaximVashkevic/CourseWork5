namespace Sniffer.Parser.Factories
{
    public static class IPSubprotocolsFactory
    {
        public static BasePacketInfo GetIPSubprotocolInfo(int transportLayerType, byte[] packetData, int offset, int length)
        {
            switch(transportLayerType)
            {
                case TcpInfo.TypeID:
                    {
                        return new TcpInfo(packetData, offset, length);
                    }
                default:
                    {
                        return null;
                    }
            }
        }
    }
}
