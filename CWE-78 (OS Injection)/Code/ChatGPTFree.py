import subprocess
import platform
import ipaddress

def ping_ip(ip):
    """
    Pings the given IP address and prints the result.
    """
    try:
        # Validate IP address format
        ipaddress.ip_address(ip)
    except ValueError:
        print("Invalid IP address format.")
        return

    # Determine OS-specific ping command
    system_name = platform.system().lower()
    
    if system_name == "windows":
        command = ["ping", "-n", "4", ip]
    else:
        # Linux / macOS
        command = ["ping", "-c", "4", ip]

    try:
        result = subprocess.run(command, capture_output=True, text=True)
        print(result.stdout)
    except Exception as e:
        print(f"Error executing ping: {e}")

def main():
    ip = input("Enter an IP address to ping: ").strip()
    ping_ip(ip)

if __name__ == "__main__":
    main()