#!/usr/bin/env node

const fs = require("fs");
const path = require("path");

const ROOT = process.cwd();
const MISSING = path.join(ROOT, "localization", "hardcoded_text_audit_missing.tsv");
const REPORT = path.join(ROOT, "localization", "hardcoded_text_missing_backfill_report.txt");

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

function decodeCString(raw) {
  let out = "";
  for (let i = 0; i < raw.length; i++) {
    const ch = raw[i];
    if (ch !== "\\") {
      out += ch;
      continue;
    }
    const n = raw[++i];
    if (n === "n") out += "\n";
    else if (n === "r") out += "\r";
    else if (n === "t") out += "\t";
    else if (n === "b") out += "\b";
    else if (n === "f") out += "\f";
    else if (n === "v") out += "\v";
    else if (n === "a") out += "\x07";
    else if (n === "\\") out += "\\";
    else if (n === '"') out += '"';
    else if (n === "'") out += "'";
    else out += n || "";
  }
  return out;
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

function skipCharLiteral(text, i) {
  i++;
  while (i < text.length) {
    if (text[i] === "\\") i += 2;
    else if (text[i] === "'") return i + 1;
    else if (text[i] === "\n") return i;
    else i++;
  }
  return i;
}

function parseString(text, quoteIndex) {
  let i = quoteIndex + 1;
  let raw = "";
  while (i < text.length) {
    if (text[i] === "\\") {
      raw += text[i];
      if (i + 1 < text.length) raw += text[i + 1];
      i += 2;
    } else if (text[i] === '"') {
      return { raw, end: i + 1 };
    } else {
      raw += text[i++];
    }
  }
  return { raw, end: i };
}

function extractTokens(text) {
  const tokens = [];
  for (let i = 0; i < text.length;) {
    if (text[i] === "/" && text[i + 1] === "/") {
      i += 2;
      while (i < text.length && text[i] !== "\n") i++;
    } else if (text[i] === "/" && text[i + 1] === "*") {
      i += 2;
      while (i < text.length && !(text[i] === "*" && text[i + 1] === "/")) i++;
      i += 2;
    } else if (text[i] === "'") {
      i = skipCharLiteral(text, i);
    } else if (text[i] === '"') {
      const start = i;
      const parsed = parseString(text, i);
      tokens.push({ start, end: parsed.end, text: decodeCString(parsed.raw) });
      i = parsed.end;
    } else {
      i++;
    }
  }
  return tokens;
}

function normalizeText(value) {
  return value.replace(/\s+/g, " ").trim();
}

const rows = parseTsv(MISSING).filter((r) => r.source && r.zh_cn && r.zh_cn !== r.source);
const byFile = new Map();
for (const row of rows) {
  if (!byFile.has(row.file)) byFile.set(row.file, []);
  byFile.get(row.file).push(row);
}

let applied = 0;
let skipped = 0;
const report = [];

for (const [file, fileRows] of byFile) {
  const abs = path.join(ROOT, file);
  let content = fs.readFileSync(abs, "utf8");
  const replacements = [];
  const tokens = extractTokens(content);
  const used = new Set();

  for (const row of fileRows) {
    let idx = tokens.findIndex((t, i) => !used.has(i) && t.text === row.source);
    if (idx < 0) {
      idx = tokens.findIndex((t, i) => !used.has(i) && normalizeText(t.text) === normalizeText(row.source));
    }
    if (idx < 0) {
      skipped++;
      report.push(`NO_MATCH ${row.id} ${file}:${row.line}:${row.col} ${JSON.stringify(row.source)}`);
      continue;
    }
    used.add(idx);
    replacements.push({ start: tokens[idx].start, end: tokens[idx].end, value: encodeCString(row.zh_cn) });
  }

  replacements.sort((a, b) => b.start - a.start);
  for (const replacement of replacements) {
    content = content.slice(0, replacement.start) + replacement.value + content.slice(replacement.end);
    applied++;
  }
  if (replacements.length) fs.writeFileSync(abs, content, "utf8");
}

report.unshift(`Skipped: ${skipped}`);
report.unshift(`Applied: ${applied}`);
fs.writeFileSync(REPORT, report.join("\n") + "\n", "utf8");
console.log(`Applied missing translations: ${applied}`);
console.log(`Skipped missing translations: ${skipped}`);
console.log(`Report: ${path.relative(ROOT, REPORT)}`);
