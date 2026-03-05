import subprocess
import platform

def ping_ip(ip_address):
    # Determine the correct ping parameter based on OS
    if platform.system().lower() == "windows":
        param = "-n"
    else:
        param = "-c"

    try:
        # Run the ping command
        result = subprocess.run(
            ["ping", param, "4", ip_address],
            capture_output=True,
            text=True
        )

        # Print the output
        print(result.stdout)

    except Exception as e:
        print("Error running ping:", e)


def main():
    ip = input("Enter an IP address to ping: ")
    ping_ip(ip)


if __name__ == "__main__":
    main()