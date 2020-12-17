using System;
using Microsoft.Win32.SafeHandles;

namespace ManagedSnifferDllWrapper
{
    public class SnifferHandle : SafeHandleZeroOrMinusOneIsInvalid
    {
        public SnifferHandle(IntPtr handle) : base(true)
        {
            SetHandle(handle);
        }

        protected override bool ReleaseHandle()
        {
            CaptureFunctions.StopSniffing(handle);
            return true;
        }
    }
}
