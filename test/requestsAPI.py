import requests
from time import sleep

while True:
    x = requests.get('http://192.168.43.57:1000//library/v1.0/books')

    print(x.text)
    sleep(0.5)