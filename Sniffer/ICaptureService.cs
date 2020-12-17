using System;

namespace Sniffer
{
    public interface ICaptureService
    {
        void StartCapture();
        void StopCapture();
        void GetPacketsFromStorage();

        int ID { get; }

        event EventHandler<PacketEventArgs> PacketReceivedEvent;
    }
}
