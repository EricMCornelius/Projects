Project({
    name: 'Project2',
    type: 'SharedLib',
    compiler_flags: ['-fPIC'],

    deps: ['Project3', 'Project4'],

    sources: ['*.cpp'],
    headers: ['*.h']
});
