namespace Sniffer.Parser.Factories
{
    public static class NetworkLayerFactory
    {
        public static BaseNetworkLayerInfo GetNetworkLayerInfo(int networkLayerType, byte[] packetData, int offset, int length)
        {
            switch (networkLayerType)
            {
                case IPv4Info.TypeID:
                    {
                        return new IPv4Info(packetData, offset, length);
                    }
                case ArpInfo.TypeID:
                    {
                        return new ArpInfo(packetData, offset, length);
                    }
                default:
                    {
                        return null;
                    }
            }
        }
    }
}
