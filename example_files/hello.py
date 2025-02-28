#!/usr/bin/python3
import sys
import cgi

# Odczytanie danych z formularza
form = cgi.FieldStorage()

# Pobranie wartości z formularza (jeśli dostępne)
name = form.getvalue('name', 'Guest')  # Jeśli nie ma wartości, użyj 'Guest' jako domyślnej

# Nagłówki odpowiedzi
print("Content-type:text/html\r\n\r\n")

# Generowanie odpowiedzi HTML
print('<html>')
print('<head>')
print('<title>Hello World - First CGI Program</title>')
print('</head>')
print('<body>')
print('<h2>Hello World! This is my first CGI program</h2>')
print(f'<p>Hello, {name}!</p>')  # Wyświetlanie danych z formularza
print('</body>')
print('</html>')
