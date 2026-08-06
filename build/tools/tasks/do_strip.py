#!/usr/bin/python

import sys, shutil, os, string
import commands


def delFile(filePath):
  print "file path: %s" % filePath
  if os.path.isdir(filePath):
    if os.path.exists(filePath):
      shutil.rmtree(filePath)
  elif os.path.isfile(filePath):
    if os.path.exists(filePath):
      os.remove(filePath)  

def do_strip(filename):
  if not os.path.exists(filename):
     print "%s: file not exist" % filename
     return
  
  surfix = os.path.splitext(filename)[1]
  if surfix == '.ko':
    return
  elif surfix == '.la' or surfix == '.lo' or surfix == '.a' or surfix == '.o':
    os.popen('rm -f '+filename)

  val = os.popen('file '+filename).read()
  if not val.find('not stripped') == -1:
    os.popen('armv7a-mediatek482_001_vfp-linux-gnueabi-strip  '+filename)


topDir = os.environ.get('DA_TOP')
outDir = os.environ.get('ROOTFS_OUT')

def do_allstrip(filepath):
  if os.path.isdir(filepath):
    names = os.listdir(filepath)
    
    dirname = os.path.basename(filepath)
    if dirname == 'include' or dirname == 'pkg-config' or dirname == 'man':
      print "dirname %s" % dirname
      delFile(filepath)
      return

    for name in names:
      do_allstrip(os.path.join(filepath,name))
  elif os.path.islink(filepath):
    return
  else:
    do_strip(filepath)

if __name__ == '__main__':
    do_allstrip(outDir)


