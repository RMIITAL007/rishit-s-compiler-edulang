export type CompilerMode =
  | 'all'
  | 'tokens'
  | 'ast'
  | 'symbols'
  | 'ir'
  | 'run'
  | 'target'
  | 'bench';

import { ParsedStages } from '../parser';

export * from '../parser';

export interface CompileResponse {
  success: boolean;
  exitCode: number | null;
  mode: string;
  stdout: string;
  stderr: string;
  timedOut: boolean;
  error: string | null;
  stages?: ParsedStages;
}


export interface SampleProgram {
  id: string;
  title: string;
  category: 'Standard Programs' | 'Diagnostic Errors' | 'Stress & Robustness';
  description: string;
  code: string;
}

export type CompileStatus =
  | 'idle'
  | 'compiling'
  | 'success'
  | 'failure'
  | 'timeout'
  | 'offline';

export type StageTab =
  | 'output'
  | 'tokens'
  | 'ast'
  | 'symbols'
  | 'ir'
  | 'opt'
  | 'target';
