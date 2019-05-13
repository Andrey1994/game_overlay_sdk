import os
from setuptools import setup, find_packages

this_directory = os.path.abspath (os.path.dirname (__file__))
with open (os.path.join (this_directory, 'README.md'), encoding = 'utf-8') as f:
    long_description = f.read ()

setup (
    name = 'game_overlay_sdk',
    version = '1.0.0',
    description = 'Library to draw overlay on top of game',
    long_description = long_description,
    long_description_content_type = 'text/markdown',
    url = 'https://github.com/Andrey1994/game_overlay_sdk',
    author = 'Andrey Parfenov',
    author_email = 'a1994ndrey@gmail.com',
    packages = find_packages (),
    classifiers = [
        'Development Status :: 2 - Pre-Alpha',
        'Topic :: Utilities'
    ],
    install_requires = ['numpy'],
    package_data = {
        'game_overlay_sdk': [
            os.path.join ('lib', 'DLLInjection64.dll'),
            os.path.join ('lib', 'GameOverlay64.dll'),
            os.path.join ('lib', 'VK_LAYER_OCAT_overlay64.dll'),
            os.path.join ('lib', 'DLLInjection32.dll'),
            os.path.join ('lib', 'GameOverlay32.dll'),
            os.path.join ('lib', 'VK_LAYER_OCAT_overlay32.dll'),
            os.path.join ('lib', 'shader.comp'),
            os.path.join ('lib', 'shader.frag'),
            os.path.join ('lib', 'shader.vert'),
            os.path.join ('lib', 'comp.spv'),
            os.path.join ('lib', 'frag.spv'),
            os.path.join ('lib', 'vert.spv')
        ]
    },
    zip_safe = True,
    python_requires = '>=3'
)
