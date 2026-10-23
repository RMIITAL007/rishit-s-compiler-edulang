import React, { useEffect } from 'react';
import { X, BookOpen, Layers, Cpu, ShieldCheck, Zap, Terminal, ArrowRight, Code } from 'lucide-react';

interface ArchitectureModalProps {
  isOpen: boolean;
  onClose: () => void;
}

export const ArchitectureModal: React.FC<ArchitectureModalProps> = ({ isOpen, onClose }) => {
  useEffect(() => {
    const handleKeyDown = (e: KeyboardEvent) => {
      if (e.key === 'Escape') onClose();
    };
    if (isOpen) {
      window.addEventListener('keydown', handleKeyDown);
    }
    return () => window.removeEventListener('keydown', handleKeyDown);
  }, [isOpen, onClose]);

  if (!isOpen) return null;

  const pipelineStages = [
    { num: '01', name: 'Source Code', desc: 'EduLang source files (.edu) with C-like syntax and strict int/bool typing.' },
    { num: '02', name: 'Lexer', desc: 'Tokenizes characters into Token stream with line, column, and span coordinates.' },
    { num: '03', name: 'Parser', desc: 'Recursive-descent syntax parser constructing the Abstract Syntax Tree (AST).' },
    { num: '04', name: 'Semantic Analyzer', desc: 'Scoped symbol tables, type validation, and lexical variable shadowing (x$1).' },
    { num: '05', name: 'TAC IR Generator', desc: 'Lowers AST into three-address code quads with short-circuit boolean jumps.' },
    { num: '06', name: 'Optimizer', desc: 'Intra-block constant expression folding and compile-time value evaluation.' },
    { num: '07', name: 'Code Generator', desc: 'Translates optimized TAC into clean, readable EduLang pseudo-assembly.' },
    { num: '08', name: 'Virtual Machine', desc: 'Direct TAC execution engine with checked 64-bit integer arithmetic and zero-division guards.' },
    { num: '09', name: 'Diagnostics', desc: 'Precise error reporting with source carets (^) and educational hints (= hint:).' },
  ];

  return (
    <div className="modal-backdrop" onClick={onClose}>
      <div className="modal-content" onClick={(e) => e.stopPropagation()}>
        <div className="modal-header">
          <div className="modal-title-group">
            <div className="modal-icon-badge">
              <BookOpen size={20} />
            </div>
            <div>
              <h2 className="modal-title">EduLang Compiler Architecture Guide</h2>
              <p className="modal-subtitle">Course BCSE307P · Compiler Design Laboratory Final Project</p>
            </div>
          </div>
          <button className="modal-close-btn" onClick={onClose} title="Close (Esc)">
            <X size={18} />
          </button>
        </div>

        <div className="modal-body">
          {/* 1. Pipeline Overview */}
          <section className="modal-section">
            <h3 className="section-title">
              <Layers size={16} className="text-blue-400" />
              <span>The 9-Stage Compiler Pipeline</span>
            </h3>
            <div className="pipeline-flow-grid">
              {pipelineStages.map((stage) => (
                <div key={stage.num} className="flow-card">
                  <div className="flow-card-num">{stage.num}</div>
                  <div className="flow-card-content">
                    <div className="flow-card-title">{stage.name}</div>
                    <div className="flow-card-desc">{stage.desc}</div>
                  </div>
                </div>
              ))}
            </div>
          </section>

          {/* 2. Key Architecture Innovations */}
          <section className="modal-section">
            <h3 className="section-title">
              <Zap size={16} className="text-cyan-400" />
              <span>Key Architectural Features for Faculty Review</span>
            </h3>
            <div className="features-grid">
              <div className="feature-box">
                <div className="feature-box-title">
                  <ShieldCheck size={16} className="text-emerald-400" />
                  <span>Lexical Scope Shadowing</span>
                </div>
                <p className="feature-box-text">
                  Inner block variables shadow outer variables without name clashes. The semantic analyzer assigns unique disambiguated storage keys (e.g., <code>x$1</code>, <code>x$2</code>), verified in the Symbol Table and assembly tabs.
                </p>
              </div>

              <div className="feature-box">
                <div className="feature-box-title">
                  <Zap size={16} className="text-blue-400" />
                  <span>Logical Short-Circuiting</span>
                </div>
                <p className="feature-box-text">
                  Operators <code>&&</code> and <code>||</code> generate conditional branch jumps in TAC, ensuring that right-hand expressions are never evaluated if the left-hand operand determines the outcome.
                </p>
              </div>

              <div className="feature-box">
                <div className="feature-box-title">
                  <Cpu size={16} className="text-amber-400" />
                  <span>Constant Expression Folding</span>
                </div>
                <p className="feature-box-text">
                  Intra-block compile-time evaluation reduces binary operations of constants (e.g. <code>1 + 2</code> $\rightarrow$ <code>3</code>). In <code>arithmetic_control.edu</code>, 1 of 17 expressions is folded (5.9% fold rate).
                </p>
              </div>

              <div className="feature-box">
                <div className="feature-box-title">
                  <Terminal size={16} className="text-rose-400" />
                  <span>Sandboxed Subprocess Runner</span>
                </div>
                <p className="feature-box-text">
                  The Node.js backend launches <code>./build/compiler</code> with a strict 2-second timeout and direct argument passing (no shell), safely protecting against command injection and infinite while loops.
                </p>
              </div>
            </div>
          </section>

          {/* 3. Language Grammar Reference */}
          <section className="modal-section">
            <h3 className="section-title">
              <Code size={16} className="text-purple-400" />
              <span>Grammar Cheat Sheet</span>
            </h3>
            <div className="grammar-card">
              <pre className="grammar-code">
{`program       -> statement* EOF
statement     -> declaration | assignment | print | if | while | block
block         -> "{" statement* "}"
declaration   -> "int" identifier "=" expression ";"
assignment    -> identifier "=" expression ";"
print         -> "print" "(" expression ")" ";"
if            -> "if" "(" expression ")" statement ("else" statement)?
while         -> "while" "(" expression ")" statement
expression    -> logical-or (supports +, -, *, /, <, <=, >, >=, ==, !=, &&, ||, !)`}
              </pre>
            </div>
          </section>
        </div>

        <div className="modal-footer">
          <span className="modal-footer-note">EduLang Compiler Design Laboratory · Built with C++20, CMake, Vite & React</span>
          <button className="btn btn-primary" onClick={onClose}>
            Got it, return to playground
          </button>
        </div>
      </div>
    </div>
  );
};
