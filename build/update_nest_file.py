import sys

if len(sys.argv) != 3:
    sys.exit(1)
path = sys.argv[1]
ext = sys.argv[2]

with open(path, 'r') as file:
    content = file.read() \
        .replace('../../x64/Debug/nest_', 'std') \
        .replace('.dll', '.' + ext)

with open(path, 'w') as file:
    file.write(content)
