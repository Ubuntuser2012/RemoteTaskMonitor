"""Tests for distutils.command.bdist_rpm."""

import unittest
import sys
import os
import tempfile
import shutil
from test.support import run_unittest

from distutils.core import Distribution
from distutils.command.bdist_rpm import bdist_rpm
from distutils.tests import support
from distutils.spawn import find_executable
from distutils import spawn
from distutils.errors import DistutilsExecError

SETUP_PY = """\
from distutils.core import setup
import foo

setup(name='foo', version='0.1', py_modules=['foo'],
      url='xxx', author='xxx', author_email='xxx')

"""

class BuildRpmTestCase(support.TempdirManager,
                       support.LoggingSilencer,
                       unittest.TestCase):

    def setUp(self):
        super(BuildRpmTestCase, self).setUp()
        self.old_location = os.getcwd()
        self.old_sys_argv = sys.argv, sys.argv[:]

    def tearDown(self):
        os.chdir(self.old_location)
        sys.argv = self.old_sys_argv[0]
        sys.argv[:] = self.old_sys_argv[1]
        super(BuildRpmTestCase, self).tearDown()

    def test_quiet(self):

        # XXX I am unable yet to make this test work without
        # spurious sdtout/stderr output under Mac OS X
        if sys.platform != 'linux2':
            return

        # this test will run only if the rpm commands are found
        if (find_executable('rpm') is None or
            find_executable('rpmbuild') is None):
            return

        # let's create a package
        tmp_dir = self.mkdtemp()
        pkg_dir = os.path.join(tmp_dir, 'foo')
        os.mkdir(pkg_dir)
        self.write_file((pkg_dir, 'setup.py'), SETUP_PY)
        self.write_file((pkg_dir, 'foo.py'), '#')
        self.write_file((pkg_dir, 'MANIFEST.in'), 'include foo.py')
        self.write_file((pkg_dir, 'README'), '')

        dist = Distribution({'name': 'foo', 'version': '0.1',
                             'py_modules': ['foo'],
                             'url': 'xxx', 'author': 'xxx',
                             'author_email': 'xxx'})
        dist.script_name = 'setup.py'
        os.chdir(pkg_dir)

        sys.argv = ['setup.py']
        cmd = bdist_rpm(dist)
        cmd.fix_python = True

        # running in quiet mode
        cmd.quiet = 1
        cmd.ensure_finalized()
        cmd.run()

        dist_created = os.listdir(os.path.join(pkg_dir, 'dist'))
        self.assertTrue('foo-0.1-1.noarch.rpm' in dist_created)

        # bug #2945: upload ignores bdist_rpm files
        self.assertIn(('bdist_rpm', 'any', 'dist/foo-0.1-1.src.rpm'), dist.dist_files)
        self.assertIn(('bdist_rpm', 'any', 'dist/foo-0.1-1.noarch.rpm'), dist.dist_files)

    def test_no_optimize_flag(self):

        # XXX I am unable yet to make this test work without
        # spurious sdtout/stderr output under Mac OS X
        if sys.platform != 'linux2':
            return

        # http://bugs.python.org/issue1533164
        # this test will run only if the rpm command is found
        if (find_executable('rpm') is None or
            find_executable('rpmbuild') is None):
            return

        # let's create a package that brakes bdist_rpm
        tmp_dir = self.mkdtemp()
        pkg_dir = os.path.join(tmp_dir, 'foo')
        os.mkdir(pkg_dir)
        self.write_file((pkg_dir, 'setup.py'), SETUP_PY)
        self.write_file((pkg_dir, 'foo.py'), '#')
        self.write_file((pkg_dir, 'MANIFEST.in'), 'include foo.py')
        self.write_file((pkg_dir, 'README'), '')

        dist = Distribution({'name': 'foo', 'version': '0.1',
                             'py_modules': ['foo'],
                             'url': 'xxx', 'author': 'xxx',
                             'author_email': 'xxx'})
        dist.script_name = 'setup.py'
        os.chdir(pkg_dir)

        sys.argv = ['setup.py']
        cmd = bdist_rpm(dist)
        cmd.fix_python = True

        cmd.quiet = 1
        cmd.ensure_finalized()
        cmd.run()

        dist_created = os.listdir(os.path.join(pkg_dir, 'dist'))
        self.assertTrue('foo-0.1-1.noarch.rpm' in dist_created)

        # bug #2945: upload ignores bdist_rpm files
        self.assertIn(('bdist_rpm', 'any', 'dist/foo-0.1-1.src.rpm'), dist.dist_files)
        self.assertIn(('bdist_rpm', 'any', 'dist/foo-0.1-1.noarch.rpm'), dist.dist_files)

        os.remove(os.path.join(pkg_dir, 'dist', 'foo-0.1-1.noarch.rpm'))

def test_suite():
    return unittest.makeSuite(BuildRpmTestCase)

if __name__ == '__main__':
    run_unittest(test_suite())
