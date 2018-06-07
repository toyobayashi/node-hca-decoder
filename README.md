# node-hca-decoder

HCA decoder based on [Nyagamon/HCADecoder](https://github.com/Nyagamon/HCADecoder)

Require Node.js >= 8.x (NAPI)

## Usage

``` js
const { HCADecoder } = require('hca-decoder')
let hca = new HCADecoder(/* ciphKey1, ciphKey2 */)
let filenameHCA = 'path/to/somefile.hca'

hca.decodeToWaveFile(filenameHCA/* , filenameWAV, volume, mode, loop, (err, wavFilePath) => {} */)
// => undefined (Async)

hca.decodeToWaveFileSync(filenameHCA/* , filenameWAV, volume, mode, loop */)
// => string | undefined

hca.decrypt(filenameHCA)
// => string | undefined

hca.printInfo(filenameHCA)
// => undefined
```

## License

* MIT
