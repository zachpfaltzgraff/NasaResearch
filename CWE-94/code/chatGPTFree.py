#!/usr/bin/env python3
import ast
import operator

# Allowed operators
OPS = {
    ast.Add: operator.add,
    ast.Sub: operator.sub,
    ast.Mult: operator.mul,
    ast.Div: operator.truediv
}

def eval_expr(node):
    if isinstance(node, ast.BinOp):
        op_type = type(node.op)
        if op_type not in OPS:
            raise ValueError("Unsupported operator")
        return OPS[op_type](eval_expr(node.left), eval_expr(node.right))
    elif isinstance(node, ast.Num):      # Python ≤3.7
        return node.n
    elif isinstance(node, ast.Constant): # Python ≥3.8
        return node.value
    else:
        raise ValueError("Invalid expression")

def main():
    text = input("Enter expression: ")
    try:
        tree = ast.parse(text, mode='eval')
        result = eval_expr(tree.body)
        print(result)
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    main()
