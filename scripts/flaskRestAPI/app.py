from pyspiceql import toUpper, toLower
from flask import Flask

app = Flask(__name__)

@app.route("/")
def spiceToUpper():
    return toUpper("hello i am a string")


if __name__ == "__main__":
    app.run()
