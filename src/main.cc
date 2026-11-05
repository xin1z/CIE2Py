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
  out << "Usage: converter [options] <input.cie>\n"
         "\n"
         "Translate Cambridge IGCSE pseudocode into Python 3.\n"
         "\n"
         "Arguments:\n"
         "  <input.cie>          Pseudocode source file.\n"
         "                       Use '-' to read the program from standard input.\n"
         "\n"
         "Options:\n"
         "  -o, --output <file>  Write the generated Python to <file>.\n"
         "                       The default is standard output; '-' also means stdout.\n"
         "      --ast            Print the parsed abstract syntax tree instead of Python.\n"
         "  -h, --help           Show this help message and exit.\n"
         "\n"
         "Examples:\n"
         "  converter program.cie\n"
         "  converter program.cie -o program.py\n"
         "  converter --ast program.cie\n"
         "  cat program.cie | converter -\n"
         "\n"
         "Exit status:\n"
         "  0  Success.\n"
         "  1  Invalid arguments, a syntax error in the input, or an I/O error.\n";
}

std::string readAll(std::istream& input)
{
  return std::string(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
}
} // namespace

int main(int argc, char** argv)
{
  if (argc == 1)
  {
    printUsage(std::cout);
    return 1;
  }

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
        std::cerr << "converter: missing file name after '" << argument << "'\n\n";
        printUsage(std::cerr);
        return 1;
      }
      outputPath = argv[++i];
      continue;
    }

    if (!argument.empty() && argument.front() == '-' && argument != "-")
    {
      std::cerr << "converter: unknown option '" << argument << "'\n\n";
      printUsage(std::cerr);
      return 1;
    }

    if (!inputPath.empty())
    {
      std::cerr << "converter: more than one input file was given\n\n";
      printUsage(std::cerr);
      return 1;
    }

    inputPath = argument;
  }

  if (inputPath.empty())
  {
    std::cerr << "converter: no input file was given\n\n";
    printUsage(std::cerr);
    return 1;
  }

  std::string source;
  if (inputPath == "-")
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
    const std::string displayName = inputPath == "-" ? "<stdin>" : inputPath;
    for (const ParseError& error : parser.errors())
    {
      std::cerr << displayName << ":" << error.loc.line << ":" << error.loc.column << ": "
                << error.message << "\n";
    }
    std::cerr << "\n";
    printUsage(std::cerr);
    return 1;
  }

  if (!program)
  {
    std::cerr << "converter: failed to parse the input\n\n";
    printUsage(std::cerr);
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
