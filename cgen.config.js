module.exports = function (options) {
  return {
    project: 'jscpp',
    dependencies: {
      './deps/googletest': {}
    },
    targets: [
      {
        name: 'jscpp',
        type: options.DLL ? 'dll' : 'lib',
        sources: [
          './src/*.cpp'
        ],
        ...(options.DLL ? { defines: ['JSCPP_BUILD_DLL'] } : {}),
        publicIncludePaths: ['include'],
        staticVCRuntime: !options.DLL
        // interfaceIncludePaths: ['include']
      },
      {
        name: 'test',
        type: 'exe',
        sources: [
          './test/main.cpp'
        ],
        // ...(options.DLL ? { defines: ['JSCPP_IMPORT_DLL'] } : {}),
        // compileOptions: ['/execution-charset:utf-8']
        ...(options.DLL ? { libs: ['jscpp', 'gtest#', 'gtest_main#'] } : { libs: ['jscpp', 'gtest!', 'gtest_main!'] }),
        staticVCRuntime: !options.DLL
      }
    ]
  }
} 
