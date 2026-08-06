#!/usr/bin/python

import sys, shutil, os, string, sys

ossdir = os.environ.get('OSS_TOP')+'/source'

class Patch_Manage:
	def __init__(self, module_name, version):
		self.module_name    = module_name  # module name
		self.version        = version      # module version
		self.patch_files    = []           # all patch files
		self.patch_count    = 0            # all patch count
		self.patch_index    = 0            # patch index
		self.patch_config   = ''           # patch config file 

	def __del__(self):
		print "__del__" 

	def get_module_path(self):
		return ossdir+'/'+self.module_name+'/'+self.version

	def set_patch_flag(self):
		os.system("cd "+self.get_module_path()+" && touch "+self.module_name+".patch.done")

	def clr_patch_flag(self):
		os.system("cd "+self.get_module_path()+" && rm -f "+self.module_name+".patch.done")

	def get_patch_flag(self):
		if os.path.exists(self.get_module_path()+'/'+self.module_name+".patch.done"):
			return True
		else:
			return False		

	def exe_one_patch(self, patch):
		print "add patch: %s" % patch
		os.system("cd "+self.get_module_path()+'/'+self.module_name+'-'+self.version+" && "+"patch -Np1 < "+" ../"+patch)

	def reverse_one_patch(self, patch):
		print "reverse patch: %s " % patch
		os.system("cd "+self.get_module_path()+'/'+self.module_name+'-'+self.version+" && "+"patch -Rp1 < "+" ../"+patch)

	def parse_patch_config(self):
		config_file = self.get_module_path()+'/patch.config'
		f = open(config_file ,'rb')
		try:
			for line in f:
				if line[0] == '#':
					continue
				if line.strip() == '':
					continue

				line = line.split('#')[0]
				self.patch_files.append(line)
				self.patch_count += 1
		finally:
			f.close()

	def do_patch(self):
		print "do_patch start:"
		if self.get_patch_flag():
			print "Patched alread!"
			return
		self.parse_patch_config()
		for p in self.patch_files:
			self.exe_one_patch(p)

		self.set_patch_flag()
		print "do_patch end!"

	def reverse_patch(self):
		print "reverse_patch start:"
		if not self.get_patch_flag():
			print "No patched!"
			return

		self.parse_patch_config()
		i = self.patch_count
		while(i>0):
			self.reverse_one_patch(self.patch_files[i-1])
			i -= 1
		
		self.clr_patch_flag()
		print "reverse_patch end!"

def help():
	print " patch manage "
	print " p1 is module name "
	print " p2 is module version "
	print " p3 is add patch or reverse patch"
	print " for example:"
	print "   do_patch alsa-lib 1.0.28 a"
	print " add patch to alsa-lib-1.0.28"

if __name__ == '__main__':
	if len(sys.argv) < 4:
		print " param error"
		help()
		exit(1)
	if sys.argv[3] == 'a':
		pm = Patch_Manage(sys.argv[1], sys.argv[2])
		pm.do_patch()
	elif sys.argv[3] == 'r':
		pm = Patch_Manage(sys.argv[1], sys.argv[2])
		pm.reverse_patch()
	elif sys.argv[3] == 'c':
		pm = Patch_Manage(sys.argv[1], sys.argv[2])
		if pm.get_patch_flag():
			print "%s-%s has been patched!!" % (sys.argv[1], sys.argv[2])
		else:
			print "%s-%s no patch!!" % (sys.argv[1], sys.argv[2])
		





