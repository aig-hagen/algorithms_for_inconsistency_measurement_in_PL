#ifndef PARSER_H
#define PARSER_H

#include "Formula.h"
#include "Kb.h"
#include <regex>
#include <string>
#include <unordered_map>
#include <vector>

// The following regex expression define what are
// a) valid names for atoms
const std::regex IdRegex("[^|&!\\s()\\<\\>\\=\\^]+");
// b) tokens that are ignored (whitespace)
const std::regex IgnoreRegex("(\\s|\\n|\\r)+");

// This class is a Parser for the TweetyProject format for propositional formulas.
// The code is partly based on this implementation of the
// Shunting Yard Algorithm: https://rosettacode.org/wiki/Parsing/Shunting-yard_algorithm#C.2B.2B
class Parser
{
  private:
    enum class Associates
    {
        none,
        left_to_right,
        right_to_left
    };
    struct Operator_Info
    {
        int precedence;
        Associates associativity;
    };
    std::unordered_map<std::string, Operator_Info> Operators = {
        {"&&", {4, Associates::left_to_right}}, {"||", {3, Associates::left_to_right}},
        {"=>", {2, Associates::right_to_left}}, {"<=>", {1, Associates::left_to_right}},
        {"!", {5, Associates::left_to_right}},
    };

    // Utility methods used during parsing
    inline int precedence(const std::string &op)
    {
        return Operators[op].precedence;
    }
    inline Associates associativity(const std::string &op)
    {
        return Operators[op].associativity;
    }
    inline bool IsIdentifier(const std::string &t) const
    {
        return regex_match(t, IdRegex);
    }
    inline bool IsIgnoreToken(const std::string &t) const
    {
        return regex_match(t, IgnoreRegex);
    }
    inline bool IsOperator(const std::string &t) const
    {
        return Operators.count(t);
    }
    inline bool IsOpenParenthesis(const std::string &t) const
    {
        return t == "(";
    }
    inline bool IsCloseParenthesis(const std::string &t) const
    {
        return t == ")";
    }
    inline bool IsParenthesis(const std::string &t) const
    {
        return IsOpenParenthesis(t) or IsCloseParenthesis(t);
    }

    // Methods used for converting the parsing stack into Formula instances
    Formula ParseOutput(const std::vector<std::string> &parser_output);
    Formula EvaluateUnaryExpression(const Formula &left, std::string op);
    Formula EvaluateBinaryExpression(Formula &left, Formula &right, std::string op);
    Formula EvaluateAtomaryExpression(std::string atom);

    // Parses the string into a formula
    Formula ParseFormula(const std::string &tokens);
	// Transforms a single string line in DIMACS format to equivalent
	// string formula representation used in this app and tweety
	// e.g "5 -67 124 57 0" -> "A5||!A67||A124||A57"
	std::string decode_DIMACS_line(std::string str, std::string delimiter);

  public:
    Parser()
    {
    }
    ~Parser()
    {
    }

    // Parse knowledge base in TweetyProject format from the
    // file with the fiven path. The formulas are expected to
    // be separated by newlines.
    Kb ParseKbFromFile(std::string path);
	
	// Parse knowledge base in DIMACS format from the
    // file with the given path. The formulas are expected to
    // be separated by newlines.
	Kb ParseKbFromDIMACSFile(std::string path);

    // Parse knowledge base in TweetyProject format from the
    // given string. The formulas are expected to be separated
    // by newlines.
    Kb ParseKbFromString(std::string s);

    // Parse a single formula from the
    // given string
    Formula ParseFormulaFromString(std::string s);
};

#endif /* PARSER_H */
