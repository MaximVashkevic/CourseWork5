namespace Sniffer.Translator.LexicalAnalyzer
{
    public class Word : Token
    {
        public string Value { get; private set; }

        public Word(string value, Tag tag) : base(tag)
        {
            Value = value;
        }
    }
}
