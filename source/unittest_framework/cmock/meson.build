###################################################################################
#                                                                                 #
# NAME: meson.build                                                               #
#                                                                                 #
# AUTHOR: Mike Karlesky, Mark VanderVoord, Greg Williams.                         #
# WRITTEN BY: Michael Brockus.                                                    #
#                                                                                 #
# License: MIT                                                                    #
#                                                                                 #
###################################################################################


project('cmock', 'c',
    license         : 'MIT',
    meson_version   : '>=0.50.0',
    subproject_dir : 'vendor',
    default_options: [
        'buildtype=minsize',
        'optimization=3', 
        'warning_level=3',
        'werror=true',
        ]
)
lang = 'c'
cc = meson.get_compiler(lang)


##
#
# Meson: Add compiler flags
#
##
if cc.get_id() == 'clang'
    add_project_arguments(cc.get_supported_arguments(
            [
            '-Wweak-vtables', '-Wexit-time-destructors',
            '-Wglobal-constructors', '-Wmissing-noreturn' 
            ]
        ), language: lang)
endif

if cc.get_argument_syntax() == 'gcc'
    add_project_arguments(cc.get_supported_arguments(
            [
            '-Wformat', '-Waddress', '-Winit-self', '-Wno-multichar',
            '-Wpointer-arith'       , '-Wwrite-strings'              , 
            '-Wno-parentheses'      , '-Wno-type-limits'             , 
            '-Wformat-security'     , '-Wunreachable-code'           , 
            '-Waggregate-return'    , '-Wformat-nonliteral'          ,
            '-Wmissing-prototypes'  , '-Wold-style-definition'       ,
            '-Wmissing-declarations', '-Wmissing-include-dirs'       , 
            '-Wno-unused-parameter' , '-Wdeclaration-after-statement'
            ]
        ), language: lang)
endif

if cc.get_id() == 'msvc'
    add_project_arguments(cc.get_supported_arguments(
            [
            '/w44265', '/w44061', '/w44062', 
            '/wd4018', '/wd4146', '/wd4244',
            '/wd4305',
            ]
        ), language: lang)
endif

unity_dep = dependency('unity', fallback: ['unity', 'unity_dep'])

subdir('src')

cmock_dep = declare_dependency(link_with: cmock_lib, include_directories: cmock_dir)
