function getValue(name) { return document.getElementsByName(name)[0].value }
function setStrValue(name, val) { document.getElementsByName(name)[0].value = val }

const mod = {}

fetch('add.wasm')
  .then(response => response.arrayBuffer())
  .then(mod => {
    const imports = {
      env : {
        addJS : (x, y) => { return x + y }
      }
    }
    return WebAssembly.instantiate(mod, imports)
  })
  .then(ret => mod.exports = ret.instance.exports)

function test_add() {
  const x = getValue('ret1')
  const y = mod.exports.add(x, 5)
  setStrValue('ret1', y)
  console.log(`subsub=${mod.exports.subsub(2, 9)}`)
  console.log(`callJS=${mod.exports.callJS(1000)}`)
}

