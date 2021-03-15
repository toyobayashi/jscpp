module.exports = {
  project: 'jscpp',
  dependencies: {
    './deps/googletest': {}
  },
  targets: [
    {
      name: 'jscpptest',
      type: 'exe',
      sources: [
        './src/main.cpp'
      ],
      // compileOptions: ['/execution-charset:utf-8']
      libs: ['jscpp', 'gtest!', 'gtest_main!'],
      staticVCRuntime: true
    },
    {
      name: 'jscpp',
      type: 'interface',
      interfaceIncludePaths: ['include']
    }
  ]
}
