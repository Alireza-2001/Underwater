values = [0, 0, 0, 0, 0]

for _ in range(20):
    new_num = int(input("Enter Number"))

    new_list = [0, 0, 0, 0, 0]
    for i in range(4):
        new_list[i] = list[i + 1]
    new_list[4] = new_num

    values = new_list

    print()
