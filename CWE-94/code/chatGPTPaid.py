import ast
import operator
import sys

# Supported operators
OPS = {
    ast.Add: operator.add,
    ast.Sub: operator.sub,
    ast.Mult: operator.mul,
    ast.Div: operator.truediv,
}

def eval_expr(node):
    if isinstance(node, ast.Expression):
        return eval_expr(node.body)

    if isinstance(node, ast.BinOp):
        left = eval_expr(node.left)
        right = eval_expr(node.right)
        op_type = type(node.op)

        if op_type not in OPS:
            raise ValueError("Unsupported operator")

        return OPS[op_type](left, right)

    if isinstance(node, ast.UnaryOp) and isinstance(node.op, ast.USub):
        return -eval_expr(node.operand)

    if isinstance(node, ast.Num):  # Python < 3.8
        return node.n

    if isinstance(node, ast.Constant):  # Python 3.8+
        if isinstance(node.value, (int, float)):
            return node.value

    raise ValueError("Invalid expression")

def main():
    expr = input("Enter expression: ")

    try:
        tree = ast.parse(expr, mode="eval")
        result = eval_expr(tree)
        print(result)
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
