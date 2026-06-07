#!/usr/bin/env node

const fs = require("fs");
const path = require("path");

const ROOT = process.cwd();
const TSV = path.join(ROOT, "localization", "hardcoded_text_priority.tsv");
const REPORT = path.join(ROOT, "localization", "hardcoded_text_backfill_report.txt");

function unescapeTsv(value) {
  let out = "";
  for (let i = 0; i < value.length; i++) {
    const ch = value[i];
    if (ch !== "\\") {
      out += ch;
      continue;
    }
    const n = value[++i];
    if (n === undefined) {
      out += "\\";
    } else if (n === "n") {
      out += "\n";
    } else if (n === "r") {
      out += "\r";
    } else if (n === "t") {
      out += "\t";
    } else if (n === "\\") {
      out += "\\";
    } else {
      out += "\\" + n;
    }
  }
  return out;
}

function parseTsv(file) {
  const lines = fs.readFileSync(file, "utf8").split(/\r?\n/).filter(Boolean);
  const header = lines.shift().split("\t");
  const rows = [];
  for (const line of lines) {
    const cols = line.split("\t");
    const row = {};
    for (let i = 0; i < header.length; i++) row[header[i]] = unescapeTsv(cols[i] || "");
    rows.push(row);
  }
  return rows;
}

function lineColFromIndex(text, index) {
  let line = 1;
  let lastNl = -1;
  for (let i = 0; i < index; i++) {
    if (text.charCodeAt(i) === 10) {
      line++;
      lastNl = i;
    }
  }
  return { line, col: index - lastNl };
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
    if (n === undefined) break;
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
    else if (n === "x") {
      let hex = "";
      while (/[0-9a-fA-F]/.test(raw[i + 1] || "")) hex += raw[++i];
      out += hex ? String.fromCharCode(parseInt(hex.slice(-2), 16)) : "x";
    } else if (/[0-7]/.test(n)) {
      let oct = n;
      for (let j = 0; j < 2 && /[0-7]/.test(raw[i + 1] || ""); j++) oct += raw[++i];
      out += String.fromCharCode(parseInt(oct, 8));
    } else if (n === "\r" && raw[i + 1] === "\n") {
      i++;
    } else if (n === "\n") {
      // Escaped physical newline.
    } else {
      out += n;
    }
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
    if (text[i] === "\\") {
      i += 2;
      continue;
    }
    if (text[i] === "'") return i + 1;
    if (text[i] === "\n") return i;
    i++;
  }
  return i;
}

function parseString(text, quoteIndex) {
  let i = quoteIndex + 1;
  let raw = "";
  while (i < text.length) {
    const ch = text[i];
    if (ch === "\\") {
      raw += ch;
      if (i + 1 < text.length) raw += text[i + 1];
      i += 2;
      continue;
    }
    if (ch === '"') return { raw, end: i + 1 };
    raw += ch;
    i++;
  }
  return { raw, end: i };
}

function extractTokens(text) {
  const tokens = [];
  let i = 0;
  while (i < text.length) {
    const ch = text[i];
    if (ch === "/" && text[i + 1] === "/") {
      i += 2;
      while (i < text.length && text[i] !== "\n") i++;
      continue;
    }
    if (ch === "/" && text[i + 1] === "*") {
      i += 2;
      while (i < text.length && !(text[i] === "*" && text[i + 1] === "/")) i++;
      i += 2;
      continue;
    }
    if (ch === "'") {
      i = skipCharLiteral(text, i);
      continue;
    }
    if (ch === '"') {
      const start = i;
      const parsed = parseString(text, i);
      tokens.push({ start, end: parsed.end, raw: parsed.raw, text: decodeCString(parsed.raw) });
      i = parsed.end;
      continue;
    }
    i++;
  }
  return tokens;
}

function onlyWhitespaceAndComments(text) {
  let i = 0;
  while (i < text.length) {
    if (/\s/.test(text[i])) {
      i++;
      continue;
    }
    if (text[i] === "/" && text[i + 1] === "/") {
      i += 2;
      while (i < text.length && text[i] !== "\n") i++;
      continue;
    }
    if (text[i] === "/" && text[i + 1] === "*") {
      i += 2;
      while (i < text.length && !(text[i] === "*" && text[i + 1] === "/")) i++;
      i += 2;
      continue;
    }
    return false;
  }
  return true;
}

