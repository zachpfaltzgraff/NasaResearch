from flask import Flask, request, Response, jsonify
import requests
from urllib.parse import urlparse

app = Flask(__name__)

@app.route('/proxy', methods=['GET'])
def proxy():
    url = request.args.get('url')

    if not url:
        return jsonify({"error": "Missing 'url' query parameter"}), 400

    # Basic URL validation
    parsed_url = urlparse(url)
    if not parsed_url.scheme or not parsed_url.netloc:
        return jsonify({"error": "Invalid URL"}), 400

    try:
        # Fetch content from the provided URL
        resp = requests.get(url, timeout=10)

        # Return response body with original content type
        return Response(
            resp.content,
            status=resp.status_code,
            content_type=resp.headers.get('Content-Type', 'text/plain')
        )

    except requests.exceptions.Timeout:
        return jsonify({"error": "Request timed out"}), 504

    except requests.exceptions.RequestException as e:
        return jsonify({"error": "Failed to fetch URL", "details": str(e)}), 502

    except Exception as e:
        return jsonify({"error": "Unexpected server error", "details": str(e)}), 500


if __name__ == '__main__':
    app.run(debug=True)