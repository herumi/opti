function getValue(name) { return document.getElementsByName(name)[0].value }
function setValue(name, val) { document.getElementsByName(name)[0].value = val }
function getText(name) { return document.getElementsByName(name)[0].innerText }
function setText(name, val) { document.getElementsByName(name)[0].innerText = val }

let mod = {}
function setupWasm(fileName, setupFct) {
  console.log('setupWasm ' + fileName)
  fetch(fileName)
    .then(response => response.arrayBuffer())
    .then(buffer => new Uint8Array(buffer))
    .then(() => {
      mod = Module()
      mod.fillJS = (p, n) => {
        a = new Uint8Array(n)
        const crypto = window.crypto || window.msCrypto
        crypto.getRandomValues(a)
        for (let i = 0; i < n; i++) {
          mod.HEAP8[p + i] = a[i]
        }
        return n * 2
      }
      mod.callAdd1 = (x, y) => {
        return x + y
      }
      mod.onRuntimeInitialized = () => {
        console.log('setupWasm end')
        mod._malloc = mod._malloc_
        mod._free = mod._free_
        console.log('malloc=' + mod._malloc)
      }
    })
}

setupWasm('fill.wasm', function() {
})

function bench(msg, count, func) {
  let start = Date.now()
  for (let i = 0; i < count; i++) {
    func()
  }
  let end = Date.now()
  let t = (end - start) / count
  setText(msg, (t * 1e6) + 'usec')
}

function test_fill() {
  let n = 8
  let pos = mod._malloc(n)
  console.log('pos=' + pos)
  console.log('fill1')
  mod._fill1(pos, n)
  for (let i = 0; i < n; i++) {
    console.log(i + ':' + mod.HEAP8[pos + i])
  }
  console.log('fill2')
  const ret = mod._fill2(pos, n)
  console.log(`ret=${ret}`)
  for (let i = 0; i < n; i++) {
    console.log(i + ':' + mod.HEAP8[pos + i])
  }
  const C = 100000
  bench('bench_fill1', C, () => { mod._fill1(pos, n) })
  bench('bench_fill2', C, () => { mod._fill2(pos, n) })
  mod._free(pos)
  console.log('a=' + a)
}

function test() {
  let x = 8
  let y = 10
  let z = 0
  z = mod._call0(x, y)
  setText('test0', (x + y == z))
  z = mod._call1(x, y)
  setText('test1', (x + y == z))
  z = mod._call2(x, y)
  setText('test2', (x + y == z))
}

function test_bench() {
  var x = 3
  var y = 4
  const C = 1000000
  bench('bench_test0', C, () => { x = mod._call0(x, y) })
  bench('bench_test1', C, () => { x = mod._call1(x, y) })
  bench('bench_test2', C, () => { x = mod._call2(x, y) })
  bench('bench_testJS', C, () => { x = x + y })
}
