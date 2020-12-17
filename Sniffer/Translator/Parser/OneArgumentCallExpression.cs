namespace Sniffer.Translator.Parser
{
    public class OneArgumentCallExpression : Node
    {
        protected OneArgumentCallbackFunction Callback { get; private set; }
        protected Node Argument { get; private set; }

        public OneArgumentCallExpression(OneArgumentCallbackFunction callback, Node argument)
        {
            Callback = callback;
            Argument = argument;
        }

        public override object Evaluate(Packet packet)
        {
            return Callback(packet, Argument.Evaluate(packet));
        }
    }
}
