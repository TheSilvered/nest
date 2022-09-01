import sys

def main():
    if len(sys.argv) != 2:
        sys.exit(1)
    path = sys.argv[1]

    with open(path, 'r') as file:
        content = file.read().replace('..\\\\..\\\\x64\\\\Debug\\\\nest_', 'std')

    with open(path, 'w') as file:
        file.write(content)

    sys.exit(0)

if __name__ == '__main__':
    main()
