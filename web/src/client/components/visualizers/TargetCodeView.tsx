import React, { useState } from 'react';
import { Terminal, Copy, Check, Hash } from 'lucide-react';
import { AsmInstruction } from '../../types';

interface TargetCodeViewProps {
  targetCode: AsmInstruction[];
}

export const TargetCodeView: React.FC<TargetCodeViewProps> = ({ targetCode }) => {
  const [copied, setCopied] = useState(false);

  if (!targetCode || targetCode.length === 0) {
    return (
      <div className="stage-placeholder-card">
        <div className="stage-icon">
          <Terminal size={22} />
        </div>
        <div className="stage-placeholder-title">No Target Code Generated</div>
        <div className="stage-placeholder-desc">
          Pseudo-assembly generation was not reached. Ensure previous stages (lexing, parsing, semantic checking, IR generation) succeed without errors.
        </div>
      </div>
    );
  }

  const handleCopy = () => {
    const rawText = targetCode.map((t) => t.raw).join('\n');
    navigator.clipboard.writeText(rawText);
    setCopied(true);
    setTimeout(() => setCopied(false), 2000);
  };

  const instructionCount = targetCode.filter((t) => !t.isComment && !t.isLabel).length;
  const labelCount = targetCode.filter((t) => t.isLabel).length;

  return (
    <div className="visualizer-container">
      <div className="visualizer-header">
        <div className="visualizer-stats">
          <span className="stat-pill">
            <Hash size={13} />
            <span>{instructionCount} Target Instructions</span>
          </span>
          <span className="stat-pill muted">
            <span>{labelCount} Assembly Labels</span>
          </span>
        </div>

        <div className="visualizer-actions">
          <button className="action-btn" onClick={handleCopy} title="Copy pseudo-assembly">
            {copied ? <Check size={13} className="text-emerald-400" /> : <Copy size={13} />}
            <span>{copied ? 'Copied' : 'Copy Assembly'}</span>
          </button>
        </div>
      </div>

      <div className="code-listing-container">
        {targetCode.map((line, idx) => {
          if (line.isComment) {
            return (
              <div key={idx} className="asm-line asm-comment">
                <span className="asm-num">{idx + 1}</span>
                <span className="asm-text">{line.raw}</span>
              </div>
            );
          }

          if (line.isLabel) {
            return (
              <div key={idx} className="asm-line asm-label-line">
                <span className="asm-num">{idx + 1}</span>
                <span className="asm-label-badge">{line.label}:</span>
              </div>
            );
          }

          return (
            <div key={idx} className="asm-line">
              <span className="asm-num">{idx + 1}</span>
              <span className="asm-mnemonic">{line.mnemonic}</span>
              <span className="asm-operands">{line.operands}</span>
            </div>
          );
        })}
      </div>
    </div>
  );
};
