using System.Windows;

namespace SnifferGui
{
    /// <summary>
    /// Логика взаимодействия для MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
        }

        private void OnShowPackets(object sender, RoutedEventArgs e)
        {
            var packetShowingControl = new PacketShowingControl();
            contentPresenter.Content = packetShowingControl;
        }
    }
}
