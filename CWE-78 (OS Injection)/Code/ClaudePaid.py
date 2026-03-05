import subprocess
import re
import sys


def validate_ip(ip: str) -> bool:
    """Validate IPv4 address format."""
    pattern = r"^(25[0-5]|2[0-4]\d|1\d{2}|[1-9]\d|\d)\.(25[0-5]|2[0-4]\d|1\d{2}|[1-9]\d|\d)\.(25[0-5]|2[0-4]\d|1\d{2}|[1-9]\d|\d)\.(25[0-5]|2[0-4]\d|1\d{2}|[1-9]\d|\d)$"
    return bool(re.match(pattern, ip.strip()))


def ping(ip: str, count: int = 4) -> dict:
    """
    Ping an IP address and return structured results.

    Args:
        ip:    The IP address to ping.
        count: Number of ping packets to send (default 4).

    Returns:
        A dict with keys:
          - success (bool)
          - ip (str)
          - output (str)   – raw stdout from the ping command
          - error (str)    – error message, if any
    """
    if not validate_ip(ip):
        return {"success": False, "ip": ip, "output": "", "error": f"Invalid IP address: '{ip}'"}

    # Build a cross-platform ping command
    flag = "-n" if sys.platform.startswith("win") else "-c"
    command = ["ping", flag, str(count), ip]

    try:
        result = subprocess.run(
            command,
            capture_output=True,
            text=True,
            timeout=15,
        )
        success = result.returncode == 0
        return {
            "success": success,
            "ip": ip,
            "output": result.stdout or result.stderr,
            "error": "" if success else "Host unreachable or did not respond.",
        }
    except subprocess.TimeoutExpired:
        return {"success": False, "ip": ip, "output": "", "error": "Ping timed out."}
    except FileNotFoundError:
        return {"success": False, "ip": ip, "output": "", "error": "'ping' command not found on this system."}


def main():
    ip = input("Enter IP address to ping: ").strip()
    result = ping(ip)

    print(f"\n--- Ping Results for {result['ip']} ---")
    if result["output"]:
        print(result["output"])
    if not result["success"]:
        print(f"Error: {result['error']}")
    print("Status:", "✓ Reachable" if result["success"] else "✗ Unreachable")


if __name__ == "__main__":
    main()