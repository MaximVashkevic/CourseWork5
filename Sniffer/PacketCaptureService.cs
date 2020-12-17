using System;
using System.Threading;
using CapturedPacket = ManagedSnifferDllWrapper.Packet;
using ManagedSnifferDllWrapper;

namespace Sniffer
{
    public sealed class PacketCaptureService : ICaptureService, IDisposable
    {
        private SnifferHandle _deviceHandle;
        private readonly int _linkLayerType;
        private Thread _thread;
        private bool _isCapturing = true;
        private PacketStorageService _storageService;

        public int ID { get; private set; } = 1;

        public event EventHandler<PacketEventArgs> PacketReceivedEvent;

        public PacketCaptureService(PacketStorageService storageService)
        {
            _deviceHandle = CaptureFunctions.StartSniffing();
            _linkLayerType = 0;
            _storageService = storageService;
        }

        public PacketCaptureService(PacketStorageService storageService, int id) : this(storageService)
        {
            ID = id;
        }

        public void StartCapture()
        {
            _thread = new Thread(GetPackets)
            {
                IsBackground = true
            };
            _thread.Start();
        }

        private void GetPackets()
        {
            CapturedPacket capturedPacket;
            while (_isCapturing)
            {
                capturedPacket = CaptureFunctions.GetPacket(_deviceHandle);
                if (capturedPacket == CapturedPacket.InvalidPacket)
                {
                    _isCapturing = false;
                }
                else
                {
                    var packet = new Packet(ID++, capturedPacket.PacketData);
                    _storageService.AddPacket(packet);
                    var packetReceivedEvent = PacketReceivedEvent;
                    packetReceivedEvent?.Invoke(this, new PacketEventArgs(packet));
                }
            }
        }

        public void StopCapture()
        {
            _isCapturing = false;
        }

        public void Dispose()
        {
            _deviceHandle?.Dispose();
            _deviceHandle = null;
            _storageService?.Dispose();
            _storageService = null;
        }

        public void GetPacketsFromStorage()
        {
            _storageService.GetPackets((Packet packet) =>
            {
                var packetReceivedEvent = PacketReceivedEvent;
                packetReceivedEvent?.Invoke(this, new PacketEventArgs(packet));
            });
        }
    }
}
