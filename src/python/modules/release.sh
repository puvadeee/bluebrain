sudo rm -fr dist build
python setup.py clean --all
python setup.py sdist bdist upload

