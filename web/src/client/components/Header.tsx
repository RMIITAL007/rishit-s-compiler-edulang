import { Cpu, CheckCircle2, AlertCircle, Clock, ServerOff, Loader2, BookOpen } from 'lucide-react';
import { CompileStatus } from '../types';

interface HeaderProps {
  status: CompileStatus;
  isBackendReady: boolean;
  onOpenArchitecture?: () => void;
}

export const Header: React.FC<HeaderProps> = ({ status, isBackendReady, onOpenArchitecture }) => {

  const renderStatus = () => {
    switch (status) {
      case 'compiling':
        return (
          <div className="status-pill compiling">
            <Loader2 size={14} className="status-dot animate-spin" />
            <span>Compiling in C++20...</span>
          </div>
        );
      case 'success':
        return (
          <div className="status-pill success">
            <CheckCircle2 size={14} />
            <span>Compiled & Executed (Exit 0)</span>
          </div>
        );
      case 'failure':
        return (
          <div className="status-pill failure">
            <AlertCircle size={14} />
            <span>Diagnostic Error (Exit 1)</span>
          </div>
        );
      case 'timeout':
        return (
          <div className="status-pill timeout">
            <Clock size={14} />
            <span>Timed Out (2000ms Limit)</span>
          </div>
        );
      case 'offline':
        return (
          <div className="status-pill offline">
            <ServerOff size={14} />
            <span>Backend Offline (Port 5001)</span>
          </div>
        );
      default:
        return (
          <div className="status-pill idle">
            <div className="status-dot" />
            <span>Ready {isBackendReady ? '(Backend Active)' : '(Connecting...)'}</span>
          </div>
        );
    }
  };

  return (
    <header className="app-header">
      <div className="brand-section">
        <div className="brand-logo">
          <Cpu size={22} />
        </div>
        <div>
          <div className="brand-title">
            EduLang Compiler Playground
            <span className="brand-version">C++20 Pipeline</span>
          </div>
          <div className="brand-subtitle">
            Compiler Design Laboratory · 9-Stage Architecture & Verification
          </div>
        </div>
      </div>

      <div className="header-actions">
        {onOpenArchitecture && (
          <button
            className="btn btn-secondary architecture-btn"
            onClick={onOpenArchitecture}
            title="View the 9-stage compiler pipeline and grammar cheat sheet"
          >
            <BookOpen size={14} className="text-cyan-400" />
            <span>Architecture Guide</span>
          </button>
        )}
        {renderStatus()}
      </div>

    </header>
  );
};
