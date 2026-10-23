import React, { useState } from 'react';
import {
  Terminal,
  Play,
  CheckCircle2,
  AlertTriangle,
  Clock,
  Copy,
  Check,
  ServerOff,
} from 'lucide-react';
import { CompileResponse, CompileStatus } from '../../types';

interface OutputViewProps {
  response: CompileResponse | null;
  status: CompileStatus;
  errorMessage: string | null;
}

export const OutputView: React.FC<OutputViewProps> = ({
  response,
  status,
  errorMessage,
}) => {
  const [copied, setCopied] = useState(false);

  // Backend offline state
  if (status === 'offline') {
    return (
      <div className="offline-banner">
        <ServerOff size={20} />
        <div>
          <strong>Backend service unavailable on port 5001.</strong>
          <div>
            Ensure the server is running by executing: <code>npm run server</code> inside the <code>web/</code> directory.
          </div>
        </div>
      </div>
    );
  }

  // Timeout state
  if (status === 'timeout' || response?.timedOut) {
    return (
      <div className="caret-diagnostic" style={{ margin: '1rem', borderColor: '#f59e0b', background: 'rgba(245, 158, 11, 0.1)' }}>
        <div style={{ display: 'flex', alignItems: 'center', gap: '0.5rem', color: '#fbbf24', fontWeight: 600, marginBottom: '0.5rem' }}>
          <Clock size={16} />
          <span>Execution Timed Out (2000ms Limit Exceeded)</span>
        </div>
        <div style={{ color: '#e2e8f0', fontSize: '0.85rem', lineHeight: 1.6 }}>
          The C++ compiler subprocess was terminated by the process runner because it exceeded the strict 2-second timeout window.
          This occurs when the EduLang program executes an unbounded loop (e.g. <code>while (condition)</code>).
        </div>
      </div>
    );
  }

  // General server error message
  if (errorMessage && !response) {
    return (
      <div className="caret-diagnostic" style={{ margin: '1rem' }}>
        <div style={{ display: 'flex', alignItems: 'center', gap: '0.5rem', color: '#fb7185', fontWeight: 600, marginBottom: '0.5rem' }}>
          <AlertTriangle size={16} />
          <span>Server Communication Error</span>
        </div>
        <div>{errorMessage}</div>
      </div>
    );
  }

  // Idle state
  if (status === 'idle' && !response) {
    return (
      <div className="stage-placeholder-card">
        <div className="stage-icon">
          <Terminal size={22} />
        </div>
        <div className="stage-placeholder-title">Ready for Compilation</div>
        <div className="stage-placeholder-desc">
          Click <strong>Compile</strong> (or press <strong>⌘ + Enter</strong>) to run the source code through the real C++20 compiler.
        </div>
      </div>
    );
  }

  const handleCopyAll = () => {
    const full = `--- STDOUT ---\n${response?.stdout ?? ''}\n\n--- STDERR ---\n${response?.stderr ?? ''}`;
    navigator.clipboard.writeText(full);
    setCopied(true);
    setTimeout(() => setCopied(false), 2000);
  };

  const hasRuntimeOutput = Boolean(response?.stages?.runtimeOutput);
  const hasStderr = Boolean(response?.stderr);
  const hasStdout = Boolean(response?.stdout);

  return (
    <div className="output-console-view">
      {/* 1. Header with Status & Action */}
      <div className="visualizer-header" style={{ marginBottom: '1rem' }}>
        <div className="visualizer-stats">
          {response?.success && response?.exitCode === 0 && (
            <span className="stat-pill success">
              <CheckCircle2 size={13} />
              <span>Compilation & Execution Succeeded (Exit Code 0)</span>
            </span>
          )}
          {response?.exitCode !== null && response?.exitCode !== 0 && (
            <span className="stat-pill failure">
              <AlertTriangle size={13} />
              <span>Exit Code {response?.exitCode} (Diagnostics Reported)</span>
            </span>
          )}
        </div>

        <button className="action-btn" onClick={handleCopyAll} title="Copy raw stdout & stderr">
          {copied ? <Check size={13} className="text-emerald-400" /> : <Copy size={13} />}
          <span>{copied ? 'Copied' : 'Copy Raw Output'}</span>
        </button>
      </div>

      {/* 2. Runtime Execution Output Box (If program printed output) */}
      {hasRuntimeOutput && (
        <div className="runtime-output-card">
          <div className="runtime-output-header">
            <Play size={14} className="text-emerald-400" />
            <span>Virtual Machine Program Output</span>
          </div>
          <pre className="runtime-output-content">{response?.stages?.runtimeOutput}</pre>
        </div>
      )}

      {/* 3. Compiler Diagnostics (stderr) */}
      {hasStderr && (
        <div className="stderr-card">
          <div className="stderr-header">
            <AlertTriangle size={14} className="text-rose-400" />
            <span>Compiler Diagnostics (stderr)</span>
          </div>
          <pre className="stderr-content">{response?.stderr}</pre>
        </div>
      )}

      {/* 4. Complete Terminal Stdout */}
      {hasStdout && (
        <div className="stdout-card">
          <div className="stdout-header">
            <Terminal size={14} className="text-slate-400" />
            <span>Complete Compiler Terminal Output (stdout)</span>
          </div>
          <pre className="stdout-content">{response?.stdout}</pre>
        </div>
      )}
    </div>
  );
};
