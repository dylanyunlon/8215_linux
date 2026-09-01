#!/usr/bin/env python3
"""Build the AWTK resource tools (xml_to_ui, bsvggen, fontgen, imagegen,
resgen, strgen, themegen) as HOST binaries so they can be invoked by
update_res.py on the build machine.

The awtk-linux-fb package builds everything (AWTK lib AND these tools)
with the cross-compile gcc, so the resulting binaries are ARM and cannot
be executed by the build host. update_res.py needs to call these as
host processes; this script produces an x86_64 copy alongside.

Usage:
    python3 scripts/build_host_tools.py <BIN_DIR> <LIB_DIR>

    BIN_DIR  output dir for the tools (e.g. $(HOST_DIR)/usr/bin)
    LIB_DIR  output dir for host-built helper libs (e.g. $(HOST_DIR)/usr/lib)
"""
import os
import subprocess
import sys

THIS_DIR = os.path.dirname(os.path.abspath(__file__))
# Default AWTK_SRC points at the SDK source tree (used only when the caller
# does not pass one explicitly). Caller (hcn-application.mk) passes the
# build-dir copy $(BUILD_DIR)/awtk-1.0/awtk instead, so the tools pick up
# the same AWTK that awtk-linux-fb was built against.
AWTK_SRC_DEFAULT = os.path.normpath(os.path.join(
    THIS_DIR, '..', '..', '..', '..', '..', 'bsp', 'arm2', 'lib', 'libawtk', 'src', 'awtk-1.8'))


def run(cmd, cwd, env=None):
    print('>>>', ' '.join(cmd), '(cwd=' + cwd + ')')
    r = subprocess.run(cmd, cwd=cwd, env=env)
    if r.returncode != 0:
        sys.exit('failed: ' + ' '.join(cmd))


def main():
    if len(sys.argv) < 3:
        sys.exit('usage: build_host_tools.py <BIN_DIR> <LIB_DIR> [AWTK_SRC]')
    bin_dir = os.path.abspath(sys.argv[1])
    lib_dir = os.path.abspath(sys.argv[2])
    awtk_src = os.path.abspath(sys.argv[3]) if len(sys.argv) > 3 else AWTK_SRC_DEFAULT
    if not os.path.isdir(awtk_src):
        sys.exit('AWTK_SRC not found: ' + awtk_src)
    os.makedirs(bin_dir, exist_ok=True)
    os.makedirs(lib_dir, exist_ok=True)

    env = os.environ.copy()
    env['BIN_DIR'] = bin_dir
    env['LIB_DIR'] = lib_dir
    env['TK_ROOT'] = awtk_src
    env['TK_3RD_ROOT'] = os.path.join(awtk_src, '3rd')

    # Make CPPPATH explicit so awtk headers resolve under host gcc.
    # The tool SConscripts use DefaultEnvironment().Clone() so any CPPPATH
    # we set here is propagated to the default env in the stub SConstruct.
    cpppath = [
        os.path.join(awtk_src, 'src'),
        os.path.join(awtk_src, 'tools'),
        os.path.join(awtk_src, 'tools', 'common'),
        os.path.join(awtk_src, '3rd'),
    ]
    env['CPPPATH'] = ':'.join(cpppath)

    # Build a stub SConstruct under BIN_DIR (writable local path) so scons's
    # .sconsign.dblite write doesn't fail on a network-mounted cwd. We use
    # absolute paths to the tool SConscripts via env['AWTK_SRC'] so the
    # relative paths inside the tools don't depend on cwd.
    stub_dir = os.path.join(bin_dir, '_host_tools_stub')
    os.makedirs(stub_dir, exist_ok=True)
    sconstruct = os.path.join(stub_dir, 'SConstruct')
    env['AWTK_SRC'] = awtk_src
    with open(sconstruct, 'w') as f:
        f.write("""import os
awtk_src = os.environ['AWTK_SRC']
default_env = DefaultEnvironment()
default_env['ENV'] = os.environ
default_env['CPPPATH'] = os.environ.get('CPPPATH', '').split(':') if os.environ.get('CPPPATH') else []
default_env['LIBPATH'] = [os.environ['LIB_DIR'], os.environ['BIN_DIR']]
# Tool SConscripts do `env['LIBS'] = [...] + env['LIBS']` and read
# OS_SUBSYSTEM_CONSOLE/LINKFLAGS; provide empty defaults so they don't
# KeyError before we get to Program().
default_env['LIBS'] = []
default_env['OS_SUBSYSTEM_CONSOLE'] = ''
default_env['OS_SUBSYSTEM_WINDOWS'] = ''
default_env['LINKFLAGS'] = ''
SConscript(os.path.join(awtk_src, 'tools', 'common', 'SConscript'))
SConscript(os.path.join(awtk_src, 'tools', 'theme_gen', 'SConscript'))
SConscript(os.path.join(awtk_src, 'tools', 'font_gen', 'SConscript'))
SConscript(os.path.join(awtk_src, 'tools', 'image_gen', 'SConscript'))
SConscript(os.path.join(awtk_src, 'tools', 'res_gen', 'SConscript'))
SConscript(os.path.join(awtk_src, 'tools', 'str_gen', 'SConscript'))
SConscript(os.path.join(awtk_src, 'tools', 'svg_gen', 'SConscript'))
SConscript(os.path.join(awtk_src, 'tools', 'ui_gen', 'xml_to_ui', 'SConscript'))
""")
    # Avoid leaving stale artefacts from a prior run that may live in BIN_DIR
    # from another AWTK_SRC.
    sconsign = os.path.join(stub_dir, '.sconsign.dblite')
    if os.path.exists(sconsign):
        os.remove(sconsign)

    run(['scons', '-C', stub_dir], cwd=stub_dir, env=env)

    # Sanity-check the tools we need for update_res.py
    needed = ['bsvggen', 'fontgen', 'imagegen', 'resgen', 'strgen', 'themegen', 'xml_to_ui']
    missing = [t for t in needed if not os.path.exists(os.path.join(bin_dir, t))]
    if missing:
        sys.exit('host tools missing after build: ' + ', '.join(missing))
    print('host tools OK:', bin_dir)


if __name__ == '__main__':
    main()