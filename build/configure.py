# Copyright 2013 The Chromium Authors. All Rights Reserved.

from optparse import OptionParser
import os
import sys

ninja_dir = os.path.join(os.path.split(__file__)[0],
                         '..', 'third_party', 'ninja')
ninja_misc_dir = os.path.join(ninja_dir, 'misc')
print 'Updating git submodules...'
if os.system('git submodule update --init') != 0:
  raise SystemExit("Couldn't update git submodules")

sys.path.append(ninja_misc_dir)
import ninja_syntax

def GetChromiumBaseFileList(base_dir, platform):
  def get_file_list():
    all_files = []
    orig = os.getcwd()
    os.chdir(base_dir)
    for path, dirs, files in os.walk('.'):
      if os.path.normpath(path).startswith('.git'):
        continue
      for file in files:
        all_files.append(os.path.normpath(os.path.join(path, file)))
    os.chdir(orig)
    return all_files


  def filter_file_list(all_files, for_types):
    result = all_files[:]
    for x in ('file_path_watcher_stub.cc',
              'dtoa.cc',
              'event_recorder_stubs.cc',
              '_mock.cc',
              'check_example.cc',
              'dynamic_annotations.c', # avoid futzing with .c
              'debug_message.cc',
              'allocator\\', # Kind of overly involved for user-configuration.
              'field_trial.cc', # Has screwy winsock inclusion, don't need it.
              'i18n\\', # Requires icu (I think)
              ):
      result = [y for y in result if x not in y]
    if 'windows' in for_types:
      for x in ('_posix', '_mac', '_android', '_linux', '_ios', '_solaris',
                '.java', '_gcc', '.mm', 'android\\', '_libevent',
                'chromeos\\', 'data\\', '_freebsd', '_nacl', 'linux_',
                '_glib', '_gtk', 'mac\\', 'unix_', 'file_descriptor',
                '_aurax11', 'sha1_win.cc', '_openbsd', 'xdg_mime', '_kqueue',
                'symbolize', 'string16.cc', '_chromeos', 'nix\\', 'xdg_',
                ):
        result = [y for y in result if x not in y]
    if 'linux' in for_types:
      for x in ('_win', '_mac', '_android', '_ios', '_solaris',
                '.java', '.mm', 'android/', '_libevent',
                'chromeos/', 'data/', '_freebsd', '_nacl', 'linux_',
                '_glib', '_gtk', 'mac/', 'file_descriptor',
                '_aurax11', 'sha1_win.cc', '_openbsd', 'xdg_mime', '_kqueue',
                'symbolize', 'string16.cc', '_chromeos', 'xdg_',
                ):
        result = [y for y in result if x not in y]
    if 'lib' in for_types:
      for x in ('README', 'LICENSE', 'OWNERS', '.h', '.patch', 'unittest',
                'PRESUBMIT', 'DEPS', '.gyp', '.py', '.isolate', '.nc', 'test\\',
                'test/', '.git', '_browsertest.cc', 'base64.cc' # TEMP
                ):
        result = [y for y in result if x not in y]
    return result

  files = get_file_list()
  return filter_file_list(files, (platform, 'lib'))


def GetRe2FileList():
  files = [
    're2/bitstate',
    're2/compile',
    're2/dfa',
    're2/filtered_re2',
    're2/mimics_pcre',
    're2/nfa',
    're2/onepass',
    're2/parse',
    're2/perl_groups',
    're2/prefilter',
    're2/prefilter_tree',
    're2/prog',
    're2/re2',
    're2/regexp',
    're2/set',
    're2/simplify',
    're2/tostring',
    're2/unicode_casefold',
    're2/unicode_groups',
    'util/arena',
    'util/hash',
    'util/rune',
    'util/stringpiece',
    'util/stringprintf',
    'util/strutil',
    'util/valgrind',
    ]
  return [os.path.normpath(p) for p in files]


