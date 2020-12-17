using System.Collections.Generic;
using Sniffer.Translator.LexicalAnalyzer;

namespace Sniffer.Translator.Parser
{
    public class TokenParser
    {
        private readonly List<Token> _tokens;
        private int _index;

        public static FunctionsRegistry FunctionsRegistry { get; private set; } = new FunctionsRegistry();

        public TokenParser(List<Token> tokens)
        {
            _tokens = tokens;
        }

        public Node GetFormula()
        {
            return CallExpression() as Node;
        }

        public Node Integer()
        {
            var token = _tokens[_index];
            Node result = null;
            if (token.Tag == Tag.Integer)
            {
                result = new IntegerLiteral(((Integer)token).Value);
                _index++;
            }
            return result;
        }

        public Node String()
        {
            var token = _tokens[_index];
            Node result = null;
            if (token.Tag == Tag.Word)
            {
                result = new StringLiteral(((Word)token).Value);
                _index++;
            }
            return result;
        }

        public Node Expression()
        {
            Node node = Integer();
            if (node != null)
            {
                return node;
            }

            node = String();
            if (node != null)
            {
                return node;
            }

            node = CallExpression();

            return node;
        }

        public Node CallExpression()
        {
            var previousIndex = _index;
            var token = _tokens[_index];
            if (token.Tag == Tag.FunctionName)
            {
                string name = (token as Word)?.Value;
                _index++;

                var leftBraceIndex = _index;
                Node firstNode, secondNode;
                if (Terminal(Tag.LeftBrace)
                    && (firstNode = Expression()) != null
                    && Terminal(Tag.Comma)
                    && (secondNode = Expression()) != null
                    && Terminal(Tag.RightBrace))
                {
                    return TwoArgumentsExpression(name, firstNode, secondNode);
                }

                _index = leftBraceIndex;
                if (Terminal(Tag.LeftBrace)
                    && (firstNode = Expression()) != null
                    && Terminal(Tag.RightBrace))
                {
                    return OneArgumentExpression(name, firstNode);
                }

                _index = leftBraceIndex;
                if (Terminal(Tag.LeftBrace) && Terminal(Tag.RightBrace))
                {
                    var callback = FunctionsRegistry.GetZeroArgumentsFunction(name);
                    return new ZeroArgumentCallExpression(callback);
                }
            }
            _index = previousIndex;
            return null;
        }

        private Node OneArgumentExpression(string name, Node firstNode)
        {
            switch (name)
            {
                case NotOperator.Name:
                    {
                        return new NotOperator(firstNode);
                    }
                default:
                    {
                        var callback = FunctionsRegistry.GetOneArgumentFunction(name);
                        return new OneArgumentCallExpression(callback, firstNode);
                    }
            }
        }

        private Node TwoArgumentsExpression(string name, Node firstNode, Node secondNode)
        {
            switch (name)
            {
                case AndOperator.Name:
                    {
                        return new AndOperator(firstNode, secondNode);
                    }
                case OrOperator.Name:
                    {
                        return new OrOperator(firstNode, secondNode);
                    }
                default:
                    {
                        var callback = FunctionsRegistry.GetTwoArgumentsFunction(name);
                        return new TwoArgumentCallExpression(callback, firstNode, secondNode);
                    }
            }
        }

        public bool Terminal(Tag expectedTag)
        {
            bool result = (_index < _tokens.Count) && (_tokens[_index].Tag == expectedTag);
            if (result)
            {
                _index++;
            }
            return result;
        }
    }
}
