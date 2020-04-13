const mod = {}

async function init () {
  const fs = require('fs')
  const buf = fs.readFileSync('./addsub.wasm')
  const memory = await new WebAssembly.Memory({initial:10})
  console.log(`memory=${memory}`)
  const imports = {
    env : {
      memory: memory,
    }
  }
  const ret = await WebAssembly.instantiate(buf, imports)
  return ret.instance.exports
} 

(async () => {
  mod.exports = await init()
  const exports = mod.exports
  console.log(`add=${exports.add(12, 34)}`)
  console.log(`sub=${exports.sub(12, 34)}`)
})()
