import os
import sys

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print('usage: python validate_releasenote.py TARGET_FILE VERSION_NO')
        print('   TARGET_FILE : web/content/docs/release-note.ja.md')
        print('   VERSION_NO  : ex: 1.0.0')
        exit(1)

    base_dir: str = os.path.dirname(__file__)
    target_file: str = sys.argv[1]
    version_no: str = sys.argv[2]

    lines = []
    with open(target_file, 'r', encoding='utf-8') as fp:
        lines = fp.readlines()

    result = -1
    for line in lines:
        if not line.startswith('###'):
            continue
        items = line.split(' ')
        for item in items:
            if not item.startswith('v'):
                continue
            if item[1:] == version_no:
                result = 0
                print('OK : Latest version is %s.' % (version_no, ))
            else:
                result = 1
                print('NG : Latest version is %s. ' % (item[1:], ))
            break
        break
    if result == -1:
        print('Not found version no.')
        exit(2)
    else:
        exit(result)
