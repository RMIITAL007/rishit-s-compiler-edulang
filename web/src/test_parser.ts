import assert from 'node:assert';
import { execFileSync } from 'node:child_process';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { parseCompilerOutput } from './parser.js';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);
const PROJECT_ROOT = path.resolve(__dirname, '..', '..');
const COMPILER = path.resolve(PROJECT_ROOT, 'build', 'compiler');

function runCLI(args: string[]): { stdout: string; stderr: string; exitCode: number } {
  try {
    const stdout = execFileSync(COMPILER, args, { encoding: 'utf-8', stdio: ['pipe', 'pipe', 'pipe'] });
    return { stdout, stderr: '', exitCode: 0 };
  } catch (err: any) {
    return {
      stdout: err.stdout?.toString() ?? '',
      stderr: err.stderr?.toString() ?? '',
      exitCode: err.status ?? 1,
    };
  }
}

console.log('--- Running EduLang Output Parser Test Suite ---');

// Test 1: arithmetic_control.edu (Valid program with constant folding)
{
  const filePath = path.join(PROJECT_ROOT, 'examples', 'arithmetic_control.edu');
  const { stdout, stderr, exitCode } = runCLI([filePath]);
  assert.strictEqual(exitCode, 0, 'arithmetic_control should exit with 0');

  const stages = parseCompilerOutput(stdout, stderr, exitCode);

  // Verify Tokens
  assert.ok(stages.parsingStatus.tokensParsed, 'Tokens should be parsed');
  assert.strictEqual(stages.tokens[0].type, "'int'");
  assert.strictEqual(stages.tokens[0].lexeme, 'int');
  assert.strictEqual(stages.tokens[stages.tokens.length - 1].type, 'end of file');

  // Verify AST
  assert.ok(stages.parsingStatus.astParsed, 'AST should be parsed');
  assert.ok(stages.ast, 'AST root should not be null');
  assert.strictEqual(stages.ast!.name, 'Program');
  assert.strictEqual(stages.ast!.children.length, 2); // Declaration(a) and If

  // Verify Symbols
  assert.ok(stages.parsingStatus.symbolsParsed, 'Symbols should be parsed');
  assert.strictEqual(stages.symbols.length, 1);
  assert.strictEqual(stages.symbols[0].sourceName, 'a');
  assert.strictEqual(stages.symbols[0].storageName, 'a');
  assert.strictEqual(stages.symbols[0].scope, 'global');
  assert.strictEqual(stages.symbols[0].status, 'active');
  assert.strictEqual(stages.symbols[0].type, 'int');

  // Verify TAC
  assert.ok(stages.parsingStatus.tacParsed, 'TAC should be parsed');
  assert.strictEqual(stages.tac[0].op, 'load_const');
  assert.strictEqual(stages.tac[0].args[0], 't0');
  assert.strictEqual(stages.tac[0].args[1], '1');

  // Verify Optimization
  assert.ok(stages.optimization, 'Optimization should be detected');
  assert.strictEqual(stages.optimization!.constantsFolded, 1);
  assert.strictEqual(stages.optimization!.instructionsAfter, 17);

  // Verify Pseudo-Assembly
  assert.ok(stages.parsingStatus.targetParsed, 'Target code should be parsed');
  assert.ok(stages.targetCode.some((i) => i.mnemonic === 'MOV'));
  assert.ok(stages.targetCode.some((i) => i.isLabel && i.label === 'L0'));

  // Verify Runtime Output
  assert.strictEqual(stages.runtimeOutput, '6', 'Output of arithmetic_control should be 6');

  console.log('✓ Test 1 Passed: arithmetic_control.edu parsed with 100% fidelity');
}

// Test 2: nested_loops.edu (Block scopes, nested while loops, multi-line output)
{
  const filePath = path.join(PROJECT_ROOT, 'examples', 'nested_loops.edu');
  const { stdout, stderr, exitCode } = runCLI([filePath]);
  assert.strictEqual(exitCode, 0);

  const stages = parseCompilerOutput(stdout, stderr, exitCode);

  assert.ok(stages.symbols.length >= 2, 'Should have multiple scoped symbols');
  const innerSymbol = stages.symbols.find((s) => s.sourceName === 'inner');
  assert.ok(innerSymbol, "Symbol 'inner' should exist");
  assert.strictEqual(innerSymbol!.scope, 'block 1');
  assert.strictEqual(innerSymbol!.status, 'out of scope');

  // Multi-line runtime output check
  const lines = stages.runtimeOutput.split('\n').map((l) => l.trim());
  assert.deepStrictEqual(lines, ['0', '1', '10', '11']);

  console.log('✓ Test 2 Passed: nested_loops.edu block scopes and multi-line output parsed');
}

