import sys

if len(sys.argv) != 3:
    sys.exit(1)
path = sys.argv[1]
ext = sys.argv[2]

header = """_cwd_.0 '/' == ? [
    '../../build/linux/linux_debug/std' = dbg_path
    '.so' = ext
] : [
    '../../build/windows/projects/nest/x64/Debug/nest_' = dbg_path
    '.dll' = ext
]

"""

with open(path, 'r') as file:
    content = file.read() \
        .replace(header, '') \
        .replace("('__C__:' dbg_path '", "'__C__:std") \
        .replace("' ext ><)", '.' + ext + "'")

with open(path, 'w') as file:
    file.write(content)
