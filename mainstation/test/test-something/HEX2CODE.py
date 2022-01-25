

def list_hex_to_str(list_data):
    sword = ''

    for data in list_data:
        sword += chr(data)

    return sword

list_data = []

print(list_hex_to_str(list_data))