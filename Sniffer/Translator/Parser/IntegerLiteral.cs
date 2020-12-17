namespace Sniffer.Translator.Parser
{
    public class IntegerLiteral : Node
    {
        public int Value { get; private set; }

        public IntegerLiteral(int value)
        {
            Value = value;
        }

        public override object Evaluate(Packet packet)
        {
            return Value;
        }
    }
}
