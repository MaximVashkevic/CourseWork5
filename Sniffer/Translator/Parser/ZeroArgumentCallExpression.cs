namespace Sniffer.Translator.Parser
{
    class ZeroArgumentCallExpression : Node
    {
        protected ZeroArgumentsCallbackFunction Callback;

        public ZeroArgumentCallExpression(ZeroArgumentsCallbackFunction callback)
        {
            Callback = callback;
        }

        public override object Evaluate(Packet packet)
        {
            return Callback(packet);
        }
    }
}
