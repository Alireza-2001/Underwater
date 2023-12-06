import requests

from time import sleep

data_url = 'http://192.168.43.57:1000//library/v1.0/books'

while True:
    x = requests.get(data_url)

    print(x.text)
    sleep(0.5)