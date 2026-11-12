// Independent ECMAScript oracle for RFC8785. Test-only; never a runtime dependency.
// Usage: node capability_jcs_reference.js <ui_capability_json_test.exe>
'use strict';
const {spawnSync} = require('node:child_process');
const assert = require('node:assert/strict');
function jcs(v) {
  if (Array.isArray(v)) return '[' + v.map(jcs).join(',') + ']';
  if (v !== null && typeof v === 'object')
    return '{' + Object.keys(v).sort().map(k => JSON.stringify(k) + ':' + jcs(v[k])).join(',') + '}';
  return JSON.stringify(v);
}
const values = [null, true, false, -0, 1e-6, 1e-7, 1e20, 1e21,
  Number.MIN_VALUE, Number.MAX_VALUE, 333333333.33333329,
  {'\ue000': 1, '\u{10000}': 2, '\u0000': '\b\t\n\f\r/\\"', 'e\u0301': 3, '\u00e9': 4}];
let state = 0x91c034d2;
const rand = () => (state = (Math.imul(state, 1664525) + 1013904223) >>> 0);
for (let i = 0; i < 256; ++i) {
  const bytes = Buffer.alloc(8); bytes.writeUInt32LE(rand(), 0); bytes.writeUInt32LE(rand(), 4);
  const d = bytes.readDoubleLE(); if (Number.isFinite(d)) values.push({z: [d, i], a: '中文😀'});
}
const inputs = values.map(JSON.stringify);
const run = spawnSync(process.argv[2], ['--canonical'], {input: inputs.join('\n') + '\n', encoding: 'utf8', maxBuffer: 1024 * 1024});
assert.equal(run.error, undefined); assert.equal(run.status, 0, run.stdout + run.stderr);
const output = run.stdout.trimEnd().split(/\r?\n/); assert.equal(output.length, values.length);
for (let i = 0; i < values.length; ++i) assert.equal(output[i], jcs(values[i]), 'vector ' + i);
console.log(`JCS independent ECMAScript reference: ${values.length} vectors passed`);
