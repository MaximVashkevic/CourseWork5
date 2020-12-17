namespace Sniffer.Parser
{
    public abstract class BaseLinkLayerInfo : BasePacketInfo
    {
        public abstract string Source { get; }
        public abstract string Destination { get; }
    }
}
