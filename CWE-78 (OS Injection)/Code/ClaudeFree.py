import subprocess
import platform
import re
import sys


def validate_ip(ip: str) -> bool:
    """Validate IPv4 address format."""
    pattern = r"^(\d{1,3}\.){3}\d{1,3}$"
    if not re.match(pattern, ip):
        return False
    return all(0 <= int(octet) <= 255 for octet in ip.split("."))


def ping(ip: str, count: int = 4) -> dict:
    """
    Ping the given IP address.

    Args:
        ip:    Target IP address string.
        count: Number of ping packets to send (default 4).

    Returns:
        dict with keys:
            success  (bool)   – True if at least one reply was received
            output   (str)    – Raw stdout from the ping command
            error    (str)    – stderr / error message, empty on success
    """
    if not validate_ip(ip):
        return {
            "success": False,
            "output": "",
            "error": f"Invalid IP address: '{ip}'",
        }

    # Build the OS-appropriate ping command
    system = platform.system().lower()
    if system == "windows":
        cmd = ["ping", "-n", str(count), ip]
    else:
        # Linux / macOS
        cmd = ["ping", "-c", str(count), ip]

    try:
        result = subprocess.run(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            timeout=count * 5,   # generous per-packet timeout
        )
        success = result.returncode == 0
        return {
            "success": success,
            "output": result.stdout.strip(),
            "error": result.stderr.strip(),
        }

    except subprocess.TimeoutExpired:
        return {
            "success": False,
            "output": "",
            "error": f"Ping timed out after {count * 5} seconds.",
        }
    except FileNotFoundError:
        return {
            "success": False,
            "output": "",
            "error": "ping command not found on this system.",
        }


def main():
    # ------------------------------------------------------------------ #
    # Entry-point for interactive / piped use.                            #
    # When integrated into a larger project you can import ping() directly#
    # and skip this function entirely.                                    #
    # ------------------------------------------------------------------ #
    try:
        ip = input("Enter IP address to ping: ").strip()
    except EOFError:
        # Allows the script to be driven by piped input
        print("No input received.", file=sys.stderr)
        sys.exit(1)

    if not ip:
        print("No IP address provided.", file=sys.stderr)
        sys.exit(1)

    print(f"\nPinging {ip} ...\n")
    result = ping(ip)

    if result["output"]:
        print(result["output"])

    if result["error"]:
        print(f"Error: {result['error']}", file=sys.stderr)

    if result["success"]:
        print(f"\n✅  {ip} is reachable.")
    else:
        print(f"\n❌  {ip} is unreachable or invalid.")

    sys.exit(0 if result["success"] else 1)


if __name__ == "__main__":
    main()