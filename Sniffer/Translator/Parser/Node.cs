namespace Sniffer.Translator.Parser
{
    public abstract class Node
    {
        public abstract object Evaluate(Packet packet);
    }
}
