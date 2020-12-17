using System;
using Sniffer.Parser;
using Sniffer.Translator.LexicalAnalyzer;
using Sniffer.Translator.Parser;

namespace Sniffer
{
    public sealed class PacketFilterService : ICaptureService, IDisposable
    {
        public event EventHandler<PacketEventArgs> PacketReceivedEvent;
        private PacketCaptureService _captureService;
        private readonly Node _callExpression;

        public int ID { get => _captureService.ID; }

        public PacketFilterService(PacketStorageService storageService, string filter, int id)
        {
            _captureService = new PacketCaptureService(storageService, id);
            _captureService.PacketReceivedEvent += OnPacketReceived;
            var lexer = new Lexer(filter);
            var parser = new TokenParser(lexer.Tokenize());
            try
            {
                _callExpression = parser.GetFormula();
            }
            catch (ArgumentException)
            {
                _callExpression = null;
            }
            if (_callExpression == null)
            {
                _callExpression = new AllPacketsNode();
            }
        }

        private void OnPacketReceived(object sender, PacketEventArgs e)
        {
            if ((_callExpression.Evaluate(e.Packet) as bool?) ?? false)
            {
                var packetReceivedEvent = PacketReceivedEvent;
                packetReceivedEvent?.Invoke(this, e);
            }
        }

        public void StartCapture()
        {
            _captureService.StartCapture();
        }

        public void StopCapture()
        {
            _captureService.StopCapture();
        }

        public void GetPacketsFromStorage()
        {
            _captureService.GetPacketsFromStorage();
        }

        public void Dispose()
        {
            _captureService?.Dispose();
            _captureService = null;
        }
    }
}
