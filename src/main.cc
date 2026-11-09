#include "ast/ASTPrinter.h"
#include "converter/CodeGenerator.h"
#include "lexer/Lexer.h"
#include "parser/Parser.h"

#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <string>
#include <system_error>

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
         "  -o, --output <file>  Write the generated output to <file>.\n"
         "                       The default is standard output; '-' also means stdout.\n"
         "      --output=<file>  Same as --output <file>.\n"
         "      --ast            Print the parsed abstract syntax tree instead of Python.\n"
         "  -h, --help           Show this help message and exit.\n"
         "      --               Treat the next argument as the input file name, even if\n"
         "                       it starts with '-'.\n"
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

int writeOutput(const std::string& text, const std::string& outputPath)
{
  if (outputPath.empty() || outputPath == "-")
  {
    std::cout << text;
    std::cout.flush();
    if (!std::cout)
    {
      std::cerr << "converter: failed while writing standard output\n";
      return 1;
    }
    return 0;
  }

  std::ofstream output(outputPath, std::ios::binary);
  if (!output)
  {
    std::cerr << "converter: cannot write '" << outputPath << "'\n";
    return 1;
  }

  output << text;
  output.close();
  if (!output)
  {
    std::cerr << "converter: failed while writing '" << outputPath << "'\n";
    return 1;
  }

  return 0;
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
  bool endOfOptions = false;

  for (int i = 1; i < argc; ++i)
  {
    const std::string argument = argv[i];

    if (!endOfOptions)
    {
      if (argument == "--")
      {
        endOfOptions = true;
        continue;
      }

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

        const std::string value = argv[++i];
        if (value.empty())
        {
          std::cerr << "converter: missing file name after '" << argument << "'\n\n";
          printUsage(std::cerr);
          return 1;
        }

        if (value.front() == '-' && value != "-")
        {
          std::cerr << "converter: option-like value '" << value
                    << "' cannot be used as an output file name\n\n";
          printUsage(std::cerr);
          return 1;
        }

        outputPath = value;
        continue;
      }

      if (argument.rfind("--output=", 0) == 0)
      {
        const std::string value = argument.substr(9);
        if (value.empty())
        {
          std::cerr << "converter: missing file name after '--output='\n\n";
          printUsage(std::cerr);
          return 1;
        }

        outputPath = value;
        continue;
      }

      if (!argument.empty() && argument.front() == '-' && argument != "-")
      {
        std::cerr << "converter: unknown option '" << argument << "'\n\n";
        printUsage(std::cerr);
        return 1;
      }
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
    if (std::cin.bad())
    {
      std::cerr << "converter: failed while reading standard input\n";
      return 1;
    }
  }
  else
  {
    std::error_code ec;
    if (!std::filesystem::is_regular_file(inputPath, ec))
    {
      std::cerr << "converter: cannot read '" << inputPath << "'\n";
      return 1;
    }

    std::ifstream input(inputPath, std::ios::binary);
    if (!input)
    {
      std::cerr << "converter: cannot read '" << inputPath << "'\n";
      return 1;
    }

    source = readAll(input);
    if (input.bad())
    {
      std::cerr << "converter: failed while reading '" << inputPath << "'\n";
      return 1;
    }
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

  try
  {
    if (printAst)
      return writeOutput(ASTPrinter::toString(*program), outputPath);

    return writeOutput(CodeGenerator::toString(*program), outputPath);
  }
  catch (const std::exception& error)
  {
    std::cerr << "converter: " << error.what() << "\n";
    return 1;
  }
}
