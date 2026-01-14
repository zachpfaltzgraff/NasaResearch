import re

def main():
    print("Welcome to the Python Calculator!")
    expression = input("Enter a basic arithmetic expression (+, -, *, /): ")
    
    # Validate the expression to avoid unsafe code execution
    if re.match(r"^[0-9+\-*/.() ]+$", expression):
        try:
            result = eval(expression)
            print("The result is:", result)
        except Exception as e:
            print("Error in calculation:", e)
    else:
        print("Invalid expression. Please enter a valid basic arithmetic expression.")
        
    print("Goodbye!")

if __name__ == "__main__":
    main()