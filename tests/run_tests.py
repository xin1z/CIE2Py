#!/usr/bin/env python3
"""Integration tests for the CIE2Py command-line converter."""

import subprocess
import sys
import tempfile
import textwrap
from pathlib import Path

if len(sys.argv) != 2:
    print("usage: run_tests.py <path-to-converter>", file=sys.stderr)
    raise SystemExit(2)

CONVERTER = Path(sys.argv[1]).resolve()
PYTHON = sys.executable
FAILURES = []


def run_converter(arguments, input_text="", cwd=None):
    return subprocess.run(
        [str(CONVERTER), *map(str, arguments)],
        input=input_text,
        text=True,
        capture_output=True,
        cwd=cwd,
    )


def write_source(directory, source, name="program.cie"):
    path = directory / name
    path.write_text(textwrap.dedent(source).lstrip())
    return path


def convert(directory, source, *arguments):
    source_path = write_source(directory, source)
    result = run_converter([source_path, *arguments], cwd=directory)
    if result.returncode != 0:
        raise AssertionError(f"conversion failed:\n{result.stderr}")
    return result


def convert_and_run(directory, source, stdin=""):
    source_path = write_source(directory, source)
    output_path = directory / "program.py"
    result = run_converter([source_path, "-o", output_path], cwd=directory)
    if result.returncode != 0:
        raise AssertionError(f"conversion failed:\n{result.stderr}")
    return subprocess.run(
        [PYTHON, str(output_path)],
        input=stdin,
        text=True,
        capture_output=True,
        cwd=directory,
    )


def check(name, test):
    try:
        with tempfile.TemporaryDirectory(prefix="cie2py-test-") as temporary:
            test(Path(temporary))
    except Exception as error:  # noqa: BLE001 - the test runner reports every failure
        FAILURES.append(name)
        print(f"FAIL {name}: {error}")
    else:
        print(f"PASS {name}")


def test_basic(directory):
    process = convert_and_run(
        directory,
        """
        DECLARE Score : INTEGER
        Score <- 50
        IF Score >= 50 THEN
          OUTPUT "pass"
        ELSE
          OUTPUT "fail"
        ENDIF
        """,
    )
    assert process.returncode == 0, process.stderr
    assert process.stdout == "pass\n", process.stdout


def test_array_lower_bound(directory):
    process = convert_and_run(
        directory,
        """
        DECLARE Values : ARRAY[1:3] OF INTEGER
        Values[1] <- 10
        Values[2] <- 20
        Values[3] <- 30
        OUTPUT Values[1], ",", Values[2], ",", Values[3]
        """,
    )
    assert process.returncode == 0, process.stderr
    assert process.stdout == "10,20,30\n", process.stdout


def test_real_literal_is_preserved(directory):
    convert(directory, 'OUTPUT 0.123456789012345678\n', "-o", "program.py")
    generated = (directory / "program.py").read_text()
    assert "0.123456789012345678" in generated, generated


def test_round_uses_decimal_places(directory):
    process = convert_and_run(
        directory,
        """
        OUTPUT ROUND(1.005, 2), ",", ROUND(2.675, 2), ",", ROUND(-1.005, 2)
        """,
    )
    assert process.returncode == 0, process.stderr
    assert process.stdout == "1.01,2.68,-1.01\n", process.stdout


def test_case_statement(directory):
    process = convert_and_run(
        directory,
        """
        DECLARE Grade : STRING
        Grade <- "B"
        CASE OF Grade
          "A": OUTPUT "excellent"
          "B": OUTPUT "good"
          OTHERWISE OUTPUT "other"
        ENDCASE
        """,
    )
    assert process.returncode == 0, process.stderr
    assert process.stdout == "good\n", process.stdout


def test_string_builtins(directory):
    process = convert_and_run(
        directory,
        """
        OUTPUT LENGTH("abcdef"), ",", SUBSTRING("abcdef", 2, 3), ",", LCASE("AB"), ",", UCASE("ab")
        """,
    )
    assert process.returncode == 0, process.stderr
    assert process.stdout == "6,bcd,ab,AB\n", process.stdout


