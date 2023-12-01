h = 15
if h >= 0:
    if h <= 90:
        h = 90 - h
    else:
        h = h - 90
        h = -1*h
else:
    if h<0:
        h = 90 + (-1*h)
    else:
        h = (-180-h) + -90
print(h)