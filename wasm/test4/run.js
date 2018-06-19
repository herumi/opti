function getValue(name) { return document.getElementsByName(name)[0].value }
function setStrValue(name, val) { document.getElementsByName(name)[0].value = val }

const mod = {}

async function init() {
  const response = await fetch('./add.wasm')
  const buf = await response.arrayBuffer()
  const memory = await new WebAssembly.Memory({initial:1})
  const imports = {
    env : {
      addJS : (x, y) => { return x + y },
      memory: memory,
    }
  }
  const ret = await WebAssembly.instantiate(buf, imports)
  return ret.instance.exports
}

(async () => {
  mod.exports = await init()
  mod.u32 = new Uint32Array(mod.exports.memory.buffer)
  mod.u8 = new Uint8Array(mod.exports.memory.buffer)
})()

function test_add() {
  const x = getValue('ret1')
  const y = mod.exports.add(x, 5)
  setStrValue('ret1', y)
  console.log(`subsub=${mod.exports.subsub(2, 9)}`)
  console.log(`callJS=${mod.exports.callJS(1000)}`)
}

