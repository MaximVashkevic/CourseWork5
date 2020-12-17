namespace Sniffer.Translator.Parser
{
    public class StringLiteral : Node
    {
        public string Value { get; private set; }

        public StringLiteral(string value)
        {
            Value = value;
        }

        public override object Evaluate(Packet packet)
        {
            return Value;
        }
    }
}