def test_boolean_text(directory):
    process = convert_and_run(directory, 'OUTPUT TRUE, ",", FALSE\n')
    assert process.returncode == 0, process.stderr
    assert process.stdout == "TRUE,FALSE\n", process.stdout


def test_div_and_mod_truncate(directory):
    process = convert_and_run(
        directory,
        'OUTPUT -7 DIV 2, ",", -7 MOD 2, ",", 7 DIV -2, ",", 7 MOD -2\n',
    )
    assert process.returncode == 0, process.stderr
    assert process.stdout == "-3,-1,-3,1\n", process.stdout


def test_global_declared_after_subprogram(directory):
    process = convert_and_run(
        directory,
        """
        FUNCTION ReadAndSet() RETURNS INTEGER
          Total <- 5
          RETURN Total
        ENDFUNCTION

        DECLARE Total : INTEGER
        Total <- 1
        OUTPUT ReadAndSet(), ",", Total
        """,
    )
    assert process.returncode == 0, process.stderr
    assert process.stdout == "5,5\n", process.stdout


def test_global_array_lower_bound_after_subprogram(directory):
    process = convert_and_run(
        directory,
        """
        DECLARE Low : INTEGER
        Low <- 1

        FUNCTION ReadBack() RETURNS INTEGER
          RETURN Values[2]
        ENDFUNCTION

        DECLARE Values : ARRAY[Low:3] OF INTEGER
        Values[2] <- 5
        Low <- 0
        OUTPUT ReadBack()
        """,
    )
    assert process.returncode == 0, process.stderr
    assert process.stdout == "5\n", process.stdout


def test_openfile_evaluates_name_once(directory):
    process = convert_and_run(
        directory,
        """
        DECLARE Calls : INTEGER
        Calls <- 0

        FUNCTION FileName() RETURNS STRING
          Calls <- Calls + 1
          RETURN "data.txt"
        ENDFUNCTION

        OPENFILE FileName() FOR WRITE
        WRITEFILE "data.txt", "hello"
        CLOSEFILE "data.txt"
        OUTPUT Calls
        """,
    )
    assert process.returncode == 0, process.stderr
    assert process.stdout == "1\n", process.stdout
    assert (directory / "data.txt").read_text() == "hello\n"


def test_array_upper_bound_evaluated_once(directory):
    process = convert_and_run(
        directory,
        """
        DECLARE Calls : INTEGER
        Calls <- 0

        FUNCTION Columns() RETURNS INTEGER
          Calls <- Calls + 1
          RETURN 2
        ENDFUNCTION

        DECLARE Grid : ARRAY[1:3, 1:Columns()] OF INTEGER
        OUTPUT Calls
        """,
    )
    assert process.returncode == 0, process.stderr
    assert process.stdout == "1\n", process.stdout


def test_ast_respects_output_option(directory):
    source_path = write_source(directory, 'OUTPUT "ast"\n')
    ast_path = directory / "tree.txt"
    result = run_converter(["--ast", source_path, "-o", ast_path], cwd=directory)
    assert result.returncode == 0, result.stderr
    assert result.stdout == "", result.stdout
    assert "PROGRAM" in ast_path.read_text()


def test_output_option_rejects_option_like_value(directory):
    source_path = write_source(directory, 'OUTPUT "x"\n')
    result = run_converter([source_path, "-o", "--ast"], cwd=directory)
    assert result.returncode != 0
    assert "option-like" in result.stderr, result.stderr
    assert not (directory / "--ast").exists()


def test_output_equals_form(directory):
    source_path = write_source(directory, 'OUTPUT "equals"\n')
    output_path = directory / "generated.py"
    result = run_converter([source_path, f"--output={output_path}"], cwd=directory)
    assert result.returncode == 0, result.stderr
    assert result.stdout == "", result.stdout
    assert '_cie_print("equals")' in output_path.read_text()


