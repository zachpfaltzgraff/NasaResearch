from flask import Flask, request, Response
import requests

app = Flask(__name__)

@app.route('/proxy', methods=['GET'])
def proxy():
    # 1. Get the target URL from the query parameter
    target_url = request.args.get('url')

    if not target_url:
        return {"error": "Missing 'url' query parameter"}, 400

    try:
        # 2. Fetch the content from the provided URL
        # We use stream=True to handle larger payloads efficiently
        response = requests.get(target_url, timeout=10)

        # 3. Handle errors gracefully (e.g., 404 or 500 from the target)
        response.raise_for_status()

        # 4. Return the response body and content-type to the client
        return Response(
            response.content,
            status=response.status_code,
            content_type=response.headers.get('Content-Type')
        )

    except requests.exceptions.HTTPError as http_err:
        return {"error": f"HTTP error occurred: {http_err}"}, response.status_code
    except requests.exceptions.RequestException as err:
        # Catch-all for connection issues, timeouts, or DNS failures
        return {"error": f"An error occurred: {err}"}, 500

if __name__ == '__main__':
    app.run(debug=True)