def GetFreetypeFileList():
  files = [
    'src/autofit/autofit',
    'src/base/ftbase',
    'src/base/ftbbox',
    'src/base/ftbitmap',
    'src/base/ftdebug',
    'src/base/ftfstype',
    'src/base/ftgasp',
    'src/base/ftglyph',
    'src/base/ftgxval',
    'src/base/ftinit',
    'src/base/ftlcdfil',
    'src/base/ftmm',
    'src/base/ftpatent',
    'src/base/ftpfr',
    'src/base/ftstroke',
    'src/base/ftsynth',
    'src/base/ftsystem',
    'src/base/fttype1',
    'src/base/ftwinfnt',
    'src/base/ftxf86',
    'src/bdf/bdf',
    'src/cache/ftcache',
    'src/cff/cff',
    'src/cid/type1cid',
    'src/gzip/ftgzip',
    'src/lzw/ftlzw',
    'src/pcf/pcf',
    'src/pfr/pfr',
    'src/psaux/psaux',
    'src/pshinter/pshinter',
    'src/psnames/psmodule',
    'src/raster/raster',
    'src/sfnt/sfnt',
    'src/smooth/smooth',
    'src/truetype/truetype',
    'src/type1/type1',
    'src/type42/type42',
    'src/winfonts/winfnt',
  ]
  return [os.path.normpath(p) for p in files]


def FilterForPlatform(sources_list, platform):
  # TODO(scottmg): Less lame for the next platform.
  if platform == 'linux':
    return filter(lambda x: not x.endswith('_win'), sources_list)
  elif platform == 'windows':
    return filter(lambda x: not x.endswith('_linux'), sources_list)
  elif platform == 'mac':
    return filter(lambda x: not x.endswith('_mac'), sources_list)


