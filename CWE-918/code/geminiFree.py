from flask import Flask, request, Response
import requests

app = Flask(__name__)

@app.route('/proxy', methods=['GET'])
def proxy():
    # 1. Get the 'url' parameter from the query string
    target_url = request.args.get('url')

    if not target_url:
        return {"error": "No URL provided. Use ?url=http://example.com"}, 400

    try:
        # 2. Fetch the content from the provided URL
        # We use stream=True to handle larger files efficiently
        response = requests.get(target_url, timeout=10)

        # 3. Return the response body, status code, and headers
        # This ensures the client gets exactly what the target site sent
        return Response(
            response.content,
            status=response.status_code,
            content_type=response.headers.get('Content-Type')
        )

    except requests.exceptions.MissingSchema:
        return {"error": "Invalid URL format. Include http:// or https://"}, 400
    except requests.exceptions.ConnectionError:
        return {"error": "Could not connect to the target URL."}, 502
    except requests.exceptions.Timeout:
        return {"error": "The request to the target URL timed out."}, 504
    except Exception as e:
        return {"error": f"An unexpected error occurred: {str(e)}"}, 500

if __name__ == '__main__':
    app.run(debug=True)