// Test 3: nested_boolean.edu (Short-circuiting, unary & binary AST nodes)
{
  const filePath = path.join(PROJECT_ROOT, 'examples', 'nested_boolean.edu');
  const { stdout, stderr, exitCode } = runCLI([filePath]);
  assert.strictEqual(exitCode, 0);

  const stages = parseCompilerOutput(stdout, stderr, exitCode);
  assert.strictEqual(stages.runtimeOutput, '10');
  assert.ok(stages.tac.length >= 30, 'Should contain lowered short-circuit jumps');

  console.log('✓ Test 3 Passed: nested_boolean.edu parsed successfully');
}

// Test 4: syntax_error.edu (Syntax diagnostic with sanitized path, caret, and hint)
{
  const filePath = path.join(PROJECT_ROOT, 'examples', 'errors', 'syntax_error.edu');
  const { stdout, stderr, exitCode } = runCLI([filePath]);
  assert.strictEqual(exitCode, 1);

  const stages = parseCompilerOutput(stdout, stderr, exitCode);

  assert.ok(stages.diagnostics.length > 0, 'Diagnostic should be extracted');
  const diag = stages.diagnostics[0];
  assert.strictEqual(diag.severity, 'error');
  assert.strictEqual(diag.line, 1);
  assert.strictEqual(diag.column, 13);
  assert.ok(diag.message.includes('expected a number'));
  assert.ok(diag.hint?.includes('provide a valid expression value'));
  assert.ok(diag.caret?.includes('^'));
  assert.strictEqual(diag.file, 'syntax_error.edu');

  // Pipeline status should report Syntax analysis FAIL
  const syntaxStatus = stages.pipelineStatuses.find((p) => p.stage === 'Syntax analysis');
  assert.ok(syntaxStatus);
  assert.strictEqual(syntaxStatus!.status, 'FAIL');

  console.log('✓ Test 4 Passed: syntax_error.edu diagnostic caret and hint extracted');
}

// Test 5: semantic_error.edu (Undeclared variable)
{
  const filePath = path.join(PROJECT_ROOT, 'examples', 'errors', 'semantic_error.edu');
  const { stdout, stderr, exitCode } = runCLI([filePath]);
  assert.strictEqual(exitCode, 1);

  const stages = parseCompilerOutput(stdout, stderr, exitCode);
  const diag = stages.diagnostics[0];
  assert.ok(diag.message.includes("undeclared variable 'missing_value'"));
  assert.ok(diag.hint?.includes("declare 'missing_value' first"));

  console.log('✓ Test 5 Passed: semantic_error.edu undeclared variable parsed');
}

// Test 6: runtime_error.edu (Division by zero)
{
  const filePath = path.join(PROJECT_ROOT, 'examples', 'errors', 'runtime_error.edu');
  const { stdout, stderr, exitCode } = runCLI([filePath]);
  assert.strictEqual(exitCode, 1);

  const stages = parseCompilerOutput(stdout, stderr, exitCode);
  const diag = stages.diagnostics[0];
  assert.ok(diag.message.includes('division by zero'));

  console.log('✓ Test 6 Passed: runtime_error.edu division by zero parsed');
}

// Test 7: Empty and malformed input robustness
{
  const stages = parseCompilerOutput('', '', null);
  assert.strictEqual(stages.tokens.length, 0);
  assert.strictEqual(stages.ast, null);
  assert.strictEqual(stages.symbols.length, 0);
  assert.strictEqual(stages.tac.length, 0);
  assert.strictEqual(stages.targetCode.length, 0);
  assert.strictEqual(stages.diagnostics.length, 0);
  assert.strictEqual(stages.runtimeOutput, '');

  console.log('✓ Test 7 Passed: Empty input handled safely with 0 errors');
}

console.log('--- ALL 7 PARSER TESTS PASSED SUCCESSFULLY ---');
