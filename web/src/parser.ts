/**
 * Pure TypeScript parser for EduLang C++ Compiler outputs.
 * Parses terminal streams (stdout & stderr) into structured JSON data.
 * Zero external dependencies.
 */

export interface TokenItem {
  line: number;
  column: number;
  type: string;
  lexeme: string;
}

export interface AstNode {
  name: string;
  level: number;
  children: AstNode[];
}

export interface SymbolEntry {
  sourceName: string;
  storageName: string;
  scope: string;
  status: string;
  type: string;
  line: number;
  column: number;
}

export interface TacInstruction {
  index: number;
  raw: string;
  isLabel: boolean;
  label?: string;
  op: string;
  args: string[];
}

export interface OptimizationInfo {
  instructionsBefore?: number;
  instructionsAfter?: number;
  constantsFolded: number;
  reductionRate?: string;
  message?: string;
}

export interface AsmInstruction {
  raw: string;
  isComment: boolean;
  isLabel: boolean;
  label?: string;
  mnemonic?: string;
  operands?: string;
}

export interface DiagnosticItem {
  file: string;
  line: number;
  column: number;
  severity: 'error' | 'warning';
  message: string;
  sourceLine?: string;
  caret?: string;
  hint?: string;
}

export interface PipelineStatusItem {
  stage: string;
  status: 'PASS' | 'FAIL' | 'SKIP';
  details?: string;
}

export interface ParsedStages {
  pipelineStatuses: PipelineStatusItem[];
  tokens: TokenItem[];
  ast: AstNode | null;
  symbols: SymbolEntry[];
  tac: TacInstruction[];
  optimization: OptimizationInfo | null;
  targetCode: AsmInstruction[];
  runtimeOutput: string;
  diagnostics: DiagnosticItem[];
  parsingStatus: {
    tokensParsed: boolean;
    astParsed: boolean;
    symbolsParsed: boolean;
    tacParsed: boolean;
    optParsed: boolean;
    targetParsed: boolean;
    diagnosticsParsed: boolean;
    runtimeOutputCaptured: boolean;
    warnings: string[];
  };
}

/**
 * Sanitizes temporary file paths like /var/folders/.../edulang_123_abc.edu into a clean display name.
 */
function sanitizePath(pathStr: string): string {
  return pathStr.replace(/(?:[A-Za-z]:)?(?:[\\/].*?[\\/])?edulang_[0-9]+_[a-f0-9]+\.edu/g, 'input.edu');
}

/**
 * Parses pipeline stage statuses ([PASS], [FAIL], [SKIP]).
 */
