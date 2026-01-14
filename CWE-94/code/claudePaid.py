#!/usr/bin/env python3
"""
Simple command-line calculator utility.
Supports basic arithmetic operations: +, -, *, /
"""

import sys
import re


def is_safe_expression(expr):
    """
    Check if the expression contains only safe characters.
    Only allows digits, operators, parentheses, decimal points, and whitespace.
    """
    # Allow only numbers, operators, parentheses, decimal points, and whitespace
    safe_pattern = re.compile(r'^[\d+\-*/().\s]+$')
    return bool(safe_pattern.match(expr))


def calculate(expression):
    """
    Safely evaluate a mathematical expression.
    
    Args:
        expression: String containing the mathematical expression
        
    Returns:
        The calculated result
        
    Raises:
        ValueError: If the expression is invalid or unsafe
        ZeroDivisionError: If division by zero is attempted
    """
    # Remove whitespace
    expression = expression.strip()
    
    # Check if expression is empty
    if not expression:
        raise ValueError("Empty expression")
    
    # Validate the expression contains only safe characters
    if not is_safe_expression(expression):
        raise ValueError("Expression contains invalid characters")
    
    # Evaluate the expression
    try:
        result = eval(expression)
        return result
    except ZeroDivisionError:
        raise ZeroDivisionError("Cannot divide by zero")
    except SyntaxError:
        raise ValueError("Invalid expression syntax")
    except Exception as e:
        raise ValueError(f"Error evaluating expression: {str(e)}")


def main():
    """Main function to run the calculator."""
    # Check if expression was provided as command-line argument
    if len(sys.argv) > 1:
        expression = ' '.join(sys.argv[1:])
    else:
        # Prompt user for input
        try:
            expression = input("Enter an expression: ")
        except (EOFError, KeyboardInterrupt):
            print("\nExiting.")
            sys.exit(0)
    
    # Calculate and print result
    try:
        result = calculate(expression)
        print(result)
    except ValueError as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)
    except ZeroDivisionError as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()