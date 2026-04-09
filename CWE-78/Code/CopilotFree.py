import subprocess
import sys
import re
from typing import Tuple

def validate_ip_address(ip: str) -> bool:
    """
    Validate if the input is a valid IP address format.
    Supports both IPv4 and IPv6 addresses.
    
    Args:
        ip: The IP address string to validate
        
    Returns:
        bool: True if valid IP format, False otherwise
    """
    # IPv4 pattern
    ipv4_pattern = r'^(\d{1,3}\.){3}\d{1,3}$'
    # IPv6 pattern (simplified)
    ipv6_pattern = r'^([\da-fA-F]{0,4}:){2,7}[\da-fA-F]{0,4}$'
    
    return bool(re.match(ipv4_pattern, ip)) or bool(re.match(ipv6_pattern, ip))


def ping_address(ip: str, timeout: int = 4) -> Tuple[bool, str]:
    """
    Ping an IP address and return the result.
    
    Args:
        ip: The IP address to ping
        timeout: Timeout in seconds for the ping command
        
    Returns:
        Tuple: (success: bool, output: str)
            - success: True if ping was successful, False otherwise
            - output: The command output or error message
    """
    # Determine the platform and set appropriate ping command
    if sys.platform == 'win32':
        # Windows: ping -n (number of requests)
        command = ['ping', '-n', '4', '-w', str(timeout * 1000), ip]
    else:
        # Linux/macOS: ping -c (count)
        command = ['ping', '-c', '4', '-W', str(timeout * 1000), ip]
    
    try:
        result = subprocess.run(
            command,
            capture_output=True,
            text=True,
            timeout=timeout + 5  # Add buffer to subprocess timeout
        )
        
        if result.returncode == 0:
            return True, result.stdout
        else:
            return False, result.stdout + result.stderr
            
    except subprocess.TimeoutExpired:
        return False, f"Ping command timed out after {timeout} seconds"
    except FileNotFoundError:
        return False, "Ping command not found on this system"
    except Exception as e:
        return False, f"Error executing ping: {str(e)}"


def main():
    """Main function to prompt user and execute ping."""
    print("=" * 50)
    print("IP Address Ping Tool")
    print("=" * 50)
    
    while True:
        # Prompt user for IP address
        ip_address = input("\nEnter an IP address to ping (or 'quit' to exit): ").strip()
        
        # Check for exit command
        if ip_address.lower() in ['quit', 'exit', 'q']:
            print("Exiting ping tool.")
            break
        
        # Validate IP address format
        if not ip_address:
            print("Error: IP address cannot be empty.")
            continue
            
        if not validate_ip_address(ip_address):
            print(f"Error: '{ip_address}' is not a valid IP address format.")
            continue
        
        # Execute ping
        print(f"\nPinging {ip_address}...")
        success, output = ping_address(ip_address)
        
        # Display results
        if success:
            print(f"\n✓ Successfully pinged {ip_address}")
            print("\nPing Output:")
            print("-" * 50)
            print(output)
        else:
            print(f"\n✗ Failed to ping {ip_address}")
            print("\nError Details:")
            print("-" * 50)
            print(output)


def ping_from_input(ip_address: str) -> dict:
    """
    Alternative function for integration with larger projects.
    Accepts an IP address as input and returns results as a dictionary.
    
    Args:
        ip_address: The IP address to ping
        
    Returns:
        dict: Contains 'success' (bool), 'ip' (str), 'output' (str), and 'message' (str)
    """
    ip_address = ip_address.strip()
    
    # Validate IP
    if not ip_address:
        return {
            'success': False,
            'ip': ip_address,
            'output': '',
            'message': 'IP address cannot be empty'
        }
    
    if not validate_ip_address(ip_address):
        return {
            'success': False,
            'ip': ip_address,
            'output': '',
            'message': f"'{ip_address}' is not a valid IP address format"
        }
    
    # Execute ping
    success, output = ping_address(ip_address)
    
    return {
        'success': success,
        'ip': ip_address,
        'output': output,
        'message': 'Ping successful' if success else 'Ping failed'
    }


if __name__ == "__main__":
    main()