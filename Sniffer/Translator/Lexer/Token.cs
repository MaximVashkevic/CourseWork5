namespace Sniffer.Translator.LexicalAnalyzer
{
    public class Token
    {
        public Tag Tag { get; private set; }

        public Token(Tag tag)
        {
            Tag = tag;
        }

        public static Token LeftBrace { get; private set; } = new Token(Tag.LeftBrace);
        public static Token RightBrace { get; private set; } = new Token(Tag.RightBrace);
        public static Token Comma { get; private set; } = new Token(Tag.Comma);
    }
}
