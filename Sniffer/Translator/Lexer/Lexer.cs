using System.Collections.Generic;
using System.Text;

namespace Sniffer.Translator.LexicalAnalyzer
{
    public class Lexer
    {
        private const char EOF = '\0';

        private char _peek;
        private int _index = 0;
        private readonly string _input;

        public Lexer(string input)
        {
            _input = input;
            if (string.IsNullOrEmpty(_input))
            {
                _peek = EOF;
            }
            else
            {
                _peek = _input[0];
            }
        }

        private char ReadChar()
        {
            _index++;
            if (_index < _input.Length)
            {
                _peek = _input[_index];
            }
            else
            {
                _peek = EOF;
            }
            return _peek;
        }

        private Token Scan()
        {
            if (char.IsWhiteSpace(_peek))
            {
                ReadChar();
            }

            switch (_peek)
            {
                case '(':
                    {
                        ReadChar();
                        return Token.LeftBrace;
                    }
                case ')':
                    {
                        ReadChar();
                        return Token.RightBrace;
                    }
                case ',':
                    {
                        ReadChar();
                        return Token.Comma;
                    }
            }

            if (char.IsDigit(_peek))
            {
                return GetInteger();
            }

            if (_peek == '"')
            {
                return GetString();
            }

            if (char.IsLetter(_peek))
            {
                return GetWord();
            }

            ReadChar();
            return new Token(Tag.Unknown);
        }

        private Token GetInteger()
        {
            int value = 0;
            do
            {
                value = value * 10 + (int)char.GetNumericValue(_peek);
                ReadChar();
            }
            while (char.IsDigit(_peek));
            return new Integer(value);
        }

        private string GetIdentifier()
        {
            var stringBuilder = new StringBuilder();
            do
            {
                stringBuilder.Append(_peek);
                ReadChar();
            }
            while (char.IsLetterOrDigit(_peek));
            return stringBuilder.ToString();
        }

        private Token GetWord()
        {
            var stringBuilder = new StringBuilder();
            stringBuilder.Append(GetIdentifier());
            while (_peek == '.')
            {
                stringBuilder.Append('_');
                ReadChar();
                stringBuilder.Append(GetIdentifier());
            }
            string name = stringBuilder.ToString();

            return new Word(name, Tag.FunctionName);
        }

        private Token GetString()
        {
            var stringBuilder = new StringBuilder();
            ReadChar();
            do
            {
                stringBuilder.Append(_peek);
                ReadChar();
            }
            while (_peek != '"' && _peek != EOF);
            ReadChar();
            string value = stringBuilder.ToString();
            return new Word(value, Tag.Word);
        }

        public List<Token> Tokenize()
        {
            var result = new List<Token>();
            while (_index < _input.Length)
            {
                result.Add(Scan());
            }
            if (result.Count == 0)
            {
                result.Add(new Token(Tag.Unknown));
            }
            return result;
        }
    }
}
