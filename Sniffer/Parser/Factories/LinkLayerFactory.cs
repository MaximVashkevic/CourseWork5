namespace Sniffer.Parser
{
    public static class LinkLayerFactory
    {
        public static BaseLinkLayerInfo GetLinkLayerInfo(int linkLayerType, byte[] packetData)
        {
            return new EthernetInfo(packetData);
        }
    }
}
