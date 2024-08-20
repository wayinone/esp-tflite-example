#!/bin/sh

# The script is meant to be run under repo's root directory
set -e

DIRECTORY=$(cd `dirname $0` && pwd)

if [ ! -e "environment/bin/activate" ]; then
  echo "venv not exists"
  python3.10 -m venv ./environment
fi

. ./environment/bin/activate

python3.10 -m pip install --no-cache-dir  pip setuptools pypandoc pip-tools
python3.10 -m pip install -r ./src/requirements.txt