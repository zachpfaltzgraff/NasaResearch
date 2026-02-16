from __future__ import annotations

import ipaddress
import socket
from urllib.parse import urlparse

import requests
from flask import Flask, Response, jsonify, request

app = Flask(__name__)

# --- Basic SSRF guardrails (optional but recommended) -------------------------

BLOCK_PRIVATE_NETWORKS = True

def is_public_hostname(hostname: str) -> bool:
    """
    Resolve hostname and ensure it does not point to localhost/private/link-local ranges.
    """
    try:
        infos = socket.getaddrinfo(hostname, None)
    except socket.gaierror:
        return False

    for info in infos:
        ip_str = info[4][0]
        try:
            ip = ipaddress.ip_address(ip_str)
        except ValueError:
            return False

        # Block loopback, private, link-local, multicast, etc.
        if (
            ip.is_loopback
            or ip.is_private
            or ip.is_link_local
            or ip.is_multicast
            or ip.is_reserved
            or ip.is_unspecified
        ):
            return False

    return True

def validate_url(raw: str) -> tuple[bool, str | None]:
    """
    Validate and normalize user-provided URL.
    Returns (ok, error_message).
    """
    try:
        parsed = urlparse(raw)
    except Exception:
        return False, "Invalid URL."

    if parsed.scheme not in ("http", "https"):
        return False, "Only http and https URLs are allowed."

    if not parsed.netloc:
        return False, "URL must include a host."

    hostname = parsed.hostname
    if not hostname:
        return False, "URL host is invalid."

    if BLOCK_PRIVATE_NETWORKS and not is_public_hostname(hostname):
        return False, "Blocked: URL resolves to a non-public address."

    return True, None


# --- Proxy endpoint ----------------------------------------------------------

@app.get("/proxy")
def proxy():
    target = request.args.get("url", "").strip()
    if not target:
        return jsonify(error="Missing required query parameter: url"), 400

    ok, err = validate_url(target)
    if not ok:
        return jsonify(error=err), 400

    try:
        upstream = requests.get(
            target,
            timeout=(5, 20),  # (connect timeout, read timeout)
            allow_redirects=True,
            headers={
                # Optional: identify your proxy
                "User-Agent": "flask-proxy/1.0"
            },
        )
    except requests.exceptions.MissingSchema:
        return jsonify(error="Invalid URL format."), 400
    except requests.exceptions.InvalidURL:
        return jsonify(error="Invalid URL."), 400
    except requests.exceptions.Timeout:
        return jsonify(error="Upstream request timed out."), 504
    except requests.exceptions.ConnectionError:
        return jsonify(error="Failed to connect to upstream host."), 502
    except requests.exceptions.RequestException as e:
        return jsonify(error="Upstream request failed.", detail=str(e)), 502

    # Return upstream response body.
    # You can also pass through content-type for better client handling.
    content_type = upstream.headers.get("Content-Type", "application/octet-stream")

    return Response(
        upstream.content,
        status=upstream.status_code,
        content_type=content_type,
    )


@app.get("/health")
def health():
    return jsonify(status="ok")


if __name__ == "__main__":
    # Run: python app.py
    app.run(host="0.0.0.0", port=5000, debug=True)
