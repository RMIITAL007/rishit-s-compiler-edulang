import React, { useState } from 'react';
import { Cpu, Copy, Check, Filter } from 'lucide-react';
import { TacInstruction } from '../../types';

interface TacViewProps {
  tac: TacInstruction[];
}

export const TacView: React.FC<TacViewProps> = ({ tac }) => {
  const [copied, setCopied] = useState(false);
  const [showLabelsOnly, setShowLabelsOnly] = useState(false);

  if (!tac || tac.length === 0) {
    return (
      <div className="stage-placeholder-card">
        <div className="stage-icon">
          <Cpu size={22} />
        </div>
        <div className="stage-placeholder-title">No Intermediate Code (TAC)</div>
        <div className="stage-placeholder-desc">
          Three-Address Code generation was not reached. Ensure lexical, syntactic, and semantic stages pass without diagnostic errors.
        </div>
      </div>
    );
  }

  const handleCopy = () => {
    const rawText = tac.map((t) => t.raw).join('\n');
    navigator.clipboard.writeText(rawText);
    setCopied(true);
    setTimeout(() => setCopied(false), 2000);
  };

  const displayedTac = showLabelsOnly ? tac.filter((t) => t.isLabel) : tac;

  return (
    <div className="visualizer-container">
      <div className="visualizer-header">
        <div className="visualizer-stats">
          <span className="stat-pill">
            <Cpu size={13} />
            <span>{tac.length} Linearized Instructions</span>
          </span>
          <span className="stat-pill muted">
            <span>{tac.filter((t) => t.isLabel).length} Jump Labels</span>
          </span>
        </div>

        <div className="visualizer-actions">
          <button
            className={`action-btn ${showLabelsOnly ? 'active' : ''}`}
            onClick={() => setShowLabelsOnly(!showLabelsOnly)}
            title="Filter labels"
          >
            <Filter size={13} />
            <span>Labels Only</span>
          </button>

          <button className="action-btn" onClick={handleCopy} title="Copy raw TAC to clipboard">
            {copied ? <Check size={13} className="text-emerald-400" /> : <Copy size={13} />}
            <span>{copied ? 'Copied' : 'Copy TAC'}</span>
          </button>
        </div>
      </div>

      <div className="code-listing-container">
        {displayedTac.map((inst) => {
          if (inst.isLabel) {
            return (
              <div key={inst.index} className="tac-line tac-label-row">
                <span className="tac-index">{inst.index}:</span>
                <span className="tac-label-badge">label {inst.label}</span>
              </div>
            );
          }

          return (
            <div key={inst.index} className="tac-line">
              <span className="tac-index">{inst.index}:</span>
              <span className="tac-op-badge">{inst.op}</span>
              <span className="tac-args-text">
                {inst.args.map((arg, aIdx) => (
                  <span key={aIdx} className="tac-arg-token">
                    {arg}
                    {aIdx < inst.args.length - 1 && ', '}
                  </span>
                ))}
              </span>
            </div>
          );
        })}
      </div>
    </div>
  );
};
