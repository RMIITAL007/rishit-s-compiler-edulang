import React, { useState, useEffect } from 'react';
import {
  ChevronRight,
  ChevronDown,
  Layers,
  Code,
  FolderTree,
  Minimize2,
  Maximize2,
} from 'lucide-react';
import { AstNode } from '../../types';

interface AstViewProps {
  ast: AstNode | null;
}

interface TreeNodeProps {
  node: AstNode;
  isAllExpanded: boolean | null;
}

const getNodeBadge = (name: string) => {
  if (name === 'Program') return { label: 'Root', color: 'node-root' };
  if (name.startsWith('Declaration')) return { label: 'Stmt:Decl', color: 'node-decl' };
  if (name.startsWith('Assignment')) return { label: 'Stmt:Assign', color: 'node-assign' };
  if (name === 'If') return { label: 'Stmt:If', color: 'node-ctrl' };
  if (name === 'While') return { label: 'Stmt:While', color: 'node-ctrl' };
  if (name === 'Block') return { label: 'Stmt:Block', color: 'node-block' };
  if (name === 'Print') return { label: 'Stmt:Print', color: 'node-io' };
  if (name.startsWith('Binary')) return { label: 'Expr:Binary', color: 'node-expr' };
  if (name.startsWith('Unary')) return { label: 'Expr:Unary', color: 'node-expr' };
  if (name.startsWith('Number')) return { label: 'Literal:Int', color: 'node-lit' };
  if (name.startsWith('Boolean')) return { label: 'Literal:Bool', color: 'node-lit' };
  if (name.startsWith('Name')) return { label: 'Variable', color: 'node-var' };
  return { label: 'Node', color: 'node-default' };
};

const TreeNodeItem: React.FC<TreeNodeProps> = ({ node, isAllExpanded }) => {
  const [isOpen, setIsOpen] = useState(true);

  // Sync with Expand All / Collapse All
  useEffect(() => {
    if (isAllExpanded !== null) {
      setIsOpen(isAllExpanded);
    }
  }, [isAllExpanded]);

  const hasChildren = node.children && node.children.length > 0;
  const badge = getNodeBadge(node.name);

  return (
    <div className="ast-node-wrapper">
      <div
        className={`ast-node-row ${hasChildren ? 'clickable' : ''}`}
        onClick={() => hasChildren && setIsOpen(!isOpen)}
      >
        <span className="expand-icon">
          {hasChildren ? (
            isOpen ? (
              <ChevronDown size={14} />
            ) : (
              <ChevronRight size={14} />
            )
          ) : (
            <span className="leaf-dot" />
          )}
        </span>

        <span className={`node-type-badge ${badge.color}`}>{badge.label}</span>
        <span className="node-title">{node.name}</span>

        {hasChildren && !isOpen && (
          <span className="child-count-pill">{node.children.length} hidden</span>
        )}
      </div>

      {hasChildren && isOpen && (
        <div className="ast-node-children">
          {node.children.map((child, index) => (
            <TreeNodeItem key={index} node={child} isAllExpanded={isAllExpanded} />
          ))}
        </div>
      )}
    </div>
  );
};

export const AstView: React.FC<AstViewProps> = ({ ast }) => {
  const [isAllExpanded, setIsAllExpanded] = useState<boolean | null>(null);

  if (!ast) {
    return (
      <div className="stage-placeholder-card">
        <div className="stage-icon">
          <Layers size={22} />
        </div>
        <div className="stage-placeholder-title">No AST Generated</div>
        <div className="stage-placeholder-desc">
          The Abstract Syntax Tree is unavailable. This typically occurs when a syntax error halts the parser before the AST can be completed.
        </div>
      </div>
    );
  }

  // Count total nodes
  const countNodes = (n: AstNode): number => {
    let count = 1;
    if (n.children) {
      for (const child of n.children) {
        count += countNodes(child);
      }
    }
    return count;
  };

  const totalNodes = countNodes(ast);

  return (
    <div className="visualizer-container">
      <div className="visualizer-header">
        <div className="visualizer-stats">
          <span className="stat-pill">
            <FolderTree size={13} />
            <span>{totalNodes} AST Nodes</span>
          </span>
          <span className="stat-pill muted">
            <Code size={13} />
            <span>Root: Program</span>
          </span>
        </div>

        <div className="visualizer-actions">
          <button
            className="action-btn"
            onClick={() => setIsAllExpanded(true)}
            title="Expand all nodes"
          >
            <Maximize2 size={13} />
            <span>Expand All</span>
          </button>
          <button
            className="action-btn"
            onClick={() => setIsAllExpanded(false)}
            title="Collapse all nodes"
          >
            <Minimize2 size={13} />
            <span>Collapse All</span>
          </button>
        </div>
      </div>

      <div className="tree-scroll-container">
        <TreeNodeItem node={ast} isAllExpanded={isAllExpanded} />
      </div>
    </div>
  );
};
