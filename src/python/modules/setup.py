from distutils.core import setup


setup(
	name='Cannybots',
	version='0.9dev',
	packages=['cannybots','cannybots.clients'],
	license='MIT license',
	author="Wayne Keenan",
	author_email="wayne@cannybots.com",
	url="http://www.cannybots.com/",
	long_description=open('README.txt').read(),
	install_requires=[
		'websocket-client',
		'scratchpy'
	],
)
