namespace Sniffer.Translator.Parser
{
    public class AllPacketsNode : Node
    {
        public override object Evaluate(Packet packet)
        {
            return true;
        }
    }
}
