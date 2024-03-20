from flask import Flask, request
import requests

app = Flask(__name__)

TARGET_DOMAIN = "https://devapi.qweather.com"


@app.route('/<path:path>')
def forward_request(path):
    target_url = f"{TARGET_DOMAIN}/{path}?{request.query_string.decode('utf-8')}"
    print(target_url)
    # 发送请求到目标URL
    response = requests.get(target_url)
    print(response.json())
    # 直接返回目标服务器的响应数据
    return response.json()


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=6789)
