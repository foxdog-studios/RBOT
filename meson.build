project('rbot', 'cpp', default_options : [
  'buildtype=release',
  'warning_level=3',
  'b_lto=true',
  'b_ndebug=if-release',
  'cpp_std=c++17',
])

cpp = meson.get_compiler('cpp')

assimp = dependency('assimp')
opencv4 = dependency('opencv4')
opengl = dependency('opengl')
qt5 = dependency('qt5', modules : ['OpenGL', 'Widgets'])
rt = cpp.find_library('rt')
threads = dependency('threads')

rbot = static_library('rbot', 'src/video.cpp',
  dependencies : opencv4,
)

executable('rbot',
    sources : [
        'src/rbot.cpp',

        'src/Arguments.cpp',
        'src/model.cpp',
        'src/object3d.cpp',
        'src/optimization_engine.cpp',
        'src/pose_estimator6d.cpp',
        'src/rendering_engine.cpp',
        'src/signed_distance_transform2d.cpp',
        'src/tclc_histograms.cpp',
        'src/template_view.cpp',
        'src/transformations.cpp',
    ],

    dependencies : [
        assimp,
        opencv4,
        opengl,
        qt5,
        rt,
    ],

    link_with : rbot,
)

executable('shmvideo',
  sources : ['src/shmvideo.cpp'],
  dependencies : [opencv4, rt, threads],
  link_with : rbot,
)
