import React from 'react';
import {
  CheckCircle2,
  AlertOctagon,
  HelpCircle,
  Activity,
  ShieldCheck,
  AlertTriangle,
} from 'lucide-react';
import { DiagnosticItem, PipelineStatusItem } from '../../types';

interface SemanticViewProps {
  diagnostics: DiagnosticItem[];
  pipelineStatuses: PipelineStatusItem[];
  warnings?: string[];
}

export const SemanticView: React.FC<SemanticViewProps> = ({
  diagnostics,
  pipelineStatuses,
  warnings = [],
}) => {
  const hasErrors = diagnostics && diagnostics.length > 0;

  const getStatusBadge = (status: 'PASS' | 'FAIL' | 'SKIP') => {
    switch (status) {
      case 'PASS':
        return <span className="status-badge-pass">PASS</span>;
      case 'FAIL':
        return <span className="status-badge-fail">FAIL</span>;
      case 'SKIP':
        return <span className="status-badge-skip">SKIP</span>;
    }
  };

  return (
    <div className="visualizer-container">
      {/* 1. Pipeline Stage Verification Status */}
      <div className="section-card">
        <div className="section-card-title">
          <Activity size={16} />
          <span>Compiler Pipeline Stage Verification</span>
        </div>
        <div className="pipeline-grid">
          {pipelineStatuses.map((p, idx) => (
            <div key={idx} className={`pipeline-item status-${p.status.toLowerCase()}`}>
              <div className="pipeline-item-header">
                <span className="pipeline-item-name">{p.stage}</span>
                {getStatusBadge(p.status)}
              </div>
              {p.details && (
                <div className="pipeline-item-details">{p.details}</div>
              )}
            </div>
          ))}
          {pipelineStatuses.length === 0 && (
            <div className="text-muted" style={{ fontSize: '0.82rem', padding: '0.5rem' }}>
              No pipeline stage status was reported by the compiler.
            </div>
          )}
        </div>
      </div>

      {/* 2. Parser Warnings (if any) */}
      {warnings.length > 0 && (
        <div className="warning-banner">
          <AlertTriangle size={16} className="text-amber-400" />
          <div>
            <strong>Parser Notice:</strong>
            <ul style={{ marginLeft: '1.25rem', marginTop: '0.25rem' }}>
              {warnings.map((w, idx) => (
                <li key={idx}>{w}</li>
              ))}
            </ul>
          </div>
        </div>
      )}

      {/* 3. Diagnostic Errors / Warnings */}
      {hasErrors ? (
        <div className="diagnostics-container">
          <div className="diagnostics-header">
            <AlertOctagon size={18} className="text-rose-400" />
            <span>Diagnostics Detected ({diagnostics.length})</span>
          </div>

          <div className="diagnostics-list">
            {diagnostics.map((diag, idx) => (
              <div key={idx} className="diagnostic-card">
                <div className="diag-title-row">
                  <span className={`diag-severity ${diag.severity}`}>
                    {diag.severity.toUpperCase()}
                  </span>
                  <span className="diag-location">
                    {diag.file}:{diag.line}:{diag.column}
                  </span>
                  <span className="diag-message">{diag.message}</span>
                </div>

                {diag.sourceLine && (
                  <div className="diag-code-box">
                    <div className="diag-code-line">
                      <span className="diag-line-num">{diag.line} |</span>
                      <span className="diag-code-text">{diag.sourceLine}</span>
                    </div>
                    {diag.caret && (
                      <div className="diag-caret-line">
                        <span className="diag-line-num">  |</span>
                        <span className="diag-caret-text">{diag.caret}</span>
                      </div>
                    )}
                  </div>
                )}

                {diag.hint && (
                  <div className="diag-hint-row">
                    <HelpCircle size={15} />
                    <span>
                      <strong>Educational Hint:</strong> {diag.hint}
                    </span>
                  </div>
                )}
              </div>
            ))}
          </div>
        </div>
      ) : (
        <div className="semantic-success-card">
          <div className="success-icon-wrapper">
            <ShieldCheck size={28} />
          </div>
          <div className="success-title">Semantic Validation Succeeded</div>
          <div className="success-desc">
            All variables are declared before use with compatible types (<code>int</code> and <code>bool</code>).
            Lexical block scoping and variable shadowing rules have been strictly verified with zero diagnostic errors.
          </div>
        </div>
      )}
    </div>
  );
};
