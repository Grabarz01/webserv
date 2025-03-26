#!/usr/bin/env python3

import cgi
import os

# Pobranie metody żądania
method = os.environ.get("REQUEST_METHOD", "GET")

# Tworzymy instancję form
form = cgi.FieldStorage()

# Sprawdzamy, jaką metodą jest żądanie
if method == "GET":
    firstname = form.getvalue("firstname", "Guest")
    print(f"""
    <html>
    <head><title>Response</title></head>
    <body style="font-family: Arial, sans-serif; background-color: #87CEEB; text-align: center;">
        <div style="background-color: rgb(215, 155, 15); padding: 20px; border-radius: 10px; box-shadow: 10px 4px 10px rgba(0, 0, 0, 0.3); display: inline-block;">
            <h1 style="color: white;">Hello, {firstname}!</h1>
			        <p style="color: white; font-size: 18px;">As I can see, you are an evaluator of Filip's webserv.  
        Thank you for your time!</p>
        <p style="color: white; font-size: 18px;">CGI: <strong style="color: lightgreen;">OK ✔</strong></p>
        </div>
    </body>
    </html>
    """)

elif method == "POST":
    firstname = form.getvalue("firstname", "Guest")
    feeling = form.getvalue("feeling", "No feelings shared.")

    print(f"""
    <html>
    <head><title>Response</title></head>
    <body style="font-family: Arial, sans-serif; background-color: #87CEEB; text-align: center;">
        <div style="background-color: rgb(215, 155, 15); padding: 20px; border-radius: 10px; box-shadow: 10px 4px 10px rgba(0, 0, 0, 0.3); display: inline-block;">
            <h1 style="color: white;">Hello, {firstname}!</h1>
            <p style="color: white;">You said: <strong>{feeling}</strong></p>
            <p style="color: white;"><em>It was nice to hear about your feelings!</em></p>
        </div>
    </body>
    </html>
    """)
