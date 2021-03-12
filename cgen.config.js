module.exports = {
  project: 'jscpp',
  targets: [
    {
      name: 'jscpptest',
      type: 'exe',
      sources: [
        './src/main.cpp'
      ],
      includePaths: ['include'],
      compileOptions: ['/execution-charset:utf-8']
      // libs: ['add'],
      // staticVCRuntime: true
    }/* ,
    {
      name: 'add',
      type: 'lib',
      'sources': [
        './src/lib.c'
      ],
      
      staticVCRuntime: true
    } */
  ]
}