export function parsePipelineStatuses(stdout: string): PipelineStatusItem[] {
  const statuses: PipelineStatusItem[] = [];
  const statusRegex = /^\[(PASS|FAIL|SKIP)\]\s+([^(]+?)(?:\s+\((.*?)\))?$/;

  const lines = stdout.split('\n');
  for (const line of lines) {
    const trimmed = line.trim();
    const match = trimmed.match(statusRegex);
    if (match) {
      statuses.push({
        status: match[1] as 'PASS' | 'FAIL' | 'SKIP',
        stage: match[2].trim(),
        details: match[3]?.trim(),
      });
    }
  }
  return statuses;
}

/**
 * Parses the token stream output from 'Tokens\n' section.
 */
export function parseTokens(stdout: string): { tokens: TokenItem[]; success: boolean } {
  const tokens: TokenItem[] = [];
  const startIdx = stdout.indexOf('Tokens\n');
  if (startIdx === -1) {
    return { tokens, success: false };
  }

  // Tokens section ends at AST, Symbol Table, or Pipeline Status
  const sectionStart = startIdx + 'Tokens\n'.length;
  const delimiters = ['\nAST\n', '\nSymbol Table\n', '\nPipeline Status\n', '\nIntermediate Code\n'];
  let endIdx = stdout.length;

  for (const delim of delimiters) {
    const idx = stdout.indexOf(delim, sectionStart);
    if (idx !== -1 && idx < endIdx) {
      endIdx = idx;
    }
  }

  const tokenText = stdout.substring(sectionStart, endIdx);
  const lines = tokenText.split('\n');
  const tokenRegex = /^(\d+):(\d+)\s+([a-zA-Z0-9_' -]+?)\s+(?:"((?:[^"\\]|\\.)*)"|EOF)$/;

  for (const line of lines) {
    const trimmed = line.trim();
    if (!trimmed) continue;

    const match = trimmed.match(tokenRegex);
    if (match) {
      const lineNum = parseInt(match[1], 10);
      const colNum = parseInt(match[2], 10);
      const type = match[3].trim();
      const lexeme = match[4] !== undefined ? match[4] : 'EOF';

      tokens.push({
        line: lineNum,
        column: colNum,
        type,
        lexeme,
      });
    }
  }

  return { tokens, success: tokens.length > 0 };
}

/**
 * Parses the 2-space indented Abstract Syntax Tree into a hierarchical tree.
 */
export function parseAst(stdout: string): { ast: AstNode | null; success: boolean } {
  const startIdx = stdout.indexOf('AST\n');
  if (startIdx === -1) {
    return { ast: null, success: false };
  }

  const sectionStart = startIdx + 'AST\n'.length;
  const delimiters = ['\nSymbol Table\n', '\n[PASS] IR', '\nIntermediate Code\n', '\nTarget Code\n'];
  let endIdx = stdout.length;

  for (const delim of delimiters) {
    const idx = stdout.indexOf(delim, sectionStart);
    if (idx !== -1 && idx < endIdx) {
      endIdx = idx;
    }
  }

  const astText = stdout.substring(sectionStart, endIdx);
  const lines = astText.split('\n');

  let root: AstNode | null = null;
  const stack: { node: AstNode; level: number }[] = [];

  for (const rawLine of lines) {
    if (!rawLine.trim()) continue;

    const leadingSpaces = rawLine.match(/^ */)?.[0].length ?? 0;
    const level = Math.floor(leadingSpaces / 2);
    const name = rawLine.trim();

    const newNode: AstNode = {
      name,
      level,
      children: [],
    };

    if (level === 0) {
      root = newNode;
      stack.length = 0;
      stack.push({ node: newNode, level: 0 });
    } else {
      while (stack.length > 0 && stack[stack.length - 1].level >= level) {
        stack.pop();
      }

      if (stack.length > 0) {
        stack[stack.length - 1].node.children.push(newNode);
      }
      stack.push({ node: newNode, level });
    }
  }

  return { ast: root, success: root !== null };
}

/**
 * Parses the scoped symbol table entries.
 */
export function parseSymbolTable(stdout: string): { symbols: SymbolEntry[]; success: boolean } {
  const symbols: SymbolEntry[] = [];
  const startIdx = stdout.indexOf('Symbol Table\n');
  if (startIdx === -1) {
    return { symbols, success: false };
  }

  const sectionStart = startIdx + 'Symbol Table\n'.length;
  const delimiters = ['\n[PASS] IR', '\nIntermediate Code\n', '\nTarget Code\n', '\nOutput\n'];
  let endIdx = stdout.length;

  for (const delim of delimiters) {
    const idx = stdout.indexOf(delim, sectionStart);
    if (idx !== -1 && idx < endIdx) {
      endIdx = idx;
    }
  }

  const tableText = stdout.substring(sectionStart, endIdx);
  const lines = tableText.split('\n');
  // Match row format: <source> <storage> <scope> <status> <type> <line>:<col>
  const rowRegex = /^(\S+)\s+(\S+)\s+(global|block \d+)\s+(active|out of scope)\s+(int|bool|error)\s+(\d+):(\d+)$/;

  for (const line of lines) {
    const trimmed = line.trim();
    if (!trimmed || trimmed.startsWith('Source Name')) continue;

    const match = trimmed.match(rowRegex);
    if (match) {
      symbols.push({
        sourceName: match[1],
        storageName: match[2],
        scope: match[3],
        status: match[4],
        type: match[5],
        line: parseInt(match[6], 10),
        column: parseInt(match[7], 10),
      });
    }
  }

  return { symbols, success: true };
}

/**
 * Parses Three-Address Code (TAC) instructions.
 */
export function parseTac(stdout: string): { tac: TacInstruction[]; success: boolean } {
  const tac: TacInstruction[] = [];
  const startIdx = stdout.indexOf('Intermediate Code\n');
  if (startIdx === -1) {
    return { tac, success: false };
  }

  const sectionStart = startIdx + 'Intermediate Code\n'.length;
  const delimiters = ['\n[PASS] Target', '\nTarget Code\n', '\nOutput\n'];
  let endIdx = stdout.length;

  for (const delim of delimiters) {
    const idx = stdout.indexOf(delim, sectionStart);
    if (idx !== -1 && idx < endIdx) {
      endIdx = idx;
    }
  }

  const tacText = stdout.substring(sectionStart, endIdx);
  const lines = tacText.split('\n');
  const lineRegex = /^(\d+):\s+(?:label\s+(\w+)|(\S+)(?:\s+(.*))?)$/;

  for (const line of lines) {
    const trimmed = line.trim();
    if (!trimmed) continue;

    const match = trimmed.match(lineRegex);
    if (match) {
      const index = parseInt(match[1], 10);
      if (match[2]) {
        // Label instruction (e.g. "13: label L0")
        tac.push({
          index,
          raw: trimmed,
          isLabel: true,
          label: match[2],
          op: 'label',
          args: [match[2]],
        });
      } else {
        // Regular TAC quad (e.g. "0: load_const t0, 1")
        const op = match[3];
        const rawArgs = match[4] ?? '';
        const args = rawArgs
          ? rawArgs.split(',').map((a) => a.trim()).filter(Boolean)
          : [];

        tac.push({
          index,
          raw: trimmed,
          isLabel: false,
          op,
          args,
        });
      }
    }
  }

  return { tac, success: tac.length > 0 };
}

/**
 * Parses optimization metrics and constant folding statistics.
 */
export function parseOptimization(stdout: string): OptimizationInfo | null {
  // Check summary line in "all" mode
  // e.g.: [PASS] IR generation and optimization (17 instructions, 1 constant(s) folded)
  // or:   [PASS] IR generation and optimization (40 instructions)
  const summaryMatch = stdout.match(
    /\[PASS\] IR generation and optimization \((\d+) instructions(?:, (\d+) constant\(s\) folded)?\)/
  );

  let instructionsAfter: number | undefined;
  let constantsFolded = 0;

  if (summaryMatch) {
    instructionsAfter = parseInt(summaryMatch[1], 10);
    constantsFolded = summaryMatch[2] ? parseInt(summaryMatch[2], 10) : 0;
  }

  // Check detailed "bench" mode metrics if available
  const benchAfterMatch = stdout.match(/Optimized TAC Instructions:\s+(\d+)/);
  const benchBeforeMatch = stdout.match(/Unoptimized TAC Instructions:\s+(\d+)/);
  const benchFoldedMatch = stdout.match(/Constant Expressions Folded:\s+(\d+)/);
  const benchRateMatch = stdout.match(/Optimization Reduction Rate:\s+([0-9.]+%)/);

  let instructionsBefore: number | undefined;
  let reductionRate: string | undefined;

  if (benchAfterMatch) instructionsAfter = parseInt(benchAfterMatch[1], 10);
  if (benchBeforeMatch) instructionsBefore = parseInt(benchBeforeMatch[1], 10);
  if (benchFoldedMatch) constantsFolded = parseInt(benchFoldedMatch[1], 10);
  if (benchRateMatch) reductionRate = benchRateMatch[1];

  if (instructionsAfter === undefined && constantsFolded === 0) {
    return null;
  }

  const message = constantsFolded > 0
    ? `${constantsFolded} constant expression(s) folded into immediate values at compile time.`
    : 'No constant expressions folded in this compilation pass.';

  return {
    instructionsBefore,
    instructionsAfter,
    constantsFolded,
    reductionRate,
    message,
  };
}

/**
 * Parses target pseudo-assembly instructions and labels.
 */
export function parseTargetCode(stdout: string): { targetCode: AsmInstruction[]; success: boolean } {
  const targetCode: AsmInstruction[] = [];
  const startIdx = stdout.indexOf('Target Code\n');
  if (startIdx === -1) {
    return { targetCode, success: false };
  }

  const sectionStart = startIdx + 'Target Code\n'.length;
  const delimiters = ['\nOutput\n', '\n[PASS] Program execution'];
  let endIdx = stdout.length;

  for (const delim of delimiters) {
    const idx = stdout.indexOf(delim, sectionStart);
    if (idx !== -1 && idx < endIdx) {
      endIdx = idx;
    }
  }

  const asmText = stdout.substring(sectionStart, endIdx);
  const lines = asmText.split('\n');

  for (const line of lines) {
    const trimmed = line.trim();
    if (!trimmed) continue;

    if (trimmed.startsWith(';')) {
      targetCode.push({
        raw: trimmed,
        isComment: true,
        isLabel: false,
      });
    } else if (trimmed.endsWith(':')) {
      targetCode.push({
        raw: trimmed,
        isComment: false,
        isLabel: true,
        label: trimmed.slice(0, -1).trim(),
      });
    } else {
      const spaceIdx = trimmed.indexOf(' ');
      const mnemonic = spaceIdx === -1 ? trimmed : trimmed.substring(0, spaceIdx);
      const operands = spaceIdx === -1 ? '' : trimmed.substring(spaceIdx + 1).trim();

      targetCode.push({
        raw: trimmed,
        isComment: false,
        isLabel: false,
        mnemonic,
        operands,
      });
    }
  }

  return { targetCode, success: targetCode.length > 0 };
}

/**
 * Extracts runtime program output (stdout from print(...) statements).
 */
export function parseRuntimeOutput(stdout: string): string {
  const startIdx = stdout.indexOf('Output\n');
  if (startIdx === -1) {
    return '';
  }

  const sectionStart = startIdx + 'Output\n'.length;
  const endIdx = stdout.indexOf('\n[PASS] Program execution', sectionStart);

  if (endIdx !== -1) {
    return stdout.substring(sectionStart, endIdx).trim();
  }
  return stdout.substring(sectionStart).trim();
}

/**
 * Parses compiler diagnostics, error carets, and educational hints from stderr/stdout.
 */
export function parseDiagnostics(stderr: string, stdout: string): DiagnosticItem[] {
  const diagnostics: DiagnosticItem[] = [];
  const combined = `${stderr}\n${stdout}`;
  const lines = combined.split('\n');

  // Match file:line:col: (error|warning): message
  const diagHeaderRegex = /^(?:.*[/\\])?([^/\\]+\.edu):(\d+):(\d+):\s+(error|warning):\s+(.+)$/;
  const sourceLineRegex = /^\s*\|\s+(.*)$/;
  const hintRegex = /^\s*=\s*hint:\s*(.+)$/;

  let currentDiag: DiagnosticItem | null = null;

  for (let i = 0; i < lines.length; ++i) {
    const line = lines[i];
    const headerMatch = line.match(diagHeaderRegex);

    if (headerMatch) {
      if (currentDiag) {
        diagnostics.push(currentDiag);
      }

      currentDiag = {
        file: sanitizePath(headerMatch[1]),
        line: parseInt(headerMatch[2], 10),
        column: parseInt(headerMatch[3], 10),
        severity: headerMatch[4] as 'error' | 'warning',
        message: sanitizePath(headerMatch[5]),
      };
      continue;
    }

    if (currentDiag) {
      const sourceMatch = line.match(sourceLineRegex);
      if (sourceMatch && !currentDiag.sourceLine) {
        currentDiag.sourceLine = sourceMatch[1];
        // Peek at next line for caret pointer
        if (i + 1 < lines.length) {
          const nextLine = lines[i + 1];
          const caretMatch = nextLine.match(/^\s*\|\s+(\^+.*)$/);
          if (caretMatch) {
            currentDiag.caret = caretMatch[1];
            i++; // consume caret line
          }
        }
        continue;
      }

      const hintMatch = line.match(hintRegex);
      if (hintMatch) {
        currentDiag.hint = hintMatch[1].trim();
        diagnostics.push(currentDiag);
        currentDiag = null;
      }
    }
  }

  if (currentDiag) {
    diagnostics.push(currentDiag);
  }

  return diagnostics;
}

/**
 * Master parser: coordinates parsing across all pipeline stages.
 */
export function parseCompilerOutput(
  stdout: string,
  stderr: string,
  exitCode: number | null
): ParsedStages {
  const warnings: string[] = [];

  const pipelineStatuses = parsePipelineStatuses(stdout);
  const { tokens, success: tokensParsed } = parseTokens(stdout);
  const { ast, success: astParsed } = parseAst(stdout);
  const { symbols, success: symbolsParsed } = parseSymbolTable(stdout);
  const { tac, success: tacParsed } = parseTac(stdout);
  const optimization = parseOptimization(stdout);
  const { targetCode, success: targetParsed } = parseTargetCode(stdout);
  const runtimeOutput = parseRuntimeOutput(stdout);
  const diagnostics = parseDiagnostics(stderr, stdout);

  if (exitCode === 0) {
    if (!tokensParsed) warnings.push('Tokens section not detected in stdout.');
    if (!astParsed) warnings.push('AST section not detected in stdout.');
    if (!symbolsParsed) warnings.push('Symbol table not detected in stdout.');
    if (!tacParsed) warnings.push('TAC intermediate code not detected in stdout.');
    if (!targetParsed) warnings.push('Target code not detected in stdout.');
  }

  return {
    pipelineStatuses,
    tokens,
    ast,
    symbols,
    tac,
    optimization,
    targetCode,
    runtimeOutput,
    diagnostics,
    parsingStatus: {
      tokensParsed,
      astParsed,
      symbolsParsed,
      tacParsed,
      optParsed: optimization !== null,
      targetParsed,
      diagnosticsParsed: diagnostics.length > 0,
      runtimeOutputCaptured: runtimeOutput.length > 0,
      warnings,
    },
  };
}