def main():
  platform = 'windows'
  if sys.platform.startswith('linux'):
    platform = 'linux'
  elif sys.platform.startswith('darwin'):
    platform = 'mac'

  if platform == 'windows':
    CXX = 'cl'
    CC = 'cl'
    objext = '.obj'
    exeext = '.exe'
  else:
    CXX = os.environ.get('CXX', 'g++')
    CC = os.environ.get('CC', 'gcc')
    objext = '.o'
    exeext = ''

  if not os.path.exists(os.path.join(ninja_dir, 'ninja' + exeext)):
    print "ninja binary doesn't exist, trying to build it..."
    old_dir = os.getcwd()
    os.chdir(ninja_dir)
    if os.system('%s bootstrap.py' % sys.executable) != 0:
      raise SystemExit('Failed to bootstrap ninja')
    os.chdir(old_dir)

  parser = OptionParser()
  parser.add_option('--debug', action='store_true',
                    help='enable debugging extras',)
  (options, args) = parser.parse_args()
  if args:
    print 'ERROR: extra unparsed command-line arguments:', args
    sys.exit(1)

  pch_enabled = options.debug and platform == 'windows'


  BUILD_FILENAME = 'build.ninja'
  buildfile = open(BUILD_FILENAME, 'w')
  n = ninja_syntax.Writer(buildfile)

  n.comment('The arguments passed to configure.py, for rerunning it.')
  n.variable('configure_args', ' '.join(sys.argv[1:]))
  n.newline()

  def src(filename):
    return os.path.normpath(os.path.join('sg', filename))
  def base_src(filename):
    return os.path.normpath(os.path.join('third_party', 'base', filename))
  def gwen_src(filename):
    return os.path.normpath(os.path.join('third_party', 'gwen', filename))
  def ft2_src(filename):
    return os.path.normpath(os.path.join('third_party', 'freetype', filename))
  def re2_src(filename):
    return os.path.normpath(os.path.join('third_party', 're2', filename))
  def built(filename):
    return os.path.normpath(os.path.join('$builddir', 'obj', filename))
  pch_name = built('sg.pch')
  pch_implicit = None
  pch_compile = ''
  if pch_enabled:
    pch_implicit = pch_name
    pch_compile = '/Fp' + pch_name + ' /Yusg/global.h'
  def cc(name, src=src, **kwargs):
    # No pch on cc because it's only ft2, and it doesn't like pch anyway (and
    # also, we have to build a separate pch so it's a bit of a mess).
    return n.build(built(name + objext), 'cc', src(name + '.c'), **kwargs)
  def cxx(name, src=src, **kwargs):
    return n.build(built(name + objext), 'cxx', src(name + '.cc'),
                   implicit=pch_implicit, **kwargs)
  def cpp(name, src=src, **kwargs):
    return n.build(built(name + objext), 'cxx', src(name + '.cpp'),
                   implicit=pch_implicit, **kwargs)
  def rc(name, src=src, **kwargs):
    return n.build(built(name + objext), 'rc', src(name + '.rc'), **kwargs)
  def binary(name):
    exe = os.path.join('$builddir', name + '.exe')
    n.build(name, 'phony', exe)
    return exe

  if not os.path.exists('out'):
    os.makedirs('out')
  n.variable('builddir', 'out')
  n.variable('cxx', CXX)
  n.variable('cc', CC)

  if platform == 'windows':
    cflags = ['/nologo',  # Don't print startup banner.
              '/Zi',  # Create pdb with debug info.
              '/W4',  # Highest warning level.
              '/WX',  # Warnings as errors.
              '/wd4530', '/wd4100', '/wd4706', '/wd4245', '/wd4018',
              '/wd4512', '/wd4800', '/wd4702', '/wd4819', '/wd4355',
              '/wd4996', '/wd4481', '/wd4127', '/wd4310', '/wd4244',
              '/wd4701', '/wd4201', '/wd4389', '/wd4722', '/wd4703',
              '/wd4510', '/wd4610',
              '/GR-',  # Disable RTTI.
              '/DNOMINMAX', '/D_CRT_SECURE_NO_WARNINGS',
              '/DUNICODE', '/D_UNICODE',
              '/D_CRT_RAND_S', '/DWIN32', '/D_WIN32',
              '/D_WIN32_WINNT=0x0601', '/D_VARIADIC_MAX=10',
              '/DDYNAMIC_ANNOTATIONS_ENABLED=0',
              '-I.', '-Ithird_party',
              '-Ithird_party/re2',
              '-Ibuild', '-Ithird_party/freetype/include',
              '-FIsg/global.h']
    if options.debug:
      cflags += ['/D_DEBUG', '/MTd']
    else:
      cflags += ['/DNDEBUG', '/MT']
    ldflags = ['/DEBUG', '/SUBSYSTEM:WINDOWS']
    if not options.debug:
      cflags += ['/Ox', '/DNDEBUG', '/GL']
      ldflags += ['/LTCG', '/OPT:REF', '/OPT:ICF']
    else:
      ldflags += ['/INCREMENTAL']
    libs = []
    cxxflags = [
        '/Fd$builddir\\sg_cxx_intermediate.pdb',
        ]
    ccflags = [
        '/DFT2_BUILD_LIBRARY', '/wd4146',
        '/Fd$builddir\\sg_cc_intermediate.pdb',
        ]
  else:
    cflags = ['-g', '-Wall', '-Wextra',
              '-Wno-deprecated',
              '-Wno-unused-parameter',
              '-Wno-sign-compare',
              '-fno-rtti',
              '-fno-exceptions',
              '-fvisibility=hidden', '-pipe',
              '-Wno-missing-field-initializers',
              '-I.', '-Ithird_party',
              '-Ithird_party/re2',
              '-Ibuild', '-Ithird_party/freetype/include',
              '-include', 'sg/global.h',
              ]
    if options.debug:
      cflags += ['-D_GLIBCXX_DEBUG', '-D_GLIBCXX_DEBUG_PEDANTIC']
      cflags.remove('-fno-rtti')  # Needed for above pedanticness.
    else:
      cflags += ['-O2', '-DNDEBUG']
    ccflags = []
    cxxflags = []
    ldflags = ['-L$builddir']

  n.newline()

  n.variable('cflags', ' '.join(cflags))
  n.variable('ccflags', ' '.join(ccflags))
  n.variable('cxxflags', ' '.join(cxxflags))
  n.variable('ldflags', ' '.join(ldflags))
  n.newline()

  if platform == 'windows':
    cxx_compiler = 'ninja -t msvc -o $out -- $cxx /showIncludes'
    n.rule('cxx',
      command=('%s $cflags $cxxflags %s '
              '-c $in /Fo$out' % (cxx_compiler, pch_compile)),
      depfile='$out.d',
      description='CXX $out')
    n.newline()
  else:
    n.rule('cxx',
      command='$cxx -MMD -MT $out -MF $out.d $cflags $cxxflags -c $in -o $out',
      depfile='$out.d',
      description='CXX $out')
    n.newline()

  if platform == 'windows':
    cc_compiler = 'ninja -t msvc -o $out -- $cc /showIncludes'
    n.rule('cc',
      command=('%s $cflags $ccflags '
              '-c $in /Fo$out' % cc_compiler),
      depfile='$out.d',
      description='CC $out')
  else:
    n.rule('cc',
      command='$cc -MMD -MT $out -MF $out.d $cflags $ccflags -c $in -o $out',
      depfile='$out.d',
      description='CXX $out')
  n.newline()

  n.rule('link',
        command='$cxx $in $libs /nologo /link $ldflags /out:$out',
        description='LINK $out')
  n.newline()

  n.rule('rc',
      command='rc /r /nologo /fo $out $in',
        description='RC $out')
  n.newline()

  pch_objs = []
  if pch_enabled:
    compiler = 'ninja -t msvc -o $objname -- $cxx /showIncludes'
    n.rule('cxx_pch',
      command=('%s $cflags $cxxflags /Ycsg/global.h %s '
              '-c $in /Fo$objname' % (compiler, pch_compile)),
      depfile=built('sg_pch.obj.d'),
      description='CXX $out')
    n.newline()

    n.comment('Build the precompiled header.')

    n.build([built('sg_pch.obj'), built('sg.pch')], 'cxx_pch', src('sg_pch.cc'),
            variables=[('objname', built('sg_pch.obj'))])
    pch_objs = [built('sg_pch.obj')]
    n.newline()

  sg_objs = []
  n.comment('Core source files.')
  core_sources = [
               'app_thread',
               #'backend/backend_native_win',
               'backend/debug_core_gdb',
               #'backend/debug_core_native_win',
               'backend/gdb_mi_parse',
               'backend/gdb_to_generic_converter',
               #'backend/process_native_win',
               'backend/subprocess_win',
               'cpp_lexer',
               'debug_presenter',
               'debug_presenter_display',
               'display_util',
               'lexer',
               'lexer_state',
               'locals_view',
               'main_loop',
               'render/renderer',
               'render/scoped_render_offset',
               'render/texture',
               'source_files',
               'source_view',
               'status_bar',
               'stack_view',
               'ui/dockable',
               'ui/docking_resizer',
               'ui/docking_split_container',
               'ui/docking_tool_window',
               'ui/docking_workspace',
               'ui/focus',
               'ui/scroll_helper',
               'ui/scrolling_output_view',
               'ui/skin',
               'ui/tool_window_dragger',
               'ui/tree_view_helper',
               'workspace',
              ]
  for name in FilterForPlatform(core_sources, platform):
    sg_objs += cxx(name)
  n.newline() 

  re2_objs = []
  n.comment('RE2.')
  for base in GetRe2FileList():
    re2_objs += cxx(base, src=re2_src)
  n.newline()

  ft2_objs = []
  n.comment('Freetype.')
  for base in GetFreetypeFileList():
    ft2_objs += cc(base, src=ft2_src)
  n.newline()

  n.comment('Chromium base.')
  crfiles = GetChromiumBaseFileList('third_party/base', platform)
  base_objs = []
  for name in crfiles:
    base, ext = os.path.splitext(name)
    base_objs += cxx(base, src=base_src)
  n.newline()

  libs = ['advapi32.lib',
          'comdlg32.lib',
          'dbghelp.lib',
          'd2d1.lib',
          'dwrite.lib',
          'gdi32.lib',
          'ole32.lib',
          'oleaut32.lib',
          'opengl32.lib',
          'shell32.lib',
          'user32.lib',
          'version.lib',
          'windowscodecs.lib',
         ]

  all_targets = []

  app_objs = sg_objs + base_objs + re2_objs + ft2_objs + pch_objs

  n.comment('Main executable is library plus main() and some startup goop.')
  main_objs = []
  base_sources = ['application',
               'render/application_window_win',
               'render/gpu_win',
               'render/direct2d_win',
               'main_win'
               ]
  for base in FilterForPlatform(base_sources, platform):
    main_objs += cxx(base)
  if platform == 'windows':
    main_objs += rc('sg', implicit=['art\\sg.ico'])
  # No .libs for /incremental to work.
  sg_binary = binary('sg')
  sg = n.build(sg_binary, 'link', inputs=main_objs + app_objs,
               variables=[('libs', libs)])
  n.newline()
  all_targets += sg

  n.comment('Tests all build into sg_test executable.')

  variables = []
  test_cflags = None
  test_ldflags = ldflags + ['/SUBSYSTEM:CONSOLE']
  test_libs = libs
  test_objs = []
  path = 'third_party/testing/gtest'

  gtest_all_incs = ['-I%s' % path,  '-I%s' % os.path.join(path, 'include')]
  gtest_cflags = cflags + gtest_all_incs
  test_objs += n.build(built('gtest-all' + objext), 'cxx',
                       inputs=os.path.join(path, 'src', 'gtest-all.cc'),
                       implicit=pch_implicit,
                       variables=[('cflags', gtest_cflags)])
  test_objs += n.build(built('gtest_main' + objext), 'cxx',
                       inputs='sg/main_test.cc',
                       implicit=pch_implicit,
                       variables=[('cflags', gtest_cflags)])

  test_cflags = cflags + ['-DGTEST_HAS_RTTI=0',
                          '-I%s' % os.path.join(path, 'include')]

  for name in [
               'lexer_test',
               #'backend/debug_core_native_win_test',
               'backend/debug_core_gdb_test',
               'backend/gdb_mi_parse_test',
               'backend/subprocess_test',
               'ui/docking_test',
              ]:
    test_objs += cxx(name, variables=[('cflags', test_cflags)])

  sg_test = n.build(binary('sg_test'), 'link', inputs=test_objs + app_objs,
                    # Unnecessary but orders pdb access so test link isn't
                    # accessing the intermediate pdb while the main-app-only
                    # objs are still compiling.
                    order_only=sg_binary,
                    variables=[('ldflags', test_ldflags),
                               ('libs', test_libs)])
  all_targets += sg_test
  n.newline()

  reader_writer_objs = []
  reader_writer_objs += cxx('backend/reader_writer_test') + pch_objs
  reader_writer_test = n.build(binary('reader_writer_test'), 'link',
                               inputs=reader_writer_objs,
                               implicit=pch_implicit,
                               order_only=sg_binary,
                               variables=[('ldflags', test_ldflags)])
  all_targets += reader_writer_test
  n.newline()

  n.comment('Regenerate build files if build script changes.')
  prefix = ''
  if platform == 'windows':
    prefix = 'cmd /c '
  n.rule('configure',
          command=prefix + 'python build/configure.py $configure_args',
          generator=True)
  n.build('build.ninja', 'configure',
          implicit=[os.path.normpath('build/configure.py'),
                    os.path.normpath('%s/misc/ninja_syntax.py' % ninja_dir)])
  n.newline()


  n.build('all', 'phony', all_targets)

  print 'wrote %s.' % BUILD_FILENAME

if __name__ == '__main__':
  main()
