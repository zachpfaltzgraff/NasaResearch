from flask import Flask, request, Response
import requests

app = Flask(__name__)


@app.route("/proxy")
def proxy():
    url = request.args.get("url")

    if not url:
        return {"error": "Missing required 'url' query parameter"}, 400

    try:
        resp = requests.get(url, timeout=30)
        excluded_headers = {"content-encoding", "transfer-encoding", "connection"}
        headers = {
            k: v
            for k, v in resp.headers.items()
            if k.lower() not in excluded_headers
        }
        return Response(resp.content, status=resp.status_code, headers=headers)

    except requests.exceptions.MissingSchema:
        return {"error": f"Invalid URL: '{url}'. Include the scheme (e.g. https://)"}, 400
    except requests.exceptions.ConnectionError:
        return {"error": f"Could not connect to '{url}'"}, 502
    except requests.exceptions.Timeout:
        return {"error": f"Request to '{url}' timed out"}, 504
    except requests.exceptions.RequestException as e:
        return {"error": f"Request failed: {str(e)}"}, 500


if __name__ == "__main__":
    app.run(debug=True)