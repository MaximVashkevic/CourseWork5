namespace Sniffer.Translator.Parser
{
    public class NotOperator : UnaryOperator
    {
        public const string Name = "NOT";
        public NotOperator(Node argument) : base(argument) { }

        public override object Evaluate(Packet packet)
        {
            return !(Argument?.Evaluate(packet) as bool?) ?? false;
        }
    }
}
