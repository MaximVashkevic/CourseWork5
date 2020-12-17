using System;

namespace Sniffer.Translator.Parser
{
    [AttributeUsage(AttributeTargets.Method)]
    public class ParserFunctionAttribute : Attribute
    {
        public string Name { get; private set; }
        public int ArgumentsCount { get; private set; }

        public ParserFunctionAttribute(string name, int argumentsCount)
        {
            Name = name;
            ArgumentsCount = argumentsCount;
        }
    }
}
