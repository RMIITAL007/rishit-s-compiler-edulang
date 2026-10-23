import React from 'react';
import { Zap, CheckCircle2, Info, ArrowRight, Sparkles } from 'lucide-react';
import { OptimizationInfo } from '../../types';

interface OptimizationViewProps {
  optimization: OptimizationInfo | null;
}

export const OptimizationView: React.FC<OptimizationViewProps> = ({ optimization }) => {
  if (!optimization) {
    return (
      <div className="stage-placeholder-card">
        <div className="stage-icon">
          <Zap size={22} />
        </div>
        <div className="stage-placeholder-title">No Optimization Metrics Available</div>
        <div className="stage-placeholder-desc">
          Optimization was not performed or the compiler did not reach the IR optimization pass due to earlier stage diagnostics.
        </div>
      </div>
    );
  }

  const hasFolds = optimization.constantsFolded > 0;

  return (
    <div className="visualizer-container">
      {/* Metrics Cards */}
      <div className="metrics-grid">
        <div className="metric-card highlight">
          <div className="metric-header">
            <span className="metric-label">Constant Expressions Folded</span>
            <Sparkles size={16} className="text-cyan-400" />
          </div>
          <div className="metric-value">{optimization.constantsFolded}</div>
          <div className="metric-footer">
            {hasFolds ? 'Folded at compile-time into immediate values' : 'No foldable constant expressions found'}
          </div>
        </div>

        <div className="metric-card">
          <div className="metric-header">
            <span className="metric-label">Optimized TAC Instructions</span>
            <Zap size={16} className="text-blue-400" />
          </div>
          <div className="metric-value">
            {optimization.instructionsAfter !== undefined ? optimization.instructionsAfter : '—'}
          </div>
          <div className="metric-footer">Total intermediate instructions in active program</div>
        </div>

        {optimization.reductionRate && (
          <div className="metric-card">
            <div className="metric-header">
              <span className="metric-label">Optimization Expression Ratio</span>
              <CheckCircle2 size={16} className="text-emerald-400" />
            </div>
            <div className="metric-value">{optimization.reductionRate}</div>
            <div className="metric-footer">Proportion of instructions containing folded constants</div>
          </div>
        )}
      </div>

      {/* Compiler Design Context Note */}
      <div className="optimization-info-banner">
        <div className="info-icon-wrapper">
          <Info size={20} className="text-blue-400" />
        </div>
        <div>
          <div className="info-banner-title">EduLang Optimization Architecture</div>
          <div className="info-banner-text">
            EduLang's constant-folding optimizer operates as an intra-block transformation pass on Three-Address Code:
          </div>
          <ul className="info-banner-list">
            <li>
              <strong>Compile-Time Evaluation:</strong> Expressions composed exclusively of literal numbers or booleans (e.g., <code>1 + 2</code>) are evaluated at compile time and replaced with direct constant load instructions (<code>load_const t2, 3</code>).
            </li>
            <li>
              <strong>Preserved Instruction Count:</strong> Because binary operations are substituted with constant assignments in-place rather than eliminated in this intra-block pass, the total instruction count remains constant while eliminating runtime ALU operations.
            </li>
            <li>
              <strong>Single-Stream Output:</strong> The C++ compiler directly emits the optimized TAC to the target code generator and runtime virtual machine.
            </li>
          </ul>
        </div>
      </div>
    </div>
  );
};
