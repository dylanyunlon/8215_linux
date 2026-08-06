#!/usr/bin/python


#
#
#   Collect oss target for do rootfs. Wrote by Ke.
#
#   If you have any suggestion, ply contact with me.

import sys, shutil, os, string

class OSS_Config:
  def __init__(self, package_name):
    self.package_name = package_name
    self.version = ''
    self.files = []

def getsize(path):
    if os.path.islink(path):
        return 0
    return os.path.getsize(path)

def getdirsize(dirpath):
    size = 0
    if os.path.islink(dirpath):
        return 0
    for root, dirs, files in os.walk(dirpath):
        size += sum([getsize(os.path.join(root, name)) for name in files])
    return size

def getallsizeofKB(path):
    size = 0
    if os.path.isdir(path):
        size = getdirsize(path)
    if os.path.islink(path):
        size = 0
    elif os.path.isfile(path):
        size = os.path.getsize(path)
    return size/1024


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
        if os.path.islink(dstname):
          os.remove(dstname)
        if os.path.isfile(dstname):
          os.remove(dstname)
        os.symlink(linkto, dstname)
      elif os.path.isdir(srcname):
        forcecopytree(srcname, dstname, symlinks)
      else:
        if os.path.isfile(dstname):
          os.remove(dstname)
        shutil.copy2(srcname, dstname)
    except (IOError, os.error) as why:
      errors.append((srcname, dstname, str(why)))
      raise BaseException(errors)
    except OSError as err:
      errors.extend(err.args[0])
      raise BaseException(errors)


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


def collect_oss_file():
  oss_configs = []

  topDir = os.environ.get('DA_TOP')
  outDir = os.environ.get('ROOTFS_OUT')
  ossDir = os.environ.get('OSS_TOP')
  prjDir = os.environ.get('PROJECT_DIR')
  oss_lib_dir = ossDir+'/lib'
  oss_src_dir = ossDir+'/source'
  oss_config_file = prjDir+'/collect_oss_files.config'

  is_package = 0
  package_cnt = 0
  has_version = 0

  f = open(oss_config_file, 'rb')

  try:
    for line in f:
      if line[0] == '#':
        continue

      if line.strip() == '':
        continue
    
      line = line.split('#')[0]

      if line.find('PACKAGE') == 0:
        if is_package == 1:
          print "error1"
          break;
        file_line = line.split(':')
        package_name = file_line[1]
        oss_configs.append(OSS_Config(package_name.strip()))
        package_cnt = package_cnt + 1
        is_package = 1
        continue

      if line.find('VERSION') == 0:
        if is_package == 0:
          print "error2"
          break
        file_line = line.split(':')
        oss_configs[package_cnt-1].version = file_line[1].strip()
        has_version = 1
        continue

      if line.find('END') == 0:
        is_package = 0
        has_version = 0
        continue

      if has_version == 1:
        oss_configs[package_cnt-1].files.append(line.strip())
  finally:
    f.close()  

  for s in oss_configs:
    src_path = oss_lib_dir+'/gnuarm-4.8.2_vfp/'+s.package_name+'/'+s.version
    print "PACKAGE: %s" % s.package_name
    for f in s.files:
      tmp = f.split(':')
      src_file = src_path+'/'+tmp[0]
      dst_file = outDir+'/'+tmp[1]
#      print "src_file %s, dst_file %s" % (src_file, dst_file)    
#      print "########################"
#      print "    srcfilesize: %d     " % (getallsizeofKB(src_file))
#      print "########################"
      if os.path.isdir(src_file):
        forcecopytree(src_file, dst_file, True)
      elif os.path.islink(src_file):
          linkto = os.readlink(src_file)
          if os.path.exists(dst_file):
            os.remove(dst_file)
#          print "what happen %s" % linkto
          os.symlink(linkto, dst_file)      
      else:
        try:
            if not os.path.isdir(os.path.dirname(dst_file)):
              os.makedirs(os.path.dirname(dst_file))
            shutil.copy(src_file,dst_file)
        except IOError,e:
          print "shutil.copy error:", e
          exit(1)

  else:
      print 'LOOP END'
      return 0

if __name__ == '__main__':
  collect_oss_file()


