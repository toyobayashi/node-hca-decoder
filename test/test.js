const { HCADecoder } = require('..')
const fs = require('fs-extra')
const path = require('path')
const assert = require('assert')

const getPath = (...args) => {
  return path.join(__dirname, '..', ...args)
}

const hd = new HCADecoder()

describe('HCADecoder class', function () {
  it('#printInfo', function () {
    fs.mkdirsSync(getPath('assets/test/printInfo'))
    fs.copySync(getPath('assets/origin/1.hca'), getPath('assets/test/printInfo/1.hca'))

    hd.printInfo(getPath('assets/test/printInfo/1.hca'))
  })

  // it('#decrypt', function () {
  //   fs.mkdirsSync(getPath('assets/test/decrypt'))
  //   fs.copySync(getPath('assets/origin/1.hca'), getPath('assets/test/decrypt/1.hca'))

  //   assert.ok(hd.decrypt(getPath('assets/test/decrypt/1.hca')))
  // })

  it('#decodeToWaveFile1', function (done) {
    fs.mkdirsSync(getPath('assets/test/decodeToWaveFile'))
    fs.copySync(getPath('assets/origin/1.hca'), getPath('assets/test/decodeToWaveFile/1.hca'))

    hd.decodeToWaveFile(getPath('assets/test/decodeToWaveFile/1.hca'), function (err, wavPath) {
      if (err) {
        done(err)
      } else {
        try {
          assert.ok(wavPath === getPath('assets/test/decodeToWaveFile/1.wav'))
          assert.ok(fs.existsSync(getPath('assets/test/decodeToWaveFile/1.wav')))
          done()
        } catch (err) {
          done(err)
        }
      }
    })
  })

  it('#decodeToWaveFile2', function (done) {
    fs.mkdirsSync(getPath('assets/test/decodeToWaveFile'))
    fs.copySync(getPath('assets/origin/1.hca'), getPath('assets/test/decodeToWaveFile/1.hca'))

    const dest = getPath('assets/test/decodeToWaveFile/custom.wav')
    hd.decodeToWaveFile(getPath('assets/test/decodeToWaveFile/1.hca'), dest, function (err, wavPath) {
      if (err) {
        done(err)
      } else {
        try {
          assert.ok(wavPath === dest)
          assert.ok(fs.existsSync(dest))
          done()
        } catch (err) {
          done(err)
        }
      }
    })
  })

  it('#decodeToWaveFile3', function (done) {
    fs.mkdirsSync(getPath('assets/test/decodeToWaveFile'))
    fs.copySync(getPath('assets/origin/1.hca'), getPath('assets/test/decodeToWaveFile/1.hca'))

    const dest = getPath('assets/test/decodeToWaveFile/volumn.wav')
    hd.decodeToWaveFile(getPath('assets/test/decodeToWaveFile/1.hca'), dest, 0.1, function (err, wavPath) {
      if (err) {
        done(err)
      } else {
        try {
          assert.ok(wavPath === dest)
          assert.ok(fs.existsSync(dest))
          done()
        } catch (err) {
          done(err)
        }
      }
    })
  })

  it('#decodeToWaveFile4', function (done) {
    fs.mkdirsSync(getPath('assets/test/decodeToWaveFile'))
    fs.copySync(getPath('assets/origin/1.hca'), getPath('assets/test/decodeToWaveFile/1.hca'))

    const dest = getPath('assets/test/decodeToWaveFile/mode.wav')
    hd.decodeToWaveFile(getPath('assets/test/decodeToWaveFile/1.hca'), dest, 1, 8, function (err, wavPath) {
      if (err) {
        done(err)
      } else {
        try {
          assert.ok(wavPath === dest)
          assert.ok(fs.existsSync(dest))
          done()
        } catch (err) {
          done(err)
        }
      }
    })
  })

  // it('#decodeToWaveFile5', function (done) {
  //   this.timeout(Infinity)
  //   fs.mkdirsSync(getPath('assets/test/decodeToWaveFile'))
  //   fs.copySync(getPath('assets/origin/1.hca'), getPath('assets/test/decodeToWaveFile/1.hca'))
  //   const hd = new HCADecoder()
  //   const dest = getPath('assets/test/decodeToWaveFile/loop.wav')
  //   hd.decodeToWaveFile(getPath('assets/test/decodeToWaveFile/1.hca'), dest, 1, 16, 4, function (err, wavPath) {
  //     if (err) {
  //       done(err)
  //     } else {
  //       try {
  //         assert.ok(wavPath === dest)
  //         assert.ok(fs.existsSync(dest))
  //         done()
  //       } catch (err) {
  //         done(err)
  //       }
  //     }
  //   })
  // })

  it('#decodeToWaveFile Promise All', function (done) {
    this.timeout(Infinity)
    fs.mkdirsSync(getPath('assets/test/PromiseAll'))
    fs.copySync(getPath('assets/origin/1.hca'), getPath('assets/test/PromiseAll/1.hca'))
    fs.copySync(getPath('assets/origin/2.hca'), getPath('assets/test/PromiseAll/2.hca'))
    fs.copySync(getPath('assets/origin/3.hca'), getPath('assets/test/PromiseAll/3.hca'))

    const decode = (hd, hca) => {
      return new Promise((resolve, reject) => {
        hd.decodeToWaveFile(hca, function (err, wav) {
          if (err) reject(err)
          else resolve(wav)
        })
      })
    }

    Promise.all([
      decode(hd, getPath('assets/test/PromiseAll/1.hca')),
      decode(hd, getPath('assets/test/PromiseAll/2.hca')),
      decode(hd, getPath('assets/test/PromiseAll/3.hca'))
    ]).then(([a, b, c]) => {
      assert.ok(a === getPath('assets/test/PromiseAll/1.wav') && fs.existsSync(a))
      assert.ok(b === getPath('assets/test/PromiseAll/2.wav') && fs.existsSync(b))
      assert.ok(c === getPath('assets/test/PromiseAll/3.wav') && fs.existsSync(c))
      done()
    }).catch(err => {
      done(err)
    })
  })

  it('#decodeToWaveFileSync', function () {
    this.timeout(Infinity)
    fs.mkdirsSync(getPath('assets/test/decodeToWaveFileSync'))
    fs.copySync(getPath('assets/origin/1.hca'), getPath('assets/test/decodeToWaveFileSync/1.hca'))
    fs.copySync(getPath('assets/origin/2.hca'), getPath('assets/test/decodeToWaveFileSync/2.hca'))
    fs.copySync(getPath('assets/origin/3.hca'), getPath('assets/test/decodeToWaveFileSync/3.hca'))

    assert.ok(hd.decodeToWaveFileSync(getPath('assets/test/decodeToWaveFileSync/1.hca')) === getPath('assets/test/decodeToWaveFileSync/1.wav'))
    assert.ok(hd.decodeToWaveFileSync(getPath('assets/test/decodeToWaveFileSync/2.hca'), null, 0.2) === getPath('assets/test/decodeToWaveFileSync/2.wav'))
    assert.ok(hd.decodeToWaveFileSync(getPath('assets/test/decodeToWaveFileSync/3.hca'), null, 1, 8) === getPath('assets/test/decodeToWaveFileSync/3.wav'))
  })

  it('#decodeToMemory Promise All', function (done) {
    this.timeout(Infinity)
    fs.mkdirsSync(getPath('assets/test/decodeToMemoryPromiseAll'))
    fs.copySync(getPath('assets/origin/1.hca'), getPath('assets/test/decodeToMemoryPromiseAll/1.hca'))
    fs.copySync(getPath('assets/origin/2.hca'), getPath('assets/test/decodeToMemoryPromiseAll/2.hca'))
    fs.copySync(getPath('assets/origin/3.hca'), getPath('assets/test/decodeToMemoryPromiseAll/3.hca'))

    const decode = (hd, hca) => {
      return new Promise((resolve, reject) => {
        hd.decodeToMemory(hca, function (err, buf) {
          if (err) reject(err)
          else resolve(buf)
        })
      })
    }

    Promise.all([
      decode(hd, getPath('assets/test/decodeToMemoryPromiseAll/1.hca')),
      decode(hd, getPath('assets/test/decodeToMemoryPromiseAll/2.hca')),
      decode(hd, getPath('assets/test/decodeToMemoryPromiseAll/3.hca'))
    ]).then(([a, b, c]) => {
      assert.ok(Buffer.isBuffer(a))
      assert.ok(Buffer.isBuffer(b))
      assert.ok(Buffer.isBuffer(c))
      fs.writeFileSync(getPath('assets/test/decodeToMemoryPromiseAll/1.wav'), a)
      fs.writeFileSync(getPath('assets/test/decodeToMemoryPromiseAll/2.wav'), b)
      fs.writeFileSync(getPath('assets/test/decodeToMemoryPromiseAll/3.wav'), c)
      done()
    }).catch(err => {
      done(err)
    })
  })

  it('#decodeToMemorySync', function () {
    this.timeout(Infinity)
    fs.mkdirsSync(getPath('assets/test/decodeToMemorySync'))
    fs.copySync(getPath('assets/origin/1.hca'), getPath('assets/test/decodeToMemorySync/1.hca'))
    fs.copySync(getPath('assets/origin/2.hca'), getPath('assets/test/decodeToMemorySync/2.hca'))
    fs.copySync(getPath('assets/origin/3.hca'), getPath('assets/test/decodeToMemorySync/3.hca'))

    let a, b, c
    assert.ok(Buffer.isBuffer(a = hd.decodeToMemorySync(getPath('assets/test/decodeToMemorySync/1.hca'))))
    assert.ok(Buffer.isBuffer(b = hd.decodeToMemorySync(getPath('assets/test/decodeToMemorySync/2.hca'), 0.2)))
    assert.ok(Buffer.isBuffer(c = hd.decodeToMemorySync(getPath('assets/test/decodeToMemorySync/3.hca'), 1, 8)))
    fs.writeFileSync(getPath('assets/test/decodeToMemorySync/1.wav'), a)
    fs.writeFileSync(getPath('assets/test/decodeToMemorySync/2.wav'), b)
    fs.writeFileSync(getPath('assets/test/decodeToMemorySync/3.wav'), c)
  })

  it('$getInfo', function () {
    this.timeout(Infinity)
    assert.ok(typeof HCADecoder.getInfo === 'function')
    fs.mkdirsSync(getPath('assets/test/getInfo'))
    fs.copySync(getPath('assets/origin/1.hca'), getPath('assets/test/getInfo/1.hca'))

    const info = HCADecoder.getInfo(getPath('assets/test/getInfo/1.hca'))
    assert.ok(typeof info === 'object' && info !== null)
    assert.ok(typeof info.loop === 'object' && info.loop !== null)
    console.log(info)
  })
})
