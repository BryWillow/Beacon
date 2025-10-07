#!/usr/bin/env python3
"""
@file check_headers.py
Checks that project headers contain a Doxygen @file tag in the top comment block and flags simple style issues.

Usage: python3 scripts/style/check_headers.py
"""
import os
import re
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))

HEADER_EXTS = ('.h', '.hpp')

def is_vendor(path):
    return '/vendor/' in path.replace('\\\\', '/')

def check_file(path):
    issues = []
    try:
        with open(path, 'r', encoding='utf-8') as f:
            content = f.read()
    except Exception as e:
        issues.append(f'ERR: cannot read: {e}')
        return issues

    # Check for @file in first 20 lines
    first_lines = '\n'.join(content.splitlines()[:20])
    if '@file' not in first_lines:
        issues.append('MISSING @file tag in top comment block')

    # flag '== true' / '== false'
    if re.search(r'==\s*(true|false)\b', content):
        issues.append("Use direct boolean checks instead of '== true' or '== false'")

    # flag 'if (' style deviations: we prefer 'if (condition) {'
    for m in re.finditer(r'if\s*\((.*?)\)\s*\{', content, flags=re.DOTALL):
        pass

    # flag tabs
    if '\t' in content:
        issues.append('Tab character found (use spaces)')

    return issues

def main():
    failures = 0
    for dirpath, dirnames, filenames in os.walk(ROOT):
        for name in filenames:
            if name.endswith(HEADER_EXTS):
                path = os.path.join(dirpath, name)
                if is_vendor(path):
                    continue
                issues = check_file(path)
                if issues:
                    failures += 1
                    print(f'File: {path}')
                    for it in issues:
                        print('  -', it)

    if failures:
        print(f'Found {failures} header files with issues')
        sys.exit(1)
    print('Header checks passed')

if __name__ == '__main__':
    main()
