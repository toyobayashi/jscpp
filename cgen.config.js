module.exports = function (options) {
  return {
    project: 'jscpp',
    dependencies: options.NOTEST ? {} : {
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
        // publicDefines: ['JSCPP_USE_ERROR=0'],
        windows: {
          publicCompileOptions: ['/wd4251', '/wd4275']
        }
      },
      ...(options.NOTEST ? [] : [{
        name: 'test',
        type: 'exe',
        sources: [
          './test/main.cpp'
        ],
        ...(options.DLL ? { defines: ['JSCPP_IMPORT_DLL'] } : {}),
        // compileOptions: ['/execution-charset:utf-8']
        ...(options.DLL ? { libs: ['jscpp#', 'gtest#', 'gtest_main#'] } : { libs: ['jscpp!', 'gtest!', 'gtest_main!'] }),
        staticVCRuntime: !options.DLL
      }])
    ]
  }
}
