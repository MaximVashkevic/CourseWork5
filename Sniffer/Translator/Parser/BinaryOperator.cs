namespace Sniffer.Translator.Parser
{
    public abstract class BinaryOperator : Node
    {
        public Node FirstArgument { get; private set; }
        public Node SecondArgument { get; private set; }

        public BinaryOperator(Node firstArgument, Node secondArgument)
        {
            FirstArgument = firstArgument;
            SecondArgument = secondArgument;
        }
    }
}
