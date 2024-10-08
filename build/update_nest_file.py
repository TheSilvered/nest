import sys

if len(sys.argv) != 2:
    sys.exit(1)
path = sys.argv[1]

dbg_path = "../../build/windows/projects/nest/\\(_debug_arch_ 'x86' == ? '' : 'x64/')Debug/nest_"

with open(path, 'r') as file:
    content = file.read() \
        .replace(dbg_path, 'std') \
        .replace('.dll', '.cnest')

with open(path, 'w') as file:
    file.write(content)
