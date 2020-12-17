namespace Sniffer.Parser
{
    public abstract class BasePacketInfo
    {
        public abstract string Type { get; }
        public abstract BasePacketInfo Payload { get; }
    }
}
