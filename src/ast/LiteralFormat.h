#ifndef LITERAL_FORMAT_H
#define LITERAL_FORMAT_H

#include <string>

// Formatting helpers shared by the AST printer and the code generator so that
// real numbers, strings, and character literals are rendered consistently.
namespace literal
{
std::string formatReal(double value);
std::string quoteString(const std::string& value);
std::string quoteChar(char value);
} // namespace literal

#endif
