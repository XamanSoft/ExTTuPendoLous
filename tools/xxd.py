# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Represent a file as a C++ constant string.
Usage:
python xxd.py VAR SOURCE DEST
"""
import sys
import os
sys.path.append(os.path.realpath(os.path.dirname(__file__)))
from jsmin import jsmin
def main():
    variable_name = sys.argv[1] if sys.argv[1][0]!='-' else sys.argv[1][1:]
    output_filename = sys.argv[2]
    input_text=""
    for input_filename in sys.argv[3:]:
		with open(input_filename) as input_file:
			input_text = input_text + " " + input_file.read()
    input_text = jsmin(input_text)
    hex_values = ['0x{0:02x}'.format(ord(char)) for char in input_text]
    const_declaration = 'unsigned char %s[] = {\n%s\n};\n' % (
        variable_name, ', '.join(hex_values))
    with open(output_filename, 'w') as output_file:
        output_file.write(const_declaration)
if __name__ == '__main__':
    sys.exit(main())