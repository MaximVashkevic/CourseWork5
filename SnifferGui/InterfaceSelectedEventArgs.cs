using System.Windows;

namespace SnifferGui
{
    public class InterfaceSelectedEventArgs : RoutedEventArgs
    {
        public string InterfaceID { get; set; }

        public InterfaceSelectedEventArgs(string interfaceID)
        {
            InterfaceID = interfaceID;
        }
    }
}