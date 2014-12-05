# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

pargs = ['--cflags', '--libs']

def options(opt):
	opt.load('compiler_cxx compiler_c')
def configure(conf):
  conf.load('compiler_cxx compiler_c')
  conf.check_cfg(atleast_pkgconfig_version='0.0.0')
  conf.check_cfg(package='gstreamer-1.0', uselib_store='GSTREAMER', args=pargs)
  conf.env.LIB_PTHREAD = 'pthread'
def build(bld):
    for app in bld.path.ant_glob('*.cpp'):
        bld(features=['cxx', 'cxxprogram'],
            target = '%s' % (str(app.change_ext('','.cpp'))),
            source = app,
            use = 'GSTREAMER PTHREAD',
            install_path = None,
            )
