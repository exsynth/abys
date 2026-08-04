import os
import subprocess

project = 'abys'
author = 'abys contributors'

extensions = [
    'sphinx.ext.viewcode',
    'breathe',
    'sphinx_rtd_theme',
]

exclude_patterns = ['_build']

html_theme = 'sphinx_rtd_theme'

# Run Doxygen on Sphinx build to populate XML for Breathe.
def run_doxygen():
    doxyfile = os.path.join(os.path.dirname(__file__), 'Doxyfile')
    subprocess.check_call(['doxygen', doxyfile])

run_doxygen()

breathe_projects = {
    'abys': 'doxyxml/xml'
}
breathe_default_project = 'abys'
