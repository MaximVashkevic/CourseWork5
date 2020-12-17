using System;
using System.Runtime.InteropServices;

namespace ManagedSnifferDllWrapper
{
    public static class CaptureFunctions
    {
        private const string LibraryName = "SnifferDllForManaged.DLL";

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl, EntryPoint = "StartSniffing")]
        private static extern IntPtr StartSniffingDll();

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
        private static extern int GetPacketManaged(IntPtr captureHandle, ref IntPtr packetData);

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern void StopSniffing(IntPtr captureHandle);

        public static SnifferHandle StartSniffing()
        {
            return new SnifferHandle(StartSniffingDll());
        }

        public static Packet GetPacket(SnifferHandle deviceHandle)
        {
            IntPtr packetDataPointer = IntPtr.Zero;
            Packet packet = null;
            int capturedLength = GetPacketManaged(deviceHandle.DangerousGetHandle(), ref packetDataPointer);

            if (capturedLength > 0)
            {
                byte[] packetData = new byte[capturedLength];
                Marshal.Copy(packetDataPointer, packetData, 0, packetData.Length);
                Marshal.FreeCoTaskMem(packetDataPointer);

                packet = new Packet(packetData);
            }

            //var getFrameResult = (GetFrameResult)GetFrame(deviceHandle.DangerousGetHandle(),
            //    ref packetHeader, ref packetDataPointer);
            //switch (getFrameResult)
            //{
            //    case GetFrameResult.Ok:
            //        {
            //            byte[] packetData = new byte[packetHeader.CaptureLength];
            //            Marshal.Copy(packetDataPointer, packetData, 0, packetData.Length);
            //            Marshal.FreeCoTaskMem(packetDataPointer);

            //            packet = new Packet(packetHeader, packetData);
            //            capturing = false;
            //            break;
            //        }
            //    case GetFrameResult.Error:
            //    case GetFrameResult.EndOfFile:
            //        {
            //            capturing = false;
            //            break;
            //        }
            //}
            return packet;
        }
    }
}
