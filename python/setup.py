"""
use `MANIFEST.in` file to add arbitrary files to package
"""


from setuptools import setup, find_packages

# with open("README.md", "r") as f:
# 	long_description = f.read()

setup(
	name="xmat",
	version="0.0.1",
	packages=find_packages(),
	description="library for nympy.ndarray serialization to file or network",
	# long_description=long_description,
	long_description_content_type="text/markdown",
	keywords='serialization multi-dimensional-array',
	url="https://github.com/signalarray/xmat",
	project_urls={
		'Documentation': 'https://https://xmat.readthedocs.io',
	},
	author="Signal Array",
	author_email="signal.array@gmail.com",
	license="MIT",
	classifiers=[
		"License :: OSI Approved :: MIT License",
		"Programming Language :: Python :: 3.10",
		"Operating System :: OS Independent",
		"Topic :: File Formats"
	],
	install_requires=["numpy"],
	python_requires=">=3.10",
	entry_points={
		"console_scripts": [
			# "testpypilib-hello = testpypilib:function_uses_numpy",
		],
	},
)
