using System;
using System.Windows;
using System.Windows.Controls;
using Sniffer;
using System.Collections.ObjectModel;
using Sniffer.Parser;
using System.Windows.Threading;

namespace SnifferGui
{
    /// <summary>
    /// Логика взаимодействия для PacketShowingControl.xaml
    /// </summary>
    public sealed partial class PacketShowingControl : UserControl, IDisposable
    {
        private readonly ObservableCollection<BasePacketInfo> _packetInfos = new ObservableCollection<BasePacketInfo>();
        private readonly ObservableCollection<Sniffer.Packet> _packets = new ObservableCollection<Sniffer.Packet>();
        private ICaptureService _packetCaptureService;
        private readonly PacketStorageService _packetStorageService;
        private readonly Dispatcher _dispatcher;

        public event EventHandler ExitEvent;

        public PacketShowingControl()
        {
            InitializeComponent();
            _dispatcher = Dispatcher.CurrentDispatcher;
            tvPacketInfo.ItemsSource = _packetInfos;
            lvPackets.ItemsSource = _packets;
            _packetStorageService = new PacketStorageService();
            _packetCaptureService = new PacketCaptureService(_packetStorageService);
            _packetCaptureService.PacketReceivedEvent += OnPacketReceived;
            _packetCaptureService.StartCapture();
        }

        private void OnPacketReceived(object sender, PacketEventArgs e)
        {
            Action action = () =>
            {
                _packets.Add(e.Packet);
            };
            _dispatcher.BeginInvoke(action);
        }

        private void StopCaptureClick(object sender, RoutedEventArgs e)
        {
            _packetCaptureService?.StopCapture();
        }

        private void OnPacketSelected(object sender, SelectionChangedEventArgs e)
        {
            if (e.AddedItems.Count == 1)
            {
                _packetInfos.Clear();
                var packet = (Packet)e.AddedItems[0];
                var linkLayerInfo = packet.LinkLayerInfo;
                if (linkLayerInfo != null)
                {
                    _packetInfos.Add(linkLayerInfo);

                    var networkLayerInfo = linkLayerInfo.Payload;
                    if (networkLayerInfo != null)
                    {
                        _packetInfos.Add(networkLayerInfo);

                        var transportLayerInfo = networkLayerInfo.Payload;
                        if (transportLayerInfo != null)
                        {
                            _packetInfos.Add(transportLayerInfo);
                        }
                    }
                }
            }
        }

        private void OnSetFilterClick(object sender, RoutedEventArgs e)
        {
            _packetCaptureService.StopCapture();
            _packetCaptureService.PacketReceivedEvent -= OnPacketReceived;
            var filter = tbFilter.Text;
            var packetFilterService = new PacketFilterService(_packetStorageService, filter, _packetCaptureService.ID);
            packetFilterService.PacketReceivedEvent += OnPacketReceived;
            _packets.Clear();
            packetFilterService.GetPacketsFromStorage();
            packetFilterService.StartCapture();
            _packetCaptureService = packetFilterService;
        }

        private void OnExitClick(object sender, RoutedEventArgs e)
        {
            _packetCaptureService.StartCapture();
        }

        public void Dispose()
        {
            _packetStorageService?.Dispose();
        }
    }
}
