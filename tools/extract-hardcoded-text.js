#!/usr/bin/env node

const fs = require("fs");
const path = require("path");

const ROOT = process.cwd();
const SRC_DIR = path.join(ROOT, "src");
const OUT_DIR = path.join(ROOT, "localization");

const SOURCE_EXTS = new Set([".c", ".h", ".m", ".rc"]);
const PRIORITY_PATTERNS = [
  { category: "message", re: /\b(?:msg_print|msg_format|cmsg_print|cmsg_format|bell|msg_boundary)\s*\(/ },
  { category: "prompt", re: /\b(?:get_check|get_string|get_com|get_quantity|inkey_special|askfor_aux)\s*\(|\bprompt\.(?:prompt|error)\s*=/ },
  { category: "display", re: /\b(?:prt|put_str|c_put_str|c_prt|Term_putstr|Term_addstr|doc_printf|doc_insert|doc_insert_text)\s*\(/ },
  { category: "name_desc", re: /\bvar_set_string\s*\(|\.(?:name|desc|title|caption|help|text|label|short_name)\s*=/ },
  { category: "file_ui", re: /\b(?:plr_display|display_player|show_file|screen_roff|roff|spoiler|dump|write)\b/ },
];

function walk(dir, out = []) {
  for (const entry of fs.readdirSync(dir, { withFileTypes: true })) {
    const full = path.join(dir, entry.name);
    if (entry.isDirectory()) {
      if (entry.name === "cocoa") {
        walk(full, out);
      } else {
        walk(full, out);
      }
    } else if (SOURCE_EXTS.has(path.extname(entry.name).toLowerCase())) {
      out.push(full);
    }
  }
  return out;
}

function rel(file) {
  return path.relative(ROOT, file).replace(/\\/g, "/");
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

function lineAt(text, line) {
  const lines = text.split(/\r?\n/);
  return (lines[line - 1] || "").trim().replace(/\s+/g, " ");
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

function escaped(value) {
  return String(value)
    .replace(/\\/g, "\\\\")
    .replace(/\t/g, "\\t")
    .replace(/\r/g, "\\r")
    .replace(/\n/g, "\\n");
}

function tsv(rows) {
  const header = ["id", "priority", "category", "file", "line", "col", "source", "zh_cn", "context", "notes"];
  const body = rows.map((r) => header.map((k) => escaped(r[k] ?? "")).join("\t"));
  return `${header.join("\t")}\n${body.join("\n")}\n`;
}

function looksLikeText(value) {
  const s = value.trim();
  if (!s) return false;
  if (s.length === 1) return false;
  const stripped = s
    .replace(/<[^>]+>/g, "")
    .replace(/%[-+#0-9. *]*[A-Za-z%]/g, "")
    .replace(/\\[nrt]/g, " ")
    .replace(/[0-9_~`!@#$%^&*()+={}\[\]|\\:;"'<>,.?/ -]/g, "");
  return /[A-Za-z]/.test(stripped);
}

function isMostlyTechnical(value, context) {
  const s = value.trim();
  if (!s) return true;
  if (/^<\/?(?:style|color|indent|topic|tab|link|key|font)[^>]*>$/.test(s)) return true;
  if (/^[A-Z0-9_]+$/.test(s) && s.length > 3) return true;
  if (/^[a-z0-9_]+$/.test(s) && /(?:_|\d)/.test(s) && !/\b(?:name|desc|prompt|msg|print|format)\b/.test(context)) return true;
  if (/^[./\\A-Za-z0-9_-]+\.(?:txt|raw|prf|bmp|png|fon|wav|cfg|ini|html|doc|sav|old|new|tmp|dll|exe|ico)$/i.test(s)) return true;
  if (/^https?:\/\//i.test(s)) return true;
  return false;
}

function classify(context, source) {
  for (const p of PRIORITY_PATTERNS) {
    if (p.re.test(context)) return p.category;
  }
  if (/\.rc$/i.test(context)) return "resource";
  if (/^[A-Z][A-Za-z' -]{2,}$/.test(source.trim())) return "possible_name";
  return "other";
}

function priorityFor(category, source, context) {
  if (["message", "prompt", "display", "name_desc", "resource"].includes(category)) return "high";
  if (category === "file_ui") return "medium";
  if (category === "possible_name" && source.trim().length <= 48) return "medium";
  if (/\b(?:menu|choice|option|spell|power|class|race|realm|store|quest|status)\b/i.test(context)) return "medium";
  return "low";
}

function collect() {
  const rows = [];
  const files = walk(SRC_DIR).sort();
  for (const file of files) {
    const content = fs.readFileSync(file, "utf8");
    const tokens = mergeAdjacent(content, extractTokens(content));
    for (const token of tokens) {
      const pos = lineColFromIndex(content, token.start);
      const context = lineAt(content, pos.line);
      if (!looksLikeText(token.text)) continue;
      const category = classify(`${rel(file)} ${context}`, token.text);
      const priority = priorityFor(category, token.text, context);
      const notes = isMostlyTechnical(token.text, context) ? "likely technical; review before translating" : "";
      rows.push({
        priority,
        category,
        file: rel(file),
        line: pos.line,
        col: pos.col,
        source: token.text,
        zh_cn: "",
        context,
        notes,
      });
    }
  }
  return rows.map((row, idx) => ({ id: `SRC_${String(idx + 1).padStart(5, "0")}`, ...row }));
}

function summarize(rows, priorityRows) {
  const byPriority = {};
  const byCategory = {};
  const byFile = {};
  for (const row of rows) {
    byPriority[row.priority] = (byPriority[row.priority] || 0) + 1;
    byCategory[row.category] = (byCategory[row.category] || 0) + 1;
    byFile[row.file] = (byFile[row.file] || 0) + 1;
  }
  const topFiles = Object.entries(byFile)
    .sort((a, b) => b[1] - a[1])
    .slice(0, 20)
    .map(([file, count]) => `${String(count).padStart(5)}  ${file}`)
    .join("\n");
  return [
    "Hardcoded text extraction summary",
    "",
    `Source root: src`,
    `Files scanned: ${walk(SRC_DIR).length}`,
    `All extracted text rows: ${rows.length}`,
    `Priority rows (high/medium): ${priorityRows.length}`,
    "",
    "By priority:",
    ...Object.entries(byPriority).sort().map(([k, v]) => `  ${k}: ${v}`),
    "",
    "By category:",
    ...Object.entries(byCategory).sort().map(([k, v]) => `  ${k}: ${v}`),
    "",
    "Top files by extracted row count:",
    topFiles,
    "",
    "Notes:",
    "- lib/help was not scanned; this export focuses on source hardcoded text under src.",
    "- Rows are occurrence-based, not deduplicated, so the file/line can be used for patching.",
    "- Preserve C printf placeholders such as %s/%d and game markup such as <color:y> when translating.",
    "- Rows marked likely technical should be reviewed before translation.",
    "",
  ].join("\n");
}

fs.mkdirSync(OUT_DIR, { recursive: true });
const rows = collect();
const priorityRows = rows.filter((r) => r.priority !== "low" && !r.notes);

fs.writeFileSync(path.join(OUT_DIR, "hardcoded_text_all.tsv"), tsv(rows), "utf8");
fs.writeFileSync(path.join(OUT_DIR, "hardcoded_text_priority.tsv"), tsv(priorityRows), "utf8");
fs.writeFileSync(path.join(OUT_DIR, "hardcoded_text_summary.txt"), summarize(rows, priorityRows), "utf8");

console.log(`Scanned src and wrote ${rows.length} rows (${priorityRows.length} priority rows) to localization/.`);
