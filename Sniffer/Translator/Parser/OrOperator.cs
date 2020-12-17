namespace Sniffer.Translator.Parser
{
    public class OrOperator : BinaryOperator
    {
        public const string Name = "OR";

        public OrOperator(Node firstArgument, Node secondArgument) : base(firstArgument, secondArgument) { }

        public override object Evaluate(Packet packet)
        {
            return ((FirstArgument?.Evaluate(packet) as bool?) ?? false)
                || ((SecondArgument?.Evaluate(packet) as bool?) ?? false);
        }
    }
}
