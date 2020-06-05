#!/usr/bin/env python
# coding: utf-8

# Recursively change dynamic library link paths for non-system libraries
# to be relative to the linked program and copy them to <program>/../libs
#
# TODO: parse individual architectures (necessary?)
#       modify temp lib instead of assuming path changes work
#
# Gist: https://gist.github.com/h0tw1r3/b12752f8e33f70422fbe
# Copyright: © 2015 Jeffrey Clark
# License: GPLv3 (http://www.gnu.org/licenses/gpl-3.0.html)

import os, stat, sys, subprocess, re, shutil

toolchain = os.getenv('TOOLCHAIN')
if toolchain == None:
	toolchain = ""
else:
	toolchain += "-"
otool = toolchain + "otool"
install_name_tool = toolchain + "install_name_tool"

def runOtool(filename):
	print otool + " -XL " + filename
	p = subprocess.Popen([otool, '-XL', filename], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
	return iter(p.stdout.readline, b'')

def fixname(filename, old, new):
	print install_name_tool + " -change " + old + " " + new + " " + filename
	p = subprocess.Popen([install_name_tool, '-change', old, new, filename], stdout=subprocess.PIPE)
	p.communicate()
	return

def fixid(filename, newid):
	print install_name_tool + " -id @loader_path/../libs/" + newid + " " + filename
	p = subprocess.Popen([install_name_tool, '-id', '@loader_path/../libs/' + newid, filename], stdout=subprocess.PIPE)
	p.communicate()
	return

def cmd_exists(cmd):
	return subprocess.call("type " + cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE) == 0

def mainloop(filename):
	for line in runOtool(filename):
		m = re.search('\s+/((usr|opt)/local/.*) \(.*\)$', line)
		if m and len(m.group(1)) != 0:
			path = os.path.realpath( os.path.join(os.getenv('LIB_BASE_PATH','/'),m.group(1)) )
			linkname = os.path.basename( path )
			if os.path.isfile( '../libs/' + linkname ) is False:
				try:
					shutil.copy2(path, '../libs/')
					os.chmod('../libs/' + linkname, stat.S_IRWXU | stat.S_IRGRP | stat.S_IROTH )
					fixid('../libs/' + linkname, linkname)
					mainloop('../libs/' + linkname)
				except IOError as e:
					print e
					continue
			fixname( filename, '/' + m.group(1), '@loader_path/../libs/' + linkname )
		else:
			m = re.search('\s+(@rpath(.*)) \(.*\)$', line)
			if m and len(m.group(2)) != 0:
				my_base = m.group(2)

				test = my_base.strip('/').split(".")[0]
				for b in os.listdir( os.path.join( os.getenv('LIB_BASE_PATH','/'), 'usr/local/lib' )):
					if ( b.find( test ) != -1 ):
						my_base = b
						break
				path = os.path.realpath(os.path.join(os.getenv('LIB_BASE_PATH','/'), 'usr/local/lib', my_base ))
				print path
				linkname = os.path.basename(path)
				fixname(filename, m.group(1), '@loader_path/../libs/' + linkname)

if (len(sys.argv) != 2):
	print "Usage: " + os.path.basename(sys.argv[0]) + " executable"
	sys.exit(1)
if (not cmd_exists(otool)):
	raise ValueError('Unable to execute otool: ' + otool)
if (not cmd_exists(install_name_tool)):
	raise ValueError('Unable to execute install_name_tool: ' + install_name_tool)
if (not os.path.isfile(sys.argv[1])):
	raise ValueError('File does not exist: ' + sys.argv[1])
if (not os.access(sys.argv[1], os.W_OK)):
	raise ValueError('Unable to write to file: ' + sys.argv[1])

mainloop(sys.argv[1])
