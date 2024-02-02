import os
import sys
import datetime
import requests

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print('usage: python updateweb.py OUTPUT_FILE VERSION_NO')
        print('   OUTPUT_FILE : path/to/shortcode_file')
        print('   VERSION_NO  : ex: 1.0.0')
        exit(1)

    base_dir: str = os.path.dirname(__file__)
    output_file: str = sys.argv[1]
    version_no: str = sys.argv[2]
    date_time: str = datetime.datetime.now().strftime('%Y/%m/%d')

    release_paage: str = 'https://github.com/ioriayane/Hagoromo/releases/tag/v%s' % (version_no, )

    res = requests.get(release_paage)
    if res.status_code != 200:
        print('Not found : %s' % (release_paage, ))
        exit(1)

    data = '<ul><li>'
    data += '<a href="%s">Version %s</a> (%s)' % (release_paage, version_no, date_time)
    data += '</li></ul>'

    with open(output_file, 'w', encoding='utf-8') as fp:
        fp.write(data)
 
