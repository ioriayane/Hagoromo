import sys
import os
import re
import glob
import shutil

def copyFile(folder: str, target_name: str, dest: str) -> None:
    files = glob.glob(os.path.join(folder, 'x64', 'Microsoft.VC143.CRT', target_name))
    if len(files) == 0:
        print('Not found ' + target_name)
        return
    print('copy... ' + files[0])
    shutil.copy(files[0], dest)

def main(src_folder: str, dest_folder: str) -> None:
    print(src_folder)
    print(dest_folder)

    targets = ['msvcp140.dll', 'msvcp140_1.dll', 'vcruntime140.dll', 'vcruntime140_1.dll']

    folders = [f for f in os.listdir(src_folder) if os.path.isdir(os.path.join(src_folder, f)) and re.search('^[0-9]+', f)]
    if len(folders) == 0:
        print('Not found target folder.')
        return

    for target in targets:
        copyFile(os.path.join(src_folder, sorted(folders)[-1]), target, dest_folder)

if __name__ == "__main__":

    if len(sys.argv) != 3:
        print('usage: python copymsvcfiles.py path/to/redist/msvc path/to/dest')
        print('   path/to/redist/msvc : "C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\VC\\Redist\\MSVC"')
        print('   path/to/dest        : deploy-hagoromo\\hagoromo')
        exit(1)

    src_folder: str = sys.argv[1]
    dest_folder: str = sys.argv[2]

    main(src_folder, dest_folder)
