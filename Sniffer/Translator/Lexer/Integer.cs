using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Sniffer.Translator.LexicalAnalyzer
{
    public class Integer : Token
    {
        public int Value { get; private set; }
        public Integer(int value) : base(Tag.Integer)
        {
            Value = value;
        }
    }
}
