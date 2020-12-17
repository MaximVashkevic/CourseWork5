using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using Sniffer.Parser;

namespace Sniffer.Translator.Parser
{
    public class FunctionsRegistry
    {
        private const string InvalidFunctionMessage = "Function not found";

        private readonly Dictionary<string, ZeroArgumentsCallbackFunction> _zeroArgumentsCallbacks;
        private readonly Dictionary<string, OneArgumentCallbackFunction> _oneArgumentCallbacks;
        private readonly Dictionary<string, TwoArgumentsCallbackFunction> _twoArgumentsCallbacks;

        public FunctionsRegistry()
        {
            _zeroArgumentsCallbacks = new Dictionary<string, ZeroArgumentsCallbackFunction>();
            _oneArgumentCallbacks = new Dictionary<string, OneArgumentCallbackFunction>();
            _twoArgumentsCallbacks = new Dictionary<string, TwoArgumentsCallbackFunction>();

            Type baseType = typeof(BasePacketInfo);
            Assembly assembly = baseType.Assembly;

            foreach (Type type in assembly.GetTypes())
            {
                if (type.IsSubclassOf(baseType))
                {
                    foreach (MethodInfo methodInfo in type.GetMethods())
                    {
                        var attribute = (ParserFunctionAttribute)methodInfo.GetCustomAttribute(typeof(ParserFunctionAttribute));
                        if (attribute != null)
                        {
                            switch (attribute.ArgumentsCount)
                            {
                                case 0:
                                    {
                                        var d = Delegate.CreateDelegate(typeof(ZeroArgumentsCallbackFunction), null, methodInfo); ;
                                        var callback = (ZeroArgumentsCallbackFunction)d;
                                        AddZeroArgumentsFunction(callback, attribute.Name);
                                        break;
                                    }
                                case 1:
                                    {
                                        var callback = (OneArgumentCallbackFunction)Delegate.CreateDelegate(typeof(OneArgumentCallbackFunction), null, methodInfo);
                                        AddOneArgumentFunction(callback, attribute.Name);
                                        break;
                                    }
                                case 2:
                                    {
                                        var callback = (TwoArgumentsCallbackFunction)Delegate.CreateDelegate(typeof(TwoArgumentsCallbackFunction), null, methodInfo);
                                        AddTwoArgumentsFunction(callback, attribute.Name);
                                        break;
                                    }
                            }
                        }
                    }
                }
            }
        }

        public void AddZeroArgumentsFunction(ZeroArgumentsCallbackFunction callback, string name)
        {
            _zeroArgumentsCallbacks.Add(name, callback);
        }

        public void AddOneArgumentFunction(OneArgumentCallbackFunction callback, string name)
        {
            _oneArgumentCallbacks.Add(name, callback);
        }

        public void AddTwoArgumentsFunction(TwoArgumentsCallbackFunction callback, string name)
        {
            _twoArgumentsCallbacks.Add(name, callback);
        }

        public ZeroArgumentsCallbackFunction GetZeroArgumentsFunction(string name)
        {
            if (_zeroArgumentsCallbacks.ContainsKey(name))
            {
                return _zeroArgumentsCallbacks[name];
            }
            throw new ArgumentException(InvalidFunctionMessage, nameof(name));
        }

        public OneArgumentCallbackFunction GetOneArgumentFunction(string name)
        {
            if (_oneArgumentCallbacks.ContainsKey(name))
            {
                return _oneArgumentCallbacks[name];
            }
            throw new ArgumentException(InvalidFunctionMessage, nameof(name));
        }

        public TwoArgumentsCallbackFunction GetTwoArgumentsFunction(string name)
        {
            if (_twoArgumentsCallbacks.ContainsKey(name))
            {
                return _twoArgumentsCallbacks[name];
            }
            throw new ArgumentException(InvalidFunctionMessage, nameof(name));
        }
    }
}
