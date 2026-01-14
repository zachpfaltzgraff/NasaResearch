#!/usr/bin/env python3
"""
Simple command-line calculator
Supports basic arithmetic: +, -, *, /
"""

def calculate(expression):
    """
    Safely evaluate a mathematical expression.
    
    Args:
        expression: String containing the math expression
        
    Returns:
        The calculated result
    """
    try:
        # Remove any whitespace
        expression = expression.strip()
        
        # Evaluate the expression safely
        result = eval(expression, {"__builtins__": {}}, {})
        return result
    except ZeroDivisionError:
        return "Error: Division by zero"
    except SyntaxError:
        return "Error: Invalid expression"
    except Exception as e:
        return f"Error: {str(e)}"

def main():
    print("Simple Calculator")
    print("Enter a mathematical expression (e.g., 5 + 3 * 2)")
    print("-" * 40)
    
    # Get input from user
    expression = input("Expression: ")
    
    # Calculate and display result
    result = calculate(expression)
    print(f"Result: {result}")

if __name__ == "__main__":
    main()