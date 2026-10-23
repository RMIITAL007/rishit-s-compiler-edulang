import express, { Request, Response } from 'express';
import cors from 'cors';
import { spawn } from 'node:child_process';
import fs from 'node:fs';
import path from 'node:path';
import os from 'node:os';
import crypto from 'node:crypto';
import { fileURLToPath } from 'node:url';
import { parseCompilerOutput, ParsedStages } from './parser.js';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

// Resolve repository root and compiler binary path safely
const PROJECT_ROOT = path.resolve(__dirname, '..', '..');
const COMPILER_PATH = path.resolve(PROJECT_ROOT, 'build', 'compiler');

const ALLOWED_MODES = [
  'all',
  'tokens',
  'ast',
  'symbols',
  'ir',
  'run',
  'target',
  'bench',
] as const;

type CompilerMode = (typeof ALLOWED_MODES)[number];

const MAX_SOURCE_SIZE = 64 * 1024; // 64 KB limit
const COMPILER_TIMEOUT_MS = 2000;   // 2-second strict execution timeout

export interface CompileResult {
  success: boolean;
  exitCode: number | null;
  mode: string;
  stdout: string;
  stderr: string;
  timedOut: boolean;
  error: string | null;
  stages?: ParsedStages;
}


export function runCompiler(
  source: string,
  mode: CompilerMode = 'all',
  timeoutMs = COMPILER_TIMEOUT_MS
): Promise<CompileResult> {
  return new Promise(async (resolve) => {
    // 1. Verify compiler executable exists
    if (!fs.existsSync(COMPILER_PATH)) {
      resolve({
        success: false,
        exitCode: null,
        mode,
        stdout: '',
        stderr: '',
        timedOut: false,
        error: `Compiler binary not found at '${COMPILER_PATH}'. Please build it using 'cmake --build build' first.`,
      });
      return;
    }

    // 2. Create unique temporary .edu file
    const uniqueId = `${Date.now()}_${crypto.randomBytes(6).toString('hex')}`;
    const tempFilePath = path.join(os.tmpdir(), `edulang_${uniqueId}.edu`);

    try {
      await fs.promises.writeFile(tempFilePath, source, 'utf-8');
    } catch (err) {
      resolve({
        success: false,
        exitCode: null,
        mode,
        stdout: '',
        stderr: '',
        timedOut: false,
        error: `Failed to create temporary source file: ${(err as Error).message}`,
      });
      return;
    }

    // 3. Prepare arguments (direct array without shell invocation to prevent command injection)
    const args: string[] = mode === 'all' ? [tempFilePath] : [mode, tempFilePath];

    let stdout = '';
    let stderr = '';
    let timedOut = false;
    let finished = false;

    // 4. Launch subprocess
    const proc = spawn(COMPILER_PATH, args, {
      shell: false,
      windowsHide: true,
    });

    const timer = setTimeout(() => {
      if (!finished) {
        timedOut = true;
        proc.kill('SIGKILL');
      }
    }, timeoutMs);

    proc.stdout.on('data', (chunk) => {
      stdout += chunk.toString();
    });

    proc.stderr.on('data', (chunk) => {
      stderr += chunk.toString();
    });

    const cleanupTempFile = async () => {
      try {
        await fs.promises.unlink(tempFilePath);
      } catch {
        // Silently ignore if file was already unlinked
      }
    };

    proc.on('error', async (err) => {
      if (finished) return;
      finished = true;
      clearTimeout(timer);
      await cleanupTempFile();

      resolve({
        success: false,
        exitCode: null,
        mode,
        stdout,
        stderr: stderr || err.message,
        timedOut,
        error: `Failed to start compiler subprocess: ${err.message}`,
      });
    });

    proc.on('close', async (code) => {
      if (finished) return;
      finished = true;
      clearTimeout(timer);
      await cleanupTempFile();

      if (timedOut) {
        resolve({
          success: false,
          exitCode: null,
          mode,
          stdout,
          stderr,
          timedOut: true,
          error: `Execution timed out after ${timeoutMs}ms`,
        });
        return;
      }

      resolve({
        success: code === 0,
        exitCode: code,
        mode,
        stdout,
        stderr,
        timedOut: false,
        error: null,
      });
    });
  });
}

// Create Express application
export const app = express();

app.use(cors());
app.use(express.json({ limit: '128kb' }));

// Health & status endpoint
app.get('/api/health', (_req: Request, res: Response) => {
  const binaryExists = fs.existsSync(COMPILER_PATH);
  res.json({
    status: 'ok',
    compilerReady: binaryExists,
    compilerPath: COMPILER_PATH,
    allowedModes: ALLOWED_MODES,
    timeoutMs: COMPILER_TIMEOUT_MS,
  });
});

// Compilation endpoint
app.post('/api/compile', async (req: Request, res: Response) => {
  const { source, mode = 'all' } = req.body ?? {};

  // Validate source
  if (typeof source !== 'string') {
    res.status(400).json({
      success: false,
      error: "The 'source' field is required and must be a string.",
    });
    return;
  }

  if (Buffer.byteLength(source, 'utf-8') > MAX_SOURCE_SIZE) {
    res.status(400).json({
      success: false,
      error: `Source code exceeds maximum allowed size of ${MAX_SOURCE_SIZE / 1024} KB.`,
    });
    return;
  }

  // Validate mode
  if (typeof mode !== 'string' || !ALLOWED_MODES.includes(mode as CompilerMode)) {
    res.status(400).json({
      success: false,
      error: `Invalid mode '${mode}'. Allowed modes are: ${ALLOWED_MODES.join(', ')}`,
    });
    return;
  }

  // Verify compiler binary existence
  if (!fs.existsSync(COMPILER_PATH)) {
    res.status(503).json({
      success: false,
      error: `Compiler executable not found at '${COMPILER_PATH}'. Please compile the C++ project with 'cmake --build build' first.`,
    });
    return;
  }

  try {
    const result = await runCompiler(source, mode as CompilerMode);
    const stages = parseCompilerOutput(result.stdout, result.stderr, result.exitCode);
    res.status(200).json({
      ...result,
      stages,
    });
  } catch (err) {

    res.status(500).json({
      success: false,
      error: `Unexpected server error: ${(err as Error).message}`,
    });
  }
});

const PORT = process.env.PORT ? parseInt(process.env.PORT, 10) : 5001;

if (process.env.NODE_ENV !== 'test') {
  app.listen(PORT, () => {
    console.log(`EduLang Compiler Backend running at http://localhost:${PORT}`);
    console.log(`Compiler binary: ${COMPILER_PATH}`);
  });
}
