import React, { useState } from 'react';
import { Binary, Search, MapPin, Tag, Layers, CheckCircle, XCircle } from 'lucide-react';
import { SymbolEntry } from '../../types';

interface SymbolTableViewProps {
  symbols: SymbolEntry[];
}

export const SymbolTableView: React.FC<SymbolTableViewProps> = ({ symbols }) => {
  const [filter, setFilter] = useState('');

  if (!symbols || symbols.length === 0) {
    return (
      <div className="stage-placeholder-card">
        <div className="stage-icon">
          <Binary size={22} />
        </div>
        <div className="stage-placeholder-title">No Symbols in Symbol Table</div>
        <div className="stage-placeholder-desc">
          No variable declarations were recorded. This occurs if the source program has no <code>int</code> declarations or if parsing failed prior to semantic analysis.
        </div>
      </div>
    );
  }

  const filtered = symbols.filter(
    (s) =>
      s.sourceName.toLowerCase().includes(filter.toLowerCase()) ||
      s.storageName.toLowerCase().includes(filter.toLowerCase()) ||
      s.scope.toLowerCase().includes(filter.toLowerCase()) ||
      s.type.toLowerCase().includes(filter.toLowerCase())
  );

  const activeCount = symbols.filter((s) => s.status === 'active').length;
  const shadowedCount = symbols.length - activeCount;

  return (
    <div className="visualizer-container">
      <div className="visualizer-header">
        <div className="visualizer-stats">
          <span className="stat-pill">
            <CheckCircle size={13} className="text-emerald-400" />
            <span>{activeCount} Active</span>
          </span>
          {shadowedCount > 0 && (
            <span className="stat-pill muted">
              <XCircle size={13} className="text-slate-400" />
              <span>{shadowedCount} Out of Scope</span>
            </span>
          )}
        </div>

        <div className="search-bar">
          <Search size={14} className="search-icon" />
          <input
            type="text"
            className="search-input"
            placeholder="Search by name, scope, or type..."
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
              <th>Source Name</th>
              <th>Storage Key</th>
              <th>
                <span className="th-content"><Layers size={12} /> Scope Depth</span>
              </th>
              <th>Status</th>
              <th>
                <span className="th-content"><Tag size={12} /> Type</span>
              </th>
              <th>
                <span className="th-content"><MapPin size={12} /> Declared At</span>
              </th>
            </tr>
          </thead>
          <tbody>
            {filtered.map((entry, idx) => (
              <tr key={idx} className="table-row">
                <td className="font-semibold text-slate-100">
                  <code>{entry.sourceName}</code>
                </td>
                <td className="text-slate-300">
                  <span className="storage-badge">{entry.storageName}</span>
                </td>
                <td>
                  <span className={`scope-badge ${entry.scope === 'global' ? 'global' : 'block'}`}>
                    {entry.scope}
                  </span>
                </td>
                <td>
                  <span
                    className={`status-pill-small ${
                      entry.status === 'active' ? 'status-active' : 'status-inactive'
                    }`}
                  >
                    {entry.status === 'active' ? 'active' : 'out of scope'}
                  </span>
                </td>
                <td>
                  <span className={`type-badge type-${entry.type}`}>
                    {entry.type}
                  </span>
                </td>
                <td className="row-pos">
                  {entry.line}:{entry.column}
                </td>
              </tr>
            ))}
            {filtered.length === 0 && (
              <tr>
                <td colSpan={6} className="empty-table-cell">
                  No symbols match "{filter}"
                </td>
              </tr>
            )}
          </tbody>
        </table>
      </div>
    </div>
  );
};
