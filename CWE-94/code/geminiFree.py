import sys

def calculate():
    # Join arguments to handle expressions with or without spaces
    # e.g., "2 + 2" or "2+2"
    expression = "".join(sys.argv[1:])

    if not expression:
        print("Usage: python calc.py <expression>")
        print("Example: python calc.py 10 + 5")
        return

    try:
        # We use a controlled dictionary for safety if using eval
        # or simple replacement for basic operators.
        # For a simple CLI, we'll use a safe evaluation approach:
        result = eval(expression, {"__builtins__": None}, {})
        print(f"Result: {result}")
    except ZeroDivisionError:
        print("Error: You can't divide by zero.")
    except Exception as e:
        print(f"Error: Invalid expression. ({e})")

if __name__ == "__main__":
    calculate()