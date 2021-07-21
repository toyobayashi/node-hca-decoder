# node-hca-decoder

[![Build status](https://api.travis-ci.com/toyobayashi/node-hca-decoder.svg?branch=master)](https://travis-ci.com/toyobayashi/node-hca-decoder)

HCA decoder based on [Nyagamon/HCADecoder](https://github.com/Nyagamon/HCADecoder)

## Usage

``` js
const { HCADecoder } = require('hca-decoder')
const hca = new HCADecoder(/* ciphKey1, ciphKey2 */)
const filenameHCA = 'path/to/somefile.hca'

hca.decodeToWaveFile(filenameHCA/* , filenameWAV, volume, mode, loop, (err, wavFilePath) => {} */)
// => undefined (Async)

hca.decodeToWaveFileSync(filenameHCA/* , filenameWAV, volume, mode, loop */)
// => boolean

hca.printInfo(filenameHCA)
// => undefined

HCADecoder.getInfo(filenameHCA)
// => HCAInfo
```

## Test

``` bash
# install node-gyp (5+)
$ npm install -g node-gyp

# tell npm use global node-gyp
$ npm config set node_gyp "`npm prefix -g`/lib/node_modules/node-gyp/bin/node-gyp.js"

# for Windows
# > for /f "delims=" %P in ('npm prefix -g') do npm config set node_gyp "%P\node_modules\node-gyp\bin\node-gyp.js"

# install node C++ header
$ node-gyp install # --target=<node version>

$ npm install # --no-package-lock

$ npm test
```

## License

* MIT
