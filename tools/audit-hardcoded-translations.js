#!/usr/bin/env node

const fs = require("fs");
const path = require("path");

const ROOT = process.cwd();
const TSV = path.join(ROOT, "localization", "hardcoded_text_priority.tsv");
const OUT = path.join(ROOT, "localization", "hardcoded_text_audit_missing.tsv");

function unescapeTsv(value) {
  let out = "";
  for (let i = 0; i < value.length; i++) {
    const ch = value[i];
    if (ch !== "\\") {
      out += ch;
      continue;
    }
    const n = value[++i];
    if (n === "n") out += "\n";
    else if (n === "r") out += "\r";
    else if (n === "t") out += "\t";
    else if (n === "\\") out += "\\";
    else if (n === undefined) out += "\\";
    else out += "\\" + n;
  }
  return out;
}

function parseTsv(file) {
  const lines = fs.readFileSync(file, "utf8").split(/\r?\n/).filter(Boolean);
  const header = lines.shift().split("\t");
  return lines.map((line) => {
    const cols = line.split("\t");
    const row = {};
    header.forEach((h, i) => (row[h] = unescapeTsv(cols[i] || "")));
    return row;
  });
}

function encodeCString(value) {
  let out = '"';
  for (const ch of value) {
    const cp = ch.codePointAt(0);
    if (ch === "\\") out += "\\\\";
    else if (ch === '"') out += '\\"';
    else if (ch === "\n") out += "\\n";
    else if (ch === "\r") out += "\\r";
    else if (ch === "\t") out += "\\t";
    else if (ch === "\b") out += "\\b";
    else if (ch === "\f") out += "\\f";
    else if (ch === "\v") out += "\\v";
    else if (cp < 0x20 || cp === 0x7f) out += `\\x${cp.toString(16).padStart(2, "0")}`;
    else out += ch;
  }
  return out + '"';
}

function tsvEscape(value) {
  return String(value)
    .replace(/\\/g, "\\\\")
    .replace(/\t/g, "\\t")
    .replace(/\r/g, "\\r")
    .replace(/\n/g, "\\n");
}

const rows = parseTsv(TSV).filter((r) => r.zh_cn.trim() && r.zh_cn !== r.source);
const byFile = new Map();
for (const row of rows) {
  if (!byFile.has(row.file)) byFile.set(row.file, []);
  byFile.get(row.file).push(row);
}

let applied = 0;
const missing = [];
for (const [file, fileRows] of byFile) {
  const content = fs.readFileSync(path.join(ROOT, file), "utf8");
  const counts = new Map();
  for (const row of fileRows) {
    const encoded = encodeCString(row.zh_cn);
    if (counts.has(encoded)) continue;
    let count = 0;
    let offset = 0;
    while ((offset = content.indexOf(encoded, offset)) !== -1) {
      count++;
      offset += encoded.length;
    }
    counts.set(encoded, count);
  }
  for (const row of fileRows) {
    const encoded = encodeCString(row.zh_cn);
    const count = counts.get(encoded) || 0;
    if (count > 0) {
      counts.set(encoded, count - 1);
      applied++;
    } else {
      missing.push(row);
    }
  }
}

const header = ["id", "file", "line", "col", "source", "zh_cn"];
const lines = [header.join("\t")].concat(
  missing.map((r) => header.map((h) => tsvEscape(r[h] || "")).join("\t"))
);
fs.writeFileSync(OUT, lines.join("\n") + "\n", "utf8");

console.log(`Rows with changed translations: ${rows.length}`);
console.log(`Found translated C strings: ${applied}`);
console.log(`Missing translated C strings: ${missing.length}`);
console.log(`Missing list: ${path.relative(ROOT, OUT)}`);
