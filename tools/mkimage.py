import argparse
from pathlib import Path
import struct
import sys

FILE_ENTRY_SIZE = 24

def main():
    parser = argparse.ArgumentParser(description='Generates a fs image')
    parser.add_argument("-o", dest="output", required=True, help="The output file.")
    parser.add_argument('dirs', nargs='+', help = 'files includeded into image.')
    args = parser.parse_args()

    dirs = args.dirs
    image = struct.pack('HHI', 8, len(dirs), 0)

    file_contents = bytes()
    file_off = len(image) + FILE_ENTRY_SIZE * len(dirs)

    for dir in dirs:
        name = str(Path(dir).stem)
        
        if len(name) >= 16:
            sys.exit(f"too long file name: {name}")

        path = dir + '/main.wasm'

        data = open(path, "rb").read()

        file_contents += data;
        image += struct.pack('16sII', name.encode("ascii"), file_off, len(data))
        file_off += len(data)

    image += file_contents
    
    with open(args.output, "wb") as f:
        f.write(image)
    
if __name__ == "__main__":
    main()
