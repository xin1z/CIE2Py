#include "ast/ASTPrinter.h"
#include "converter/CodeGenerator.h"
#include "lexer/Lexer.h"
#include "parser/Parser.h"

#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <string>

namespace
{
void printUsage(std::ostream& out)
{
  out << "Usage: converter [options] [input.cie]\n"
         "\n"
         "Translate Cambridge IGCSE pseudocode into Python 3.\n"
         "When no input file is given, or when the input is '-', stdin is read.\n"
         "\n"
         "Options:\n"
         "  -o, --output <file>  Write the generated Python to <file> (default: stdout)\n"
         "      --ast            Print the parsed AST instead of the Python program\n"
         "  -h, --help           Show this help message\n";
}

std::string readAll(std::istream& input)
{
  return std::string(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
}
} // namespace

int main(int argc, char** argv)
{
  std::string inputPath;
  std::string outputPath;
  bool printAst = false;

  for (int i = 1; i < argc; ++i)
  {
    const std::string argument = argv[i];

    if (argument == "-h" || argument == "--help")
    {
      printUsage(std::cout);
      return 0;
    }

    if (argument == "--ast")
    {
      printAst = true;
      continue;
    }

    if (argument == "-o" || argument == "--output")
    {
      if (i + 1 >= argc)
      {
        std::cerr << "converter: missing file name after '" << argument << "'\n";
        return 1;
      }
      outputPath = argv[++i];
      continue;
    }

    if (!argument.empty() && argument.front() == '-' && argument != "-")
    {
      std::cerr << "converter: unknown option '" << argument << "'\n";
      printUsage(std::cerr);
      return 1;
    }

    if (!inputPath.empty())
    {
      std::cerr << "converter: more than one input file was given\n";
      return 1;
    }

    inputPath = argument;
  }

  std::string source;
  if (inputPath.empty() || inputPath == "-")
  {
    source = readAll(std::cin);
  }
  else
  {
    std::ifstream input(inputPath, std::ios::binary);
    if (!input)
    {
      std::cerr << "converter: cannot read '" << inputPath << "'\n";
      return 1;
    }
    source = readAll(input);
  }

  Lexer lexer(source);
  Parser parser(lexer.tokenize());
  std::unique_ptr<Program> program = parser.parse();

  if (!parser.errors().empty())
  {
    const std::string displayName = (inputPath.empty() || inputPath == "-") ? "<stdin>" : inputPath;
    for (const ParseError& error : parser.errors())
    {
      std::cerr << displayName << ":" << error.loc.line << ":" << error.loc.column << ": "
                << error.message << "\n";
    }
    return 1;
  }

  if (!program)
  {
    std::cerr << "converter: failed to parse the input\n";
    return 1;
  }

  if (printAst)
  {
    ASTPrinter printer(std::cout);
    printer.print(*program);
    return 0;
  }

  const std::string generated = CodeGenerator::toString(*program);

  if (outputPath.empty() || outputPath == "-")
  {
    std::cout << generated;
    return 0;
  }

  std::ofstream output(outputPath, std::ios::binary);
  if (!output)
  {
    std::cerr << "converter: cannot write '" << outputPath << "'\n";
    return 1;
  }

  output << generated;
  if (!output)
  {
    std::cerr << "converter: failed while writing '" << outputPath << "'\n";
    return 1;
  }

  return 0;
}
