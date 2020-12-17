using System;
using System.IO;
using Sniffer.Translator.Parser;

namespace Sniffer.Parser
{
    public class TcpInfo : BasePacketInfo
    {
        private const int DataOffsetOffset = 12;
        private const int BytesPerWord = 4;
        private const int FlagsMask = 0x0FFF;
        private const int HeaderWithoutOptionsWordCount = 5;
        private const int HeaderWithoutOptionsSize = HeaderWithoutOptionsWordCount * BytesPerWord;
        private const string TypeDescription = "TCP";

        private const int CwrMask = 0b1000_0000;
        private const int EcnMask = 0b0100_0000;
        private const int UrgMask = 0b0010_0000;
        private const int AckMask = 0b0001_0000;
        private const int PshMask = 0b0000_1000;
        private const int RstMask = 0b0000_0100;
        private const int SynMask = 0b0000_0010;
        private const int FinMask = 0b0000_0001;

        public const int TypeID = 0x06;

        public ushort SourcePort { get; private set; }
        public ushort DestinationPort { get; private set; }
        public uint SequenceNumber { get; private set; }
        public uint AcknowledgmentNumber { get; private set; }
        public int DataOffset { get; private set; }
        public int Flags { get; private set; }
        public bool Cwr { get => (Flags & CwrMask) != 0; }
        public bool Ecn { get => (Flags & EcnMask) != 0; }
        public bool Urg { get => (Flags & UrgMask) != 0; }
        public bool Ack { get => (Flags & AckMask) != 0; }
        public bool Psh { get => (Flags & PshMask) != 0; }
        public bool Rst { get => (Flags & RstMask) != 0; }
        public bool Syn { get => (Flags & SynMask) != 0; }
        public bool Fin { get => (Flags & FinMask) != 0; }
        public ushort WindowSize { get; private set; }
        public ushort Checksum { get; private set; }
        public ushort UrgentPointer { get; private set; }
        public byte[] Options { get; private set; }


        public BasePacketInfo ApplicationLayerInfo { get; private set; }
        public override BasePacketInfo Payload { get => ApplicationLayerInfo; }
        public override string Type => Payload?.Type ?? TypeDescription;

        public TcpInfo(byte[] packetData, int offset, int length)
        {
            using (var stream = new MemoryStream(packetData, offset, length))
            {
                using (var reader = new NetworkEndianBinaryReader(stream))
                {
                    SourcePort = reader.ReadUInt16();
                    DestinationPort = reader.ReadUInt16();
                    SequenceNumber = reader.ReadUInt32();
                    AcknowledgmentNumber = reader.ReadUInt32();

                    var dataOffsetAndFlags = reader.ReadUInt16();
                    DataOffset = (dataOffsetAndFlags >> DataOffsetOffset) * BytesPerWord;
                    Flags = dataOffsetAndFlags & FlagsMask;
                    WindowSize = reader.ReadUInt16();
                    Checksum = reader.ReadUInt16();
                    UrgentPointer = reader.ReadUInt16();

                    if (DataOffset > HeaderWithoutOptionsSize)
                    {
                        var optionsSize = DataOffset - HeaderWithoutOptionsSize;
                        Options = reader.ReadBytes(optionsSize);
                    }
                }
            }
        }

        public override string ToString()
        {
            var result = "TCP";
            result += $"\nSource port: {SourcePort}";
            result += $"\nDestination port: {DestinationPort}";
            result += $"\nSequence number: {SequenceNumber}";
            result += $"\nAcknowledgement number: {AcknowledgmentNumber}";
            result += "\nFlags:";
            if (Cwr) { result += "\n\tCongestion window reduced"; }
            if (Ecn) { result += "\n\tEcho"; }
            if (Urg) { result += "\n\tUrgent"; }
            if (Ack) { result += "\n\tAcknowledgement"; }
            if (Psh) { result += "\n\tPush"; }
            if (Rst) { result += "\n\tReset"; }
            if (Syn) { result += "\n\tSyn"; }
            if (Fin) { result += "\n\tFin"; }
            result += $"\nWindow size: {WindowSize}";
            result += $"\nChecksum: {Checksum:X4}";
            return result;
        }

        [ParserFunction("TCP", 0)]
        public static bool IsTcp(object packet)
        {
            return GetTcpInfo(packet) is TcpInfo;
        }

        private static TcpInfo GetTcpInfo(object packetObject)
        {
            var packet = packetObject as Packet;
            var linkLayerInfo = packet?.Payload as BasePacketInfo;
            var networkLayerInfo = linkLayerInfo?.Payload as BasePacketInfo;
            return networkLayerInfo?.Payload as TcpInfo;
        }

        [ParserFunction("TCP_SourcePort", 1)]
        public static bool MatchSourcePort(object packet, object port)
        {
            var tcpInfo = GetTcpInfo(packet);
            return MatchPort(tcpInfo?.SourcePort, port);
        }

        [ParserFunction("TCP_DestinationPort", 1)]
        public static bool MatchDestinationPort(object packet, object port)
        {
            var tcpInfo = GetTcpInfo(packet);
            return MatchPort(tcpInfo?.DestinationPort, port);
        }

        private static bool MatchPort(int? firstPort, object secondPortObject)
        {
            var secondPort = secondPortObject as int?;
            return firstPort != null && secondPort != null && firstPort == secondPort;

        }
    }
}
