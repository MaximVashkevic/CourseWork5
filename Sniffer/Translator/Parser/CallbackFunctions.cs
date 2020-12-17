namespace Sniffer.Translator.Parser
{
    public delegate bool ZeroArgumentsCallbackFunction(Packet packet);
    public delegate bool OneArgumentCallbackFunction(Packet packet, object argument);
    public delegate bool TwoArgumentsCallbackFunction(Packet packet, object first, object second);
}
