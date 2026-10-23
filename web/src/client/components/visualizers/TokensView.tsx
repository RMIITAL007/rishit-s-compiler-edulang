import React, { useState } from 'react';
import { Search, FileCode, Hash, MapPin, Tag } from 'lucide-react';
import { TokenItem } from '../../types';

interface TokensViewProps {
  tokens: TokenItem[];
}

export const TokensView: React.FC<TokensViewProps> = ({ tokens }) => {
  const [filter, setFilter] = useState('');

  if (!tokens || tokens.length === 0) {
    return (
      <div className="stage-placeholder-card">
        <div className="stage-icon">
          <FileCode size={22} />
        </div>
        <div className="stage-placeholder-title">No Tokens Available</div>
        <div className="stage-placeholder-desc">
          The lexer did not produce tokens or compilation failed before lexical analysis.
        </div>
      </div>
    );
  }

  const filtered = tokens.filter(
    (t) =>
      t.type.toLowerCase().includes(filter.toLowerCase()) ||
      t.lexeme.toLowerCase().includes(filter.toLowerCase())
  );

  const getBadgeClass = (type: string) => {
    if (type.startsWith("'")) return 'token-badge-keyword';
    if (type === 'identifier') return 'token-badge-ident';
    if (type === 'number') return 'token-badge-number';
    if (type === 'end of file') return 'token-badge-eof';
    return 'token-badge-symbol';
  };

  return (
    <div className="visualizer-container">
      <div className="visualizer-header">
        <div className="visualizer-stats">
          <span className="stat-pill">
            <Hash size={13} />
            <span>{tokens.length} Total Tokens</span>
          </span>
          {filter && (
            <span className="stat-pill muted">
              <span>{filtered.length} Matches</span>
            </span>
          )}
        </div>

        <div className="search-bar">
          <Search size={14} className="search-icon" />
          <input
            type="text"
            className="search-input"
            placeholder="Filter by type or lexeme..."
            value={filter}
            onChange={(e) => setFilter(e.target.value)}
          />
          {filter && (
            <button className="clear-btn" onClick={() => setFilter('')}>
              ×
            </button>
          )}
        </div>
      </div>

      <div className="table-wrapper">
        <table className="data-table">
          <thead>
            <tr>
              <th style={{ width: '60px' }}>#</th>
              <th style={{ width: '100px' }}>
                <span className="th-content"><MapPin size={12} /> Position</span>
              </th>
              <th>
                <span className="th-content"><Tag size={12} /> Token Type</span>
              </th>
              <th>Lexeme</th>
            </tr>
          </thead>
          <tbody>
            {filtered.map((token, idx) => (
              <tr key={idx} className="table-row">
                <td className="row-num">{idx + 1}</td>
                <td className="row-pos">
                  {token.line}:{token.column}
                </td>
                <td>
                  <span className={`token-badge ${getBadgeClass(token.type)}`}>
                    {token.type}
                  </span>
                </td>
                <td className="row-lexeme">
                  {token.type === 'end of file' ? (
                    <span className="eof-indicator">EOF</span>
                  ) : (
                    <code>{token.lexeme}</code>
                  )}
                </td>
              </tr>
            ))}
            {filtered.length === 0 && (
              <tr>
                <td colSpan={4} className="empty-table-cell">
                  No tokens match "{filter}"
                </td>
              </tr>
            )}
          </tbody>
        </table>
      </div>
    </div>
  );
};
