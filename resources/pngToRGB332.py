#!/usr/bin/env python
#
# Script to extract RGBA values from png files and save them to a binary file
# Requires Pillow (pip install Pillow)
#
import os
import fnmatch
import sys
from PIL import Image

# Check for correct number of arguments
if len(sys.argv) != 2:
    print("Usage: python extract_rgba.py <directory>")
    sys.exit(1)

# Get the directory to process from the command line argument
input_directory = sys.argv[1]

# Walk through the directory
for root, dirnames, filenames in os.walk(input_directory):
    for filename in fnmatch.filter(filenames, '*.png'):
        image_path = os.path.join(root, filename)
        print(f"Processing {image_path}...")

        # Open the image and convert to RGBA
        image = Image.open(image_path).convert('RGBA')

        # Get the RGBA values
        rgba_values = list(image.getdata())

        # Create a binary file to store the RGBA values
        rgba_bin_filename = os.path.splitext(image_path)[0] + '.rgba'
        with open(rgba_bin_filename, 'wb') as bin_file:
            for pixel in rgba_values:
                # Write each RGBA value as 4 bytes to the file
                bin_file.write(bytearray(pixel))

print("Extraction complete.")
