# social-app の providers.ts にある SUPPORT_PROVIDERS から
# lib/tools/supportprovider.cpp のプロバイダーテーブルを生成するツール
# https://github.com/bluesky-social/social-app/blob/main/src/features/profileLinks/providers.ts

# python3 ./scripts/update_support_providers.py
# python3 ./scripts/update_support_providers.py --check  # 差分があれば終了コード1

import os
import re
import sys
import argparse

BEGIN_MARKER = '// BEGIN SUPPORT_PROVIDERS'
END_MARKER = '// END SUPPORT_PROVIDERS'


def extract_array(source: str) -> str:
    """ SUPPORT_PROVIDERS 配列の中身を取り出す """
    match = re.search(r'export\s+const\s+SUPPORT_PROVIDERS\s*:[^=]*=\s*\[', source)
    if match is None:
        raise ValueError('SUPPORT_PROVIDERS is not found.')
    depth = 1
    pos = match.end()
    quote = ''
    while pos < len(source):
        c = source[pos]
        if quote:
            if c == '\\':
                pos += 1
            elif c == quote:
                quote = ''
        elif c in ('\'', '"', '`'):
            quote = c
        elif c == '[':
            depth += 1
        elif c == ']':
            depth -= 1
            if depth == 0:
                return source[match.end():pos]
        pos += 1
    raise ValueError('The end of SUPPORT_PROVIDERS is not found.')


def parse_string_value(item: str, key: str):
    match = re.search(r'\b' + key + r'\s*:\s*(\'(?:[^\'\\]|\\.)*\'|"(?:[^"\\]|\\.)*")', item)
    if match is None:
        return None
    value = match.group(1)[1:-1]
    return re.sub(r'\\(.)', r'\1', value)


def parse_providers(source: str) -> list:
    body = extract_array(source)
    # コメントを除去
    body = re.sub(r'/\*.*?\*/', '', body, flags=re.DOTALL)
    body = re.sub(r'^\s*//.*$', '', body, flags=re.MULTILINE)
    providers = []
    for item in re.findall(r'\{([^{}]*)\}', body):
        name = parse_string_value(item, 'name')
        domain = parse_string_value(item, 'domain')
        if name is None or domain is None:
            raise ValueError('name or domain is not found : ' + item.strip())
        providers.append({
            'name': name,
            'domain': domain.lower(),
            'path_prefix': parse_string_value(item, 'pathPrefix'),
        })
    if len(providers) == 0:
        raise ValueError('SUPPORT_PROVIDERS is empty.')
    return providers


def to_cpp_string(value: str) -> str:
    return '"' + value.replace('\\', '\\\\').replace('"', '\\"') + '"'


def make_table(providers: list, newline: str) -> list:
    lines = ['const ProviderRule SUPPORT_PROVIDERS[] = {' + newline]
    for provider in providers:
        path_prefix = 'nullptr'
        if provider['path_prefix'] is not None:
            path_prefix = to_cpp_string(provider['path_prefix'])
        lines.append('    { %s, %s, %s },%s' % (to_cpp_string(provider['name']),
                                                to_cpp_string(provider['domain']),
                                                path_prefix, newline))
    lines.append('};' + newline)
    return lines


def update_cpp(cpp_lines: list, providers: list) -> list:
    begin = -1
    end = -1
    for i, line in enumerate(cpp_lines):
        if line.startswith(BEGIN_MARKER):
            begin = i
        elif line.startswith(END_MARKER):
            end = i
    if begin < 0 or end < 0 or end < begin:
        raise ValueError('The markers are not found in the output file.')
    newline = '\r\n' if cpp_lines[begin].endswith('\r\n') else '\n'
    return cpp_lines[:begin + 1] + make_table(providers, newline) + cpp_lines[end:]


if __name__ == "__main__":
    base_dir: str = os.path.dirname(os.path.abspath(__file__))
    root_dir: str = os.path.dirname(base_dir)

    parser = argparse.ArgumentParser(description='Generate the support provider table from providers.ts.')
    parser.add_argument('--input', default=os.path.join(root_dir, '1stparty/social-app/src/features/profileLinks/providers.ts'),
                        help='path/to/providers.ts')
    parser.add_argument('--output', default=os.path.join(root_dir, 'lib/tools/supportprovider.cpp'),
                        help='path/to/supportprovider.cpp')
    parser.add_argument('--check', action='store_true',
                        help='do not write, exit with 1 if the output file is out of date')
    args = parser.parse_args()

    try:
        with open(args.input, 'r', encoding='utf-8') as fp:
            providers = parse_providers(fp.read())
        with open(args.output, 'r', encoding='utf-8', newline='') as fp:
            current_lines = fp.read().splitlines(keepends=True)
        updated_lines = update_cpp(current_lines, providers)
    except (OSError, ValueError) as e:
        print('Error : %s' % (e, ))
        sys.exit(2)

    if current_lines == updated_lines:
        print('OK : %s is up to date. (%d providers)' % (args.output, len(providers)))
        sys.exit(0)
    if args.check:
        print('NG : %s is out of date. Run scripts/update_support_providers.py.' % (args.output, ))
        sys.exit(1)

    with open(args.output, 'w', encoding='utf-8', newline='') as fp:
        fp.write(''.join(updated_lines))
    print('Updated : %s (%d providers)' % (args.output, len(providers)))
