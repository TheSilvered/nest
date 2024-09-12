ENCODINGS = [
    "ascii",
    "cp1250",
    "cp1251",
    "cp1252",
    "cp1253",
    "cp1254",
    "cp1255",
    "cp1256",
    "cp1257",
    "cp1258"
]

UNICODE_ENCODINGS = [
    ("utf_8", r"\xef\xbb\xbf"),
    ("utf_16_le", r"\xff\xfe"),
    ("utf_16_be", r"\xfe\xff"),
    ("utf_32_le", r"\xff\xfe\x00\x00"),
    ("utf_32_be", r"\x00\x00\xfe\xff")
]

def ch_range(cp):
    return "".join(chr(i) for i in range(cp - 5, cp + 6))

UNICODE_STRING = ch_range(0x80) + ch_range(0x800) + ch_range(0x10000)


def write_string(encoding, string, strings_file):
    strings_file.write(f"{string!a} \\\n    = {encoding}_text\n")


def gen_file(strings_file, encoding):
    with open(f"file_{encoding}.txt", "wb") as f:
        string = ""
        for i in range(256):
            try:
                string += i.to_bytes().decode(encoding)
                f.write(i.to_bytes())
            except UnicodeDecodeError:
                pass
        write_string(encoding, string, strings_file)


def gen_latin1_file(strings_file):
    with open("file_latin1.txt", "wb") as f:
        string = ""
        for i in range(0x20, 0x7f):
            string += i.to_bytes().decode("latin1")
            f.write(i.to_bytes())
        for i in range(0xa0, 0x100):
            string += i.to_bytes().decode("latin1")
            f.write(i.to_bytes())
        write_string("latin1", string, strings_file)


def gen_unicode_file(strings_file, encoding, bom):
    nest_encoding = encoding.replace("_", "")
    with open(f"file_{nest_encoding}.txt", "wb") as f:
        f.write(UNICODE_STRING.encode(encoding))

    with open(f"file_{nest_encoding}_bom.txt", "wb") as f:
        f.write(UNICODE_STRING.encode(encoding))


def main():
    with open("strings.nest", "w", encoding="utf8") as f:
        for encoding in ENCODINGS:
            gen_file(f, encoding)
        gen_latin1_file(f)
        for encoding, bom in UNICODE_ENCODINGS:
            gen_unicode_file(f, encoding, bom)
        write_string("unicode", UNICODE_STRING, f)


if __name__ == "__main__":
    main()
