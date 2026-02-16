from flask import Flask, request, jsonify
import requests
from urllib.parse import urlparse

app = Flask(__name__)

@app.route('/proxy', methods=['GET'])
def proxy():
    """
    Proxy endpoint that fetches the contents of a provided URL.
    
    Query Parameters:
        url (str): The URL to fetch contents from
    
    Returns:
        Response body from the fetched URL or error message
    """
    # Get the url query parameter
    target_url = request.args.get('url')
    
    # Validate that url parameter is provided
    if not target_url:
        return jsonify({
            'error': 'Missing required query parameter: url'
        }), 400
    
    # Validate URL format
    try:
        result = urlparse(target_url)
        if not all([result.scheme, result.netloc]):
            return jsonify({
                'error': 'Invalid URL format. URL must include scheme (http/https) and domain'
            }), 400
    except Exception as e:
        return jsonify({
            'error': f'Error parsing URL: {str(e)}'
        }), 400
    
    try:
        # Set a timeout to prevent hanging requests
        response = requests.get(target_url, timeout=10)
        
        # Raise an exception for bad status codes
        response.raise_for_status()
        
        # Return the response body
        return response.text, response.status_code, response.headers
    
    except requests.exceptions.Timeout:
        return jsonify({
            'error': 'Request timeout: The URL took too long to respond'
        }), 504
    
    except requests.exceptions.ConnectionError:
        return jsonify({
            'error': 'Connection error: Unable to connect to the provided URL'
        }), 502
    
    except requests.exceptions.HTTPError as e:
        return jsonify({
            'error': f'HTTP error: {str(e)}'
        }), response.status_code
    
    except requests.exceptions.RequestException as e:
        return jsonify({
            'error': f'Request failed: {str(e)}'
        }), 500
    
    except Exception as e:
        return jsonify({
            'error': f'Unexpected error: {str(e)}'
        }), 500

@app.route('/health', methods=['GET'])
def health():
    """Health check endpoint"""
    return jsonify({'status': 'ok'}), 200

if __name__ == '__main__':
    app.run(debug=True)