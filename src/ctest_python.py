#!/usr/bin/env python

import os
import re
import string
import time
import commands
import sys

def find_ctesttestfiles(dir):
    matches = []
    for root, dirnames, filenames in os.walk(dir):
        for filename in filenames:
            if filename == 'CTestTestfile.cmake':
                matches.append(os.path.join(root, filename))
    return matches

def extract_command_args(concat_command):
    retval = []
    in_string = False
    string = ''
    for ch in concat_command:
        if ch == '"':
            if in_string:
                retval.append(string)
                string = ''
                in_string = False
            else:
                in_string = True
        else:
            if in_string:
                string += ch
    return retval

class Test:
    def __init__(self, name, command, dir):
        self.name = name
        self.command = "PATH=.:$PATH; export PATH;" + command
        self.dir = dir

    def run(self, index, max_index):
        print "      Start:", self.name

        here = os.getcwd()
        os.chdir(self.dir)
        t0 = time.time()
        status, output = commands.getstatusoutput(self.command)
        t1 = time.time()
        os.chdir(here)
        beginning = "%2d/%2d" % (index, max_index) + ' '
        beginning += "Test #%d:" % index + ' '
        beginning += self.name
        end = ''
        if status:
            end += "***Failed   "
        else:
            end += "   Passed   "
        end += "%0.2f sec" % (t1 - t0)
        dots = '...................................................................'
        output_width = 78
        num_dots = output_width - len(beginning) - len(end)
        print "%s %s %s" % (beginning, dots[0:num_dots], end)
        return status, output

def extract_tests_subdirs(dir_name):
    tests = []
    subdirs = []
    ctesttestfile = os.path.join(dir_name, 'CTestTestfile.cmake')
    try:
        f = open(ctesttestfile, 'r')
        for line in f.readlines():
            match = re.match('add_test\((.*)\)', line, re.IGNORECASE)
            if match:
                split = match.group(1).split(' ')
                name = split[0]
                concat_command = string.join(split[1:])
                tests.append(Test(name, concat_command, os.path.dirname(ctesttestfile)))
            match = re.match('subdirs\((.*)\)', line, re.IGNORECASE)
            if match:
                split = match.group(1).split(' ')
                subdir_name = split[0]
                subdirs.append(os.path.join(dir_name, subdir_name))
    except IOError:
        print "ignored missing", ctesttestfile
    return tests, subdirs

def extract_all_tests(dir_name):
    tests = []
    subdirs = [dir_name]
    while subdirs != []:
        old_subdirs = subdirs
        subdirs = []
        for subdir in old_subdirs:
            new_tests, new_subdirs = extract_tests_subdirs(subdir)
            tests.extend(new_tests)
            subdirs.extend(new_subdirs)
    return tests

def create_unique_directory(directory):
    max_version = 99
    version = 0
    created_directory = "%s.%02d" % (directory, version)
    while os.path.isdir(created_directory):
        version += 1
        if version > max_version:
            sys.stderr.write('ctest_python.py: output directory index too high (max=%d)\n' % \
                max_version)
            sys.exit(1)
        created_directory = "%s.%02d" % (directory, version)
    os.mkdir(created_directory)
    return created_directory

def run_tests(tests):
    num_tests = len(tests)
    err_indices = []
    err_tests = []
    err_output_base = "ctest_python_logs"
    err_output_dir = None
    t0 = time.time()
    for index, test in zip(range(1,num_tests+1), tests):
        status, output = test.run(index, num_tests)
        if status != 0:
            err_indices.append(index)
            err_tests.append(test)
            if not err_output_dir:
                err_output_dir = create_unique_directory(err_output_base)            
            output_fname = '%03d_%s.out' % (index, test.name)
            output_path = os.path.join(err_output_dir, output_fname)
            open(output_path, 'w').write(output)
    t1 = time.time()
    num_errs = len(err_indices)
    print
    pass_percentage = 100.0*(num_tests - num_errs)/(1.0*num_tests)
    print '%0.1f%% tests passed, %d tests failed out of %d' % \
        (pass_percentage, num_errs, num_tests)
    print
    print 'Total Test time (real) = %0.2f sec' % (t1 - t0)
    print
    if num_errs > 0:
        print 'The following tests FAILED:'
        for index, test in zip(err_indices, err_tests):
            print '    % 3d' % index, '-', test.name
        print 'log files written in directory', err_output_dir
        
            
           
if __name__ == '__main__':
    tests = extract_all_tests('.')
    run_tests(tests)
