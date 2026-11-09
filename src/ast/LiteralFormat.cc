#include "LiteralFormat.h"

#include <cstdio>
#include <iomanip>
#include <limits>
#include <sstream>

namespace literal
{
std::string formatReal(double value)
{
  std::ostringstream stream;
  stream << std::defaultfloat << std::setprecision(std::numeric_limits<double>::max_digits10)
         << value;
  std::string text = stream.str();

  if (text == "inf" || text == "-inf" || text == "nan" || text == "-nan")
    return "float(\"" + text + "\")";

  if (text.find_first_of(".eE") == std::string::npos)
    text += ".0";

  return text;
}

std::string quoteString(const std::string& value)
{
  std::string result = "\"";
  result.reserve(value.size() + 2);

  for (char raw : value)
  {
    const unsigned char character = static_cast<unsigned char>(raw);
    switch (character)
    {
    case '\\':
      result += "\\\\";
      break;
    case '"':
      result += "\\\"";
      break;
    case '\n':
      result += "\\n";
      break;
    case '\r':
      result += "\\r";
      break;
    case '\t':
      result += "\\t";
      break;
    default:
      if (character < 0x20 || character == 0x7F)
      {
        char buffer[8];
        std::snprintf(buffer, sizeof(buffer), "\\x%02x", character);
        result += buffer;
      }
      else
      {
        result += raw;
      }
      break;
    }
  }

  result += '"';
  return result;
}

std::string quoteChar(char value)
{
  const unsigned char character = static_cast<unsigned char>(value);
  switch (character)
  {
  case '\\':
    return "'\\\\'";
  case '\'':
    return "'\\''";
  case '\n':
    return "'\\n'";
  case '\r':
    return "'\\r'";
  case '\t':
    return "'\\t'";
  default:
  {
    if (character < 0x20 || character == 0x7F)
    {
      char buffer[8];
      std::snprintf(buffer, sizeof(buffer), "'\\x%02x'", character);
      return buffer;
    }

    return std::string("'") + value + "'";
  }
  }
}
} // namespace literal
