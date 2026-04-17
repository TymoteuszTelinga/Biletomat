# System obsługi biletomatu

system składający się z serwera i klienta umorzliwający rezerwację i zakup biletów.

### wymagania:
cmake,
conan

## urzytkowanie

w celu poprawnej inicjalizacji danych po stronie serwera i klienta pliki tickets.txt i cash.txt muszą znajdowac się w tej samej lokaliacji co pliki wykonywalne serwera i klienta. klient jak i serwer komunikaja się na adresie localhost:8080

## układ danych:
podczas uruchamiania serwera serwer wczytuje dane dotyczące biletów z pliku ```tickets.txt```
dane przechowywane są w postacji JSON, koszt biletu jest przechowyany w groszach
```json
[
  [
    0,
    {
      "Cost": 170,
      "Count": 8,
      "Name": "Ulgowy"
    }
  ],
  [
    1,
    {
      "Cost": 270,
      "Count": 3,
      "Name": "Stundencki"
    }
  ]
]
```

client na początku wczytuje stan kasy z pliku ```cash.txt``` jest to prosta tablica przechowywjąca liczbę monet z danego nominału, nominały są w kolejności [5zł, 2zł, 1zł, 50gr, 20gr, 10gr] na przykład:
```[0, 1, 3, 5, 0, 3]``` 
