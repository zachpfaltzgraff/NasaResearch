#!/usr/bin/env python3
"""
calc.py — simple, safe arithmetic expression evaluator

Usage:
  python calc.py "2 + 3 * (4 - 1)"
  python calc.py          # will prompt for an expression

Supports: +, -, *, / and unary +/-. Only numeric literals and these operators are allowed.
"""
import argparse
import ast
import operator
import sys
import numbers

# Mapping AST operator types to python operator functions
BIN_OPS = {
    ast.Add: operator.add,
    ast.Sub: operator.sub,
    ast.Mult: operator.mul,
    ast.Div: operator.truediv,
}

UNARY_OPS = {
    ast.UAdd: operator.pos,
    ast.USub: operator.neg,
}

def eval_node(node):
    """Recursively evaluate an AST node, permitting only simple arithmetic."""
    # Binary operations: left <op> right
    if isinstance(node, ast.BinOp):
        op_type = type(node.op)
        if op_type not in BIN_OPS:
            raise ValueError(f"Unsupported binary operator: {op_type.__name__}")
        left = eval_node(node.left)
        right = eval_node(node.right)
        return BIN_OPS[op_type](left, right)

    # Unary operations: +operand, -operand
    if isinstance(node, ast.UnaryOp):
        op_type = type(node.op)
        if op_type not in UNARY_OPS:
            raise ValueError(f"Unsupported unary operator: {op_type.__name__}")
        operand = eval_node(node.operand)
        return UNARY_OPS[op_type](operand)

    # Numeric literal (supports ast.Constant for newer versions and ast.Num for older)
    if isinstance(node, ast.Constant):
        if isinstance(node.value, numbers.Number):
            return node.value
        raise ValueError("Only numeric constants are allowed")
    if isinstance(node, ast.Num):  # fallback for older Pythons
        return node.n

    # Parentheses are represented by nested nodes, so there's no separate node type needed.

    raise ValueError(f"Unsupported expression element: {type(node).__name__}")

def eval_expr(expr_text):
    """Parse and safely evaluate an arithmetic expression string."""
    try:
        parsed = ast.parse(expr_text, mode="eval")
    except SyntaxError as e:
        raise ValueError(f"Syntax error in expression: {e}")

    # Ensure top-level node is Expression
    if not isinstance(parsed, ast.Expression):
        raise ValueError("Only expressions are allowed")

    return eval_node(parsed.body)

def main(argv=None):
    parser = argparse.ArgumentParser(description="Evaluate a simple arithmetic expression (+, -, *, /).")
    parser.add_argument("expression", nargs="?", help="Expression to evaluate, e.g. '2 + 3 * (4 - 1)'")
    args = parser.parse_args(argv)

    if args.expression is None:
        try:
            expr = input("Enter expression: ").strip()
        except (EOFError, KeyboardInterrupt):
            print()  # newline on Ctrl-D / Ctrl-C
            return 1
    else:
        expr = args.expression

    if not expr:
        print("No expression provided.", file=sys.stderr)
        return 2

    try:
        result = eval_expr(expr)
    except ZeroDivisionError:
        print("Error: division by zero", file=sys.stderr)
        return 3
    except ValueError as e:
        print(f"Error: {e}", file=sys.stderr)
        return 4
    except Exception as e:
        print(f"Unexpected error: {e}", file=sys.stderr)
        return 5

    # Print result (use default formatting)
    print(result)
    return 0

if __name__ == "__main__":
    sys.exit(main())