import platform
import subprocess
import ipaddress


def ping_ip(ip: str, count: int = 4, timeout_seconds: int = 2) -> int:
    """
    Ping an IP address and return the command's exit code.
    0 typically means success, non-zero means failure.

    Notes:
    - Uses OS-native ping.
    - Validates the input as an IPv4/IPv6 address before running.
    """
    # Validate as an actual IP address (not hostname)
    ipaddress.ip_address(ip)

    system = platform.system().lower()

    if system == "windows":
        # Windows:
        #   -n <count>   number of echo requests
        #   -w <ms>      timeout in milliseconds per reply
        cmd = ["ping", "-n", str(count), "-w", str(timeout_seconds * 1000), ip]
    else:
        # Linux/macOS:
        #   -c <count>   number of packets
        #   -W <sec>     timeout in seconds (Linux). On macOS it's different, but ping still works without it.
        # We'll include -c always, and add -W on Linux only.
        cmd = ["ping", "-c", str(count), ip]
        if system == "linux":
            cmd[1:1] = ["-W", str(timeout_seconds)]  # insert after 'ping'

    # Run without shell=True to avoid command injection risk.
    # Capture output so the caller can log/handle it if needed.
    result = subprocess.run(cmd, capture_output=True, text=True)
    print(result.stdout, end="")
    if result.stderr:
        print(result.stderr, end="")
    return result.returncode


def main() -> None:
    raw = input("Enter an IP address to ping: ").strip()

    try:
        exit_code = ping_ip(raw)
    except ValueError:
        print("Invalid IP address. Please enter a valid IPv4 or IPv6 address (e.g., 8.8.8.8).")
        return
    except FileNotFoundError:
        print("Ping command not found on this system.")
        return

    if exit_code == 0:
        print("\nPing succeeded.")
    else:
        print(f"\nPing failed (exit code {exit_code}).")


if __name__ == "__main__":
    main()