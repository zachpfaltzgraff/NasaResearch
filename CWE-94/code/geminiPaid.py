import argparse
import sys

def main():
    # Set up the command-line argument parser
    parser = argparse.ArgumentParser(description="A simple CLI calculator.")
    parser.add_argument(
        "expression", 
        type=str, 
        help="The math expression to evaluate (e.g., '2 + 2')"
    )

    args = parser.parse_args()

    try:
        # Evaluate the string expression
        # Note: eval() can be dangerous; use only for trusted local inputs.
        result = eval(args.expression)
        print(f"Result: {result}")
    except ZeroDivisionError:
        print("Error: Division by zero is not allowed.")
        sys.exit(1)
    except Exception as e:
        print(f"Error: Invalid expression or characters. ({e})")
        sys.exit(1)

if __name__ == "__main__":
    main()