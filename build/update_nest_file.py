import sys

if len(sys.argv) != 2:
    sys.exit(1)
path = sys.argv[1]

dbg_path = '../../build/windows/projects/nest/\\(_debug_)/Debug/nest_'
arch_str = "(_debug_arch_ 'x86' == ? '' : 'x64/') = arch\n"

with open(path, 'r') as file:
    content = file.read() \
        .replace('\r\n', '\n') \
        .replace('\r', '\n') \
        .replace(dbg_path, 'std') \
        .replace('.dll', '.cnest') \
        .replace(arch_str, '')

with open(path, 'w') as file:
    file.write(content)
