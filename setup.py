"""
Python DRPC package config
"""

from setuptools import setup

with open("README.md", "r", encoding="utf-8") as fh:
    long_description = fh.read()

setup(
    name='pydrpc',
    version='0.1.0',
    packages=['pydrpc'],
    author="Thomas Dokas",
    author_email="dokastho@umich.edu",
    url="https://github.com/dokastho/drpc",
    description="DRPC, now in Python! (this was all part of the plan)",
    long_description=long_description,
    long_description_content_type="text/markdown",
    include_package_data=True,
    install_requires=[],
    python_requires='>=3.6',
)
