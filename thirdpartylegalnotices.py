#!/usr/bin/env python3

import os

# It is quite annoying to have to write all the dependency notices
# by hand, so this script goes through the dependency list and generates
# a file with all the licenses automatically during the build process

def read_description_file(subdirectory):
    description_filename = "DESCRIPTION"
    description_extensions = [ "", ".txt", ".md", ".rst" ]
    for extension in description_extensions:
        fill_path = os.path.join(subdirectory, description_filename + extension)
        if os.path.exists(fill_path):
            with open(fill_path, 'r', encoding='utf-8') as file:
                return file.read().rstrip().splitlines()[0]
    return None

def read_license_file(subdirectory):
    license_filenames = [ "LICENSE", "COPYING", "NOTICE" ]
    license_extensions = [ "", ".txt", ".md", ".rst" ]
    for filename in license_filenames:
        for extension in license_extensions:
            fill_path = os.path.join(subdirectory, filename + extension)
            if os.path.exists(fill_path):
                with open(fill_path, 'r', encoding='utf-8') as file:
                    return file.read().rstrip()
    return None

def get_dependencies(directory):
    dependencies = []
    for subdirectory in os.listdir(directory):
        subdirectory_path = os.path.join(directory, subdirectory)
        if os.path.isdir(subdirectory_path):
            license = read_license_file(subdirectory_path)
            description = read_description_file(subdirectory_path)
            if description and len(description) > 80:
                description = description[:77] + "..."
            if license:
                dependencies.append((subdirectory, license, description))
    return dependencies

assert len(os.sys.argv) > 1, "Please provide the path to the dependencies directory."
dependencies_dir = os.sys.argv[1]
dependencies = get_dependencies(dependencies_dir)
assert len(dependencies) > 0, "No licenses found in the dependencies directory."

with open("thirdpartylegalnotices.txt", 'w', encoding='utf-8') as out_file:
    out_file.write("Voxelius uses third-party code for certain functions. All the\n")
    out_file.write("license texts are included below using an automated script; this generated\n")
    out_file.write("file is to be included in binary distributions of the project.\n\n")

    for name, license, description in dependencies:
        out_file.write(f"{'=' * 80}\n")
        if description and len(description) > 0:
            out_file.write(f"{name} - {description}\n")
        else:
            out_file.write(f"{name}\n")
        out_file.write(f"{'=' * 80}\n")
        out_file.write(f"{license}\n\n\n")
