#!/usr/bin/php
<?php
header("Content-Type: text/html; charset=UTF-8");

$method = $_SERVER['REQUEST_METHOD'];

// Odczytanie danych z stdin tylko jeśli metoda to POST
$stdin_data = '';
if ($method == "POST") {
    $stdin_data = file_get_contents('php://stdin');
    parse_str($stdin_data, $_POST); // Zdekodowanie danych
}

if ($method == "GET") {
    // Jeśli metoda to GET, sprawdzamy QUERY_STRING w $_SERVER
    $query_string = isset($_SERVER['QUERY_STRING']) ? $_SERVER['QUERY_STRING'] : '';
    
    // Parsowanie parametrów QUERY_STRING do zmiennej $firstname
    parse_str($query_string, $query_params);
    $firstname = isset($query_params['firstname']) ? htmlspecialchars($query_params['firstname']) : "Guest";
    
    echo <<<HTML
    <html>
        <head><title>Response</title></head>
        <body style="font-family: Arial, sans-serif; background-color: #87CEEB; text-align: center;">
            <div style="background-color: rgb(215, 155, 15); padding: 20px; border-radius: 10px; 
                        box-shadow: 10px 4px 10px rgba(0, 0, 0, 0.3); display: inline-block;">
                <h1 style="color: white;">Hello, $firstname!</h1>
                <p style="color: white; font-size: 18px;">As I can see, you are an evaluator of Filips' webserv.  
                    Thank you for your time!</p>
                <p style="color: white; font-size: 18px;">CGI: <strong style="color: lightgreen;">OK ✔</strong></p>
            </div>
        </body>
    </html>
    HTML;
} elseif ($method == "POST") {
    // Jeśli metoda to POST, zaktualizuj dane i wyświetl odpowiedź
    $firstname = isset($_POST['firstname']) ? htmlspecialchars($_POST['firstname']) : "Guest";
    $feeling = isset($_POST['feeling']) ? htmlspecialchars($_POST['feeling']) : "No feelings shared.";
    echo <<<HTML
    <html>
    <head><title>Response</title></head>
    <body style="font-family: Arial, sans-serif; background-color: #87CEEB; text-align: center;">
        <div style="background-color: rgb(215, 155, 15); padding: 20px; border-radius: 10px; 
                    box-shadow: 10px 4px 10px rgba(0, 0, 0, 0.3); display: inline-block;">
            <h1 style="color: white;">Hello, $firstname!</h1>
            <p style="color: white;">You said: <strong>$feeling</strong></p>
            <p style="color: white;"><em>It was nice to hear about your feelings!</em></p>
        </div>
    </body>
    </html>
HTML;
} else {
    echo "Unsupported method.";
}
?>
