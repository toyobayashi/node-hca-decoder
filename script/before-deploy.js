const path = require('path')
const fs = require('fs-extra')

const getPath = (...args) => {
  return path.join(__dirname, '..', ...args)
}

const nodeVersion = process.versions.node.replace(/\./g, '_')
const arch = process.arch
const platform = process.platform

// const packageVersion = require(getPath('package.json')).version

if (fs.existsSync(getPath('dist/hca.node'))) {
  fs.mkdirsSync(getPath('release'))
  fs.copySync(
    getPath('dist/hca.node'),
    getPath(`release/hca-${process.env.TRAVIS_TAG}-${platform}-${arch}-${nodeVersion}.node`)
  )
}
