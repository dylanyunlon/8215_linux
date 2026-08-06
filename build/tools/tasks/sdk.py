#!/usr/bin/python

import sys, shutil, os, string

class SDK_Config:
  def __init__(self, module_name):
    self.module_name = module_name
    self.files = []

#
#
#  can copy a folder or signle file
def forcecopytree(src, dst, symlinks=False):
  names = os.listdir(src)
  if not os.path.isdir(dst):
    os.makedirs(dst)

  errors = []
  for name in names:
    srcname = os.path.join(src, name)
    dstname = os.path.join(dst, name)
    try:
      if symlinks and os.path.islink(srcname):
        linkto = os.readlink(srcname)
        os.symlink(linkto, dstname)
      elif os.path.isdir(srcname):
        forcecopytree(srcname, dstname, symlinks)
      else:
        if os.path.isdir(dstname):
          os.rmdir(dstname)
        elif os.path.isfile(dstname):
          os.remove(dstname)
        shutil.copy2(srcname, dstname)
    except (IOError, os.error) as why:
      errors.append((srcname, dstname, str(why)))
    except OSError as err:
      errors.extend(err.args[0])
  if errors:
    raise Error(errors)

#
#
#  can delete a folder or signle file
def delFile(filePath):
  print "file path: %s" % filePath
  if os.path.isdir(filePath):
    if os.path.exists(filePath):
      shutil.rmtree(filePath)
  elif os.path.isfile(filePath):
    if os.path.exists(filePath):
      os.remove(filePath)    

sdk_configs = []

topDir = os.environ.get('DA_TOP')
project = os.environ.get('PROJECT_DIR')
sdk_file_path = project + os.path.sep + 'SDK.config'
destDir = topDir + os.path.sep + 'SDK'

#  clean SDK folder
delFile(destDir)
os.mkdir(destDir)

is_module = 0
module_cnt = 0

f = open(sdk_file_path, 'rb')

try:
  for line in f:
    if line[0] == '#':
      continue

    if line.strip() == '':
      continue
    
    line = line.split('#')[0]

    if line.find('MODULE') == 0:
      if is_module == 1:
        print "error"
        break;
      module_line = line.split(':')
      module_name = module_line[1]
      sdk_configs.append(SDK_Config(module_name.strip()))
      module_cnt = module_cnt + 1
      is_module = 1
      continue

    if line.find('END') == 0:
      is_module = 0
      continue

    sdk_configs[module_cnt-1].files.append(line.strip())
finally:
  f.close()

for s in sdk_configs:
  dest_module_path = destDir+os.path.sep+s.module_name

  print "dest module path %s" % dest_module_path
  if not os.path.exists(dest_module_path):
    os.mkdir(dest_module_path)

  for p in s.files:
    src_file = topDir+os.path.sep+s.module_name+os.path.sep+p
    if not os.path.exists(src_file):
      print "error, file %s not exist" % src_file
      exit(1)
    if os.path.isdir(src_file):
      print "cp folder %s -> %s" % (src_file, dest_module_path+os.path.sep+p)
      forcecopytree(src_file, dest_module_path+os.path.sep+p)
      continue
    if os.path.isfile(src_file):
      dest_file = dest_module_path+os.path.sep+p
      dest_dir = os.path.dirname(dest_file)
      if not os.path.exists(dest_dir):
        os.makedirs(dest_dir)
      print "cp file %s -> %s" % (src_file, dest_file)
      shutil.copy(src_file, dest_file)
else:
    print 'LOOP END'

exit(0)






