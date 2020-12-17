namespace Sniffer.Translator.Parser
{
    public class AndOperator : BinaryOperator
    {
        public const string Name = "AND";

        public AndOperator(Node firstArgument, Node secondArgument) : base(firstArgument, secondArgument) { }

        public override object Evaluate(Packet packet)
        {
            return ((FirstArgument?.Evaluate(packet) as bool?) ?? false)
                && ((SecondArgument?.Evaluate(packet) as bool?) ?? false);
        }
    }
}
