#!/bin/bash
set -x
pip install twine
twine upload --username $TWINE_USERNAME --password $TWINE_PASSWORD --skip-existing wheelhouse/*
