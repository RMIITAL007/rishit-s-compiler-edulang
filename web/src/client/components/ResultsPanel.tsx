import React from 'react';
import {
  Terminal,
  FileCode,
  Binary,
  Layers,
  Cpu,
  Zap,
  Code2,
} from 'lucide-react';
import { CompileResponse, CompileStatus, StageTab } from '../types';
import { OutputView } from './visualizers/OutputView';
import { TokensView } from './visualizers/TokensView';
import { AstView } from './visualizers/AstView';
import { SemanticView } from './visualizers/SemanticView';
import { SymbolTableView } from './visualizers/SymbolTableView';
import { TacView } from './visualizers/TacView';
import { OptimizationView } from './visualizers/OptimizationView';
import { TargetCodeView } from './visualizers/TargetCodeView';

interface ResultsPanelProps {
  response: CompileResponse | null;
  status: CompileStatus;
  activeTab: StageTab;
  onTabChange: (tab: StageTab) => void;
  errorMessage: string | null;
}

export const ResultsPanel: React.FC<ResultsPanelProps> = ({
  response,
  status,
  activeTab,
  onTabChange,
  errorMessage,
}) => {
  const stages = response?.stages;

  const tabs: { id: StageTab; label: string; icon: React.ReactNode; badge?: number | string }[] = [
    { id: 'output', label: 'Console Output', icon: <Terminal size={14} /> },
    {
      id: 'tokens',
      label: 'Tokens',
      icon: <FileCode size={14} />,
      badge: stages?.tokens?.length ? stages.tokens.length : undefined,
    },
    {
      id: 'ast',
      label: 'AST',
      icon: <Layers size={14} />,
      badge: stages?.ast ? '✓' : undefined,
    },
    {
      id: 'symbols',
      label: 'Semantic Analysis',
      icon: <Binary size={14} />,
      badge: stages?.diagnostics?.length ? `${stages.diagnostics.length}!` : undefined,
    },
    {
      id: 'ir',
      label: 'Intermediate Code',
      icon: <Cpu size={14} />,
      badge: stages?.tac?.length ? stages.tac.length : undefined,
    },
    {
      id: 'opt',
      label: 'Optimization',
      icon: <Zap size={14} />,
      badge: stages?.optimization?.constantsFolded ? `${stages.optimization.constantsFolded} fold` : undefined,
    },
    {
      id: 'target',
      label: 'Target Code',
      icon: <Code2 size={14} />,
      badge: stages?.targetCode?.length ? stages.targetCode.length : undefined,
    },
  ];

  const renderActiveVisualizer = () => {
    switch (activeTab) {
      case 'output':
        return (
          <OutputView
            response={response}
            status={status}
            errorMessage={errorMessage}
          />
        );
      case 'tokens':
        return <TokensView tokens={stages?.tokens ?? []} />;
      case 'ast':
        return <AstView ast={stages?.ast ?? null} />;
      case 'symbols':
        return (
          <>
            <SemanticView
              diagnostics={stages?.diagnostics ?? []}
              pipelineStatuses={stages?.pipelineStatuses ?? []}
              warnings={stages?.parsingStatus?.warnings}
            />
            {stages?.symbols && stages.symbols.length > 0 && (
              <div style={{ marginTop: '1.5rem' }}>
                <div style={{ padding: '0 0.5rem', marginBottom: '0.75rem', fontWeight: 600, fontSize: '0.9rem', color: '#94a3b8' }}>
                  Scoped Symbol Table
                </div>
                <SymbolTableView symbols={stages.symbols} />
              </div>
            )}
          </>
        );
      case 'ir':
        return <TacView tac={stages?.tac ?? []} />;
      case 'opt':
        return <OptimizationView optimization={stages?.optimization ?? null} />;
      case 'target':
        return <TargetCodeView targetCode={stages?.targetCode ?? []} />;
      default:
        return null;
    }
  };

  return (
    <div className="panel right">
      <div className="tabs-bar">
        {tabs.map((tab) => (
          <button
            key={tab.id}
            className={`tab-btn ${activeTab === tab.id ? 'active' : ''}`}
            onClick={() => onTabChange(tab.id)}
          >
            {tab.icon}
            <span>{tab.label}</span>
            {tab.badge !== undefined && (
              <span className={`tab-badge ${tab.badge.toString().includes('!') ? 'badge-error' : ''}`}>
                {tab.badge}
              </span>
            )}
          </button>
        ))}
      </div>

      <div className="output-body">{renderActiveVisualizer()}</div>
    </div>
  );
};
