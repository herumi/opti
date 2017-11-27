function getValue(name) { return document.getElementsByName(name)[0].value }
function setStrValue(name, val) { document.getElementsByName(name)[0].value = val }

let mod = {}
fetch('func.wasm')
  .then(response => response.arrayBuffer())
  .then(buffer => new Uint8Array(buffer))
  .then(binary => { Module(mod) })

function test_itos() {
  const x = -1
  const bufSize = 20
  const buf = mod._malloc(bufSize)
  const n = mod._itos(buf, bufSize, x)
  console.log('n = ' + n)
  let s = ''
  for (let i = 0; i < n; i++) {
    s += String.fromCharCode(mod.HEAP8[buf + i])
  }
  console.log('s = ' + s)
  mod._free(buf)
}
