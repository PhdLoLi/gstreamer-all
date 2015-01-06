# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-
from waflib import Logs, Configure

pargs = ['--cflags', '--libs']

def options(opt):
	opt.load('compiler_cxx compiler_c')
def configure(conf):
  defaultFlags = ['-std=c++0x', '-std=c++11',
                  '-stdlib=libc++',   # clang on OSX < 10.9 by default uses gcc's
                                      # libstdc++, which is not C++11 compatible
                  '-pedantic', '-Wall']
  conf.load('compiler_cxx compiler_c')
  conf.add_supported_cxxflags(defaultFlags)
  conf.check_cfg(atleast_pkgconfig_version='0.0.0')
  conf.check_cfg(package='gstreamer-1.0', uselib_store='GSTREAMER', args=pargs)
  conf.env.LIB_PTHREAD = 'pthread'

@Configure.conf
def add_supported_cxxflags(self, cxxflags):
    
    self.start_msg('Checking supported CXXFLAGS')

    supportedFlags = []
    for flag in cxxflags:
        if self.check_cxx(cxxflags=['-Werror', flag], mandatory=False):
            supportedFlags += [flag]

    self.end_msg(' '.join(supportedFlags))
    self.env.CXXFLAGS = supportedFlags + self.env.CXXFLAGS

def build(bld):
    for app in bld.path.ant_glob('*.cpp'):
        bld(features=['cxx', 'cxxprogram'],
            target = '%s' % (str(app.change_ext('','.cpp'))),
            source = app,
            use = 'GSTREAMER PTHREAD',
            install_path = None,
            )
