from flask import Flask, request, jsonify, Response
import requests

app = Flask(__name__)

@app.route('/proxy')
def proxy():
    """
    Proxy endpoint that fetches content from a provided URL.
    
    Query Parameters:
        url (str): The URL to fetch content from
    
    Returns:
        Response: The content from the requested URL or an error message
    """
    # Get the URL from query parameters
    target_url = request.args.get('url')
    
    # Validate that URL parameter is provided
    if not target_url:
        return jsonify({
            'error': 'Missing required parameter',
            'message': 'Please provide a "url" query parameter'
        }), 400
    
    try:
        # Fetch the content from the target URL
        response = requests.get(target_url, timeout=10)
        
        # Return the response with the same status code and content type
        return Response(
            response.content,
            status=response.status_code,
            content_type=response.headers.get('Content-Type', 'text/plain')
        )
    
    except requests.exceptions.Timeout:
        return jsonify({
            'error': 'Request timeout',
            'message': 'The request to the target URL timed out'
        }), 504
    
    except requests.exceptions.ConnectionError:
        return jsonify({
            'error': 'Connection error',
            'message': 'Could not connect to the target URL'
        }), 502
    
    except requests.exceptions.InvalidURL:
        return jsonify({
            'error': 'Invalid URL',
            'message': 'The provided URL is not valid'
        }), 400
    
    except requests.exceptions.RequestException as e:
        return jsonify({
            'error': 'Request failed',
            'message': str(e)
        }), 500
    
    except Exception as e:
        return jsonify({
            'error': 'Internal server error',
            'message': str(e)
        }), 500

if __name__ == '__main__':
    app.run(debug=True)