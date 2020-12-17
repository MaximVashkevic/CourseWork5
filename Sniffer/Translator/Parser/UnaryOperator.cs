namespace Sniffer.Translator.Parser
{
    public abstract class UnaryOperator : Node
    {
        protected Node Argument { get; private set; }

        public UnaryOperator(Node argument)
        {
            Argument = argument;
        }
    }
}
