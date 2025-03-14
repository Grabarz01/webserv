#!/usr/bin/php
<?php
// Wymuszenie buforowania wyjścia
ob_start();

// Wysyłamy poprawny status HTTP i nagłówki
echo "Status: 200 OK\r\n";
echo "Content-Type: text/html; charset=UTF-8\r\n";
echo "\r\n"; // Oddzielenie nagłówków od body

// Rozpoczynamy treść odpowiedzi
echo "<html lang='pl'>";
echo "<head><title>Odpowiedź z PHP</title></head>";
echo "<body>";

if ($_SERVER["REQUEST_METHOD"] == "POST") {
    // Pobieranie zmiennych POST
    $name = isset($_POST['name']) ? htmlspecialchars($_POST['name'], ENT_QUOTES, 'UTF-8') : 'Nie podano';
    $email = isset($_POST['email']) ? htmlspecialchars($_POST['email'], ENT_QUOTES, 'UTF-8') : 'Nie podano';

    echo "<h1>Otrzymane dane z formularza POST:</h1>";
    echo "<p>Imię: $name</p>";
    echo "<p>Email: $email</p>";
} else {
    echo "<p>Błędne zapytanie.</p>";
}

// Obsługa danych przesłanych przez stdin
$stdin_data = file_get_contents("php://stdin");

if (!empty($stdin_data)) {
    echo "<h2>Dane przesłane przez stdin:</h2>";
    echo "<pre>$stdin_data</pre>";

    // Parsowanie formatu application/x-www-form-urlencoded
    parse_str($stdin_data, $parsed_data);
    
    if (!empty($parsed_data)) {
        echo "<h3>Parsowane dane ze stdin:</h3>";
        echo "<p>Imię: " . htmlspecialchars($parsed_data['name'] ?? 'Nie podano', ENT_QUOTES, 'UTF-8') . "</p>";
        echo "<p>Email: " . htmlspecialchars($parsed_data['email'] ?? 'Nie podano', ENT_QUOTES, 'UTF-8') . "</p>";
    }
}

echo "</body></html>";

// Wypisanie pełnej odpowiedzi
ob_end_flush();
?>
