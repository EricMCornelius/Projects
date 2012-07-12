Project({
    name: 'Project4',
    type: 'StaticLib',
    compiler_flags: ['-fPIC', '-std=c++11'],
    installdir: 'install',
    include_paths: ['include'],

    source_root: 'src',
    sources: ['*.cpp'],

    header_root: 'include',
    headers: ['*.h']
});
