"""
Test script for the Flask proxy server.
Run this after starting the proxy server in another terminal.
"""
import requests
import json

BASE_URL = "http://localhost:5000"

def test_proxy():
    """Test the proxy endpoint with various scenarios."""
    
    print("=" * 60)
    print("Testing Flask Proxy Server")
    print("=" * 60)
    
    # Test 1: Valid URL - JSON response
    print("\n1. Testing with valid JSON endpoint...")
    try:
        response = requests.get(f"{BASE_URL}/proxy?url=https://httpbin.org/json")
        print(f"   Status: {response.status_code}")
        print(f"   Content-Type: {response.headers.get('Content-Type')}")
        print(f"   Response preview: {response.text[:100]}...")
    except Exception as e:
        print(f"   Error: {e}")
    
    # Test 2: Valid URL - HTML response
    print("\n2. Testing with HTML endpoint...")
    try:
        response = requests.get(f"{BASE_URL}/proxy?url=https://example.com")
        print(f"   Status: {response.status_code}")
        print(f"   Content-Type: {response.headers.get('Content-Type')}")
        print(f"   Response length: {len(response.text)} bytes")
    except Exception as e:
        print(f"   Error: {e}")
    
    # Test 3: Missing URL parameter
    print("\n3. Testing with missing URL parameter...")
    try:
        response = requests.get(f"{BASE_URL}/proxy")
        print(f"   Status: {response.status_code}")
        print(f"   Response: {response.json()}")
    except Exception as e:
        print(f"   Error: {e}")
    
    # Test 4: Invalid URL format
    print("\n4. Testing with invalid URL format...")
    try:
        response = requests.get(f"{BASE_URL}/proxy?url=not-a-valid-url")
        print(f"   Status: {response.status_code}")
        print(f"   Response: {response.json()}")
    except Exception as e:
        print(f"   Error: {e}")
    
    # Test 5: Non-existent domain
    print("\n5. Testing with non-existent domain...")
    try:
        response = requests.get(f"{BASE_URL}/proxy?url=https://this-domain-does-not-exist-12345.com")
        print(f"   Status: {response.status_code}")
        print(f"   Response: {response.json()}")
    except Exception as e:
        print(f"   Error: {e}")
    
    # Test 6: Root endpoint
    print("\n6. Testing root endpoint...")
    try:
        response = requests.get(f"{BASE_URL}/")
        print(f"   Status: {response.status_code}")
        print(f"   Response: {json.dumps(response.json(), indent=2)}")
    except Exception as e:
        print(f"   Error: {e}")
    
    print("\n" + "=" * 60)
    print("Tests completed!")
    print("=" * 60)

if __name__ == "__main__":
    print("\nMake sure the Flask server is running first!")
    print("Start it with: python proxy_app.py\n")
    
    input("Press Enter to run tests...")
    test_proxy()