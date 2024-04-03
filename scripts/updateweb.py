import os
import sys
import datetime
from jinja2 import Template, Environment, FileSystemLoader

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

    environment = Environment(loader=FileSystemLoader(os.path.dirname(__file__), encoding='utf8'))
    template = environment.get_template('template/download_link.html.j2')
    params = {}
    params['version_no'] = version_no
    params['date_time'] = date_time

    with open(output_file, 'w', encoding='utf-8') as fp:
        fp.write(template.render(params))