function mergeAdjacent(text, tokens) {
  const merged = [];
  for (const token of tokens) {
    const prev = merged[merged.length - 1];
    if (prev && onlyWhitespaceAndComments(text.slice(prev.end, token.start))) {
      prev.end = token.end;
      prev.raw += token.raw;
      prev.text += token.text;
    } else {
      merged.push({ ...token });
    }
  }
  return merged;
}

function rel(file) {
  return file.replace(/\\/g, "/");
}

function applyTranslations(rows) {
  const byFile = new Map();
  for (const row of rows) {
    const zh = row.zh_cn.trim();
    if (!zh || zh === row.source) continue;
    if (!byFile.has(row.file)) byFile.set(row.file, []);
    byFile.get(row.file).push(row);
  }

  const report = [];
  let applied = 0;
  let alreadyApplied = 0;
  let skipped = 0;
  let missing = 0;

  for (const [file, fileRows] of byFile) {
    const abs = path.join(ROOT, file);
    if (!fs.existsSync(abs)) {
      missing += fileRows.length;
      report.push(`MISSING_FILE ${file} (${fileRows.length} rows)`);
      continue;
    }
    const content = fs.readFileSync(abs, "utf8");
    const tokens = mergeAdjacent(content, extractTokens(content));
    const index = new Map();
    const byPosition = new Map();
    const byLine = new Map();
    for (const token of tokens) {
      const pos = lineColFromIndex(content, token.start);
      token.line = pos.line;
      token.col = pos.col;
      const key = `${pos.line}:${pos.col}:${token.text}`;
      if (!index.has(key)) index.set(key, []);
      index.get(key).push(token);
      const posKey = `${pos.line}:${pos.col}`;
      if (!byPosition.has(posKey)) byPosition.set(posKey, []);
      byPosition.get(posKey).push(token);
      if (!byLine.has(pos.line)) byLine.set(pos.line, []);
      byLine.get(pos.line).push(token);
    }

    const replacements = [];
    for (const row of fileRows) {
      const key = `${row.line}:${row.col}:${row.source}`;
      const candidates = index.get(key) || [];
      let token = candidates.shift();
      if (!token) {
        const samePosition = byPosition.get(`${row.line}:${row.col}`) || [];
        token = samePosition.find((t) => normalizeText(t.text) === normalizeText(row.source));
      }
      if (!token) {
        const sameLine = byLine.get(Number(row.line)) || [];
        token = sameLine.find((t) => normalizeText(t.text) === normalizeText(row.source));
      }
      if (!token) {
        const samePosition = byPosition.get(`${row.line}:${row.col}`) || [];
        if (samePosition.some((t) => t.text === row.zh_cn)) {
          alreadyApplied++;
          continue;
        }
      }
      if (!token) {
        skipped++;
        report.push(`NO_MATCH ${row.id} ${file}:${row.line}:${row.col} ${JSON.stringify(row.source)}`);
        continue;
      }
      replacements.push({ start: token.start, end: token.end, value: encodeCString(row.zh_cn) });
    }

    if (!replacements.length) continue;
    replacements.sort((a, b) => b.start - a.start);
    let next = content;
    for (const r of replacements) {
      next = next.slice(0, r.start) + r.value + next.slice(r.end);
      applied++;
    }
    fs.writeFileSync(abs, next, "utf8");
  }

  report.unshift(`Applied: ${applied}`);
  report.unshift(`Already applied: ${alreadyApplied}`);
  report.unshift(`Skipped no match: ${skipped}`);
  report.unshift(`Missing file rows: ${missing}`);
  fs.writeFileSync(REPORT, report.join("\n") + "\n", "utf8");
  return { applied, alreadyApplied, skipped, missing };
}

function normalizeText(value) {
  return value.replace(/\s+/g, " ").trim();
}

const rows = parseTsv(TSV);
const translated = rows.filter((r) => r.zh_cn.trim()).length;
const result = applyTranslations(rows);
console.log(`Translated rows in TSV: ${translated}`);
console.log(`Applied replacements: ${result.applied}`);
console.log(`Already applied: ${result.alreadyApplied}`);
console.log(`Skipped no match: ${result.skipped}`);
console.log(`Missing file rows: ${result.missing}`);
console.log(`Report: ${path.relative(ROOT, REPORT)}`);
