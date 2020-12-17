namespace Sniffer.Translator.Parser
{
    public class TwoArgumentCallExpression : Node
    {
        public TwoArgumentsCallbackFunction Callback { get; private set; }
        public Node FirstArgument { get; private set; }
        public Node SecondArgument { get; private set; }

        public TwoArgumentCallExpression(TwoArgumentsCallbackFunction callback, Node firstArgument, Node secondArgument)
        {
            Callback = callback;
            FirstArgument = firstArgument;
            SecondArgument = secondArgument;
        }

        public override object Evaluate(Packet packet)
        {
            return Callback(packet, FirstArgument.Evaluate(packet), SecondArgument.Evaluate(packet));
        }
    }
}