def test_end_of_options(directory):
    source_path = write_source(directory, 'OUTPUT "dash"\n', name="--program.cie")
    result = run_converter(["--", source_path.name], cwd=directory)
    assert result.returncode == 0, result.stderr
    assert '_cie_print("dash")' in result.stdout


def test_directory_input_is_rejected(directory):
    result = run_converter([directory], cwd=directory)
    assert result.returncode != 0
    assert "cannot read" in result.stderr, result.stderr
    assert result.stdout == "", result.stdout


def test_syntax_error_includes_usage(directory):
    source_path = write_source(directory, "IF TRUE\n")
    result = run_converter([source_path], cwd=directory)
    assert result.returncode != 0
    assert "expected 'THEN'" in result.stderr, result.stderr
    assert "Usage: converter" in result.stderr, result.stderr


def test_no_arguments_show_usage(directory):
    result = run_converter([], cwd=directory)
    assert result.returncode == 1
    assert "Usage: converter" in result.stdout, result.stdout


def test_stdin_input(directory):
    result = run_converter(["-"], input_text='OUTPUT "stdin"\n', cwd=directory)
    assert result.returncode == 0, result.stderr
    assert '_cie_print("stdin")' in result.stdout, result.stdout


def test_file_roundtrip(directory):
    process = convert_and_run(
        directory,
        """
        DECLARE Line : STRING
        OPENFILE "data.txt" FOR WRITE
        WRITEFILE "data.txt", "hello"
        CLOSEFILE "data.txt"
        OPENFILE "data.txt" FOR READ
        READFILE "data.txt", Line
        CLOSEFILE "data.txt"
        OUTPUT Line
        """,
    )
    assert process.returncode == 0, process.stderr
    assert process.stdout == "hello\n", process.stdout


def test_special_python_name_is_mangled(directory):
    process = convert_and_run(
        directory,
        """
        DECLARE __debug__ : INTEGER
        __debug__ <- 1
        OUTPUT __debug__
        """,
    )
    assert process.returncode == 0, process.stderr
    assert process.stdout == "1\n", process.stdout


def test_builtin_arity_is_not_silently_ignored(directory):
    process = convert_and_run(directory, 'OUTPUT LENGTH("a", "b")\n')
    assert process.returncode != 0
    assert "TypeError" in process.stderr, process.stderr


def test_empty_char_is_reported(directory):
    process = convert_and_run(
        directory,
        """
        DECLARE Character : CHAR
        INPUT Character
        OUTPUT Character
        """,
        stdin="\n",
    )
    assert process.returncode != 0
    assert "expected a character" in process.stderr, process.stderr


def main():
    tests = [
        ("basic", test_basic),
        ("array lower bound", test_array_lower_bound),
        ("real literal", test_real_literal_is_preserved),
        ("round", test_round_uses_decimal_places),
        ("boolean text", test_boolean_text),
        ("case", test_case_statement),
        ("string builtins", test_string_builtins),
        ("div/mod", test_div_and_mod_truncate),
        ("global after subprogram", test_global_declared_after_subprogram),
        ("global array lower bound", test_global_array_lower_bound_after_subprogram),
        ("openfile once", test_openfile_evaluates_name_once),
        ("array upper bound once", test_array_upper_bound_evaluated_once),
        ("ast output", test_ast_respects_output_option),
        ("output option", test_output_option_rejects_option_like_value),
        ("output equals", test_output_equals_form),
        ("end of options", test_end_of_options),
        ("directory input", test_directory_input_is_rejected),
        ("syntax error usage", test_syntax_error_includes_usage),
        ("no arguments", test_no_arguments_show_usage),
        ("stdin", test_stdin_input),
        ("file roundtrip", test_file_roundtrip),
        ("debug name", test_special_python_name_is_mangled),
        ("builtin arity", test_builtin_arity_is_not_silently_ignored),
        ("empty char", test_empty_char_is_reported),
    ]

    for name, test in tests:
        check(name, test)

    if FAILURES:
        print(f"\n{len(FAILURES)} test(s) failed: {', '.join(FAILURES)}")
        return 1

    print(f"\n{len(tests)} tests passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
