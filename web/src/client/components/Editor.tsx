import React, { useRef, useEffect } from 'react';
import { Play, RotateCcw, Code2 } from 'lucide-react';
import { SAMPLES } from '../samples';

interface EditorProps {
  source: string;
  onChange: (value: string) => void;
  onCompile: () => void;
  onReset: () => void;
  isCompiling: boolean;
  selectedSampleId: string;
  onSelectSample: (id: string) => void;
}

export const Editor: React.FC<EditorProps> = ({
  source,
  onChange,
  onCompile,
  onReset,
  isCompiling,
  selectedSampleId,
  onSelectSample,
}) => {
  const lineNumbersRef = useRef<HTMLDivElement>(null);
  const textareaRef = useRef<HTMLTextAreaElement>(null);

  const lines = source.split('\n');
  const lineCount = lines.length;

  const handleScroll = () => {
    if (textareaRef.current && lineNumbersRef.current) {
      lineNumbersRef.current.scrollTop = textareaRef.current.scrollTop;
    }
  };

  const handleKeyDown = (e: React.KeyboardEvent<HTMLTextAreaElement>) => {
    // Handle Cmd+Enter or Ctrl+Enter to compile
    if ((e.metaKey || e.ctrlKey) && e.key === 'Enter') {
      e.preventDefault();
      if (!isCompiling) {
        onCompile();
      }
      return;
    }

    // Handle Tab key to insert 4 spaces
    if (e.key === 'Tab') {
      e.preventDefault();
      const start = e.currentTarget.selectionStart;
      const end = e.currentTarget.selectionEnd;
      const updated = source.substring(0, start) + '    ' + source.substring(end);
      onChange(updated);
      requestAnimationFrame(() => {
        if (textareaRef.current) {
          textareaRef.current.selectionStart = textareaRef.current.selectionEnd = start + 4;
        }
      });
    }
  };

  // Group samples by category
  const categories = Array.from(new Set(SAMPLES.map((s) => s.category)));

  return (
    <div className="panel left">
      <div className="panel-header">
        <div className="panel-title">
          <Code2 size={16} />
          <span>EduLang Source Code</span>
        </div>

        <div className="panel-controls">
          <select
            className="sample-select"
            value={selectedSampleId}
            onChange={(e) => onSelectSample(e.target.value)}
            disabled={isCompiling}
            title="Load sample program from examples/"
          >
            {categories.map((cat) => (
              <optgroup key={cat} label={cat}>
                {SAMPLES.filter((s) => s.category === cat).map((s) => (
                  <option key={s.id} value={s.id}>
                    {s.title}
                  </option>
                ))}
              </optgroup>
            ))}
          </select>

          <button
            className="btn btn-secondary"
            onClick={onReset}
            disabled={isCompiling}
            title="Reset code to original sample"
          >
            <RotateCcw size={14} />
            <span>Reset</span>
          </button>

          <button
            className="btn btn-primary"
            onClick={onCompile}
            disabled={isCompiling}
            title="Compile and run in C++ compiler (Cmd+Enter)"
          >
            <Play size={14} />
            <span>{isCompiling ? 'Compiling...' : 'Compile'}</span>
          </button>
        </div>
      </div>

      <div className="editor-container">
        <div className="line-numbers" ref={lineNumbersRef}>
          {Array.from({ length: lineCount }, (_, i) => (
            <div key={i + 1}>{i + 1}</div>
          ))}
        </div>

        <textarea
          ref={textareaRef}
          className="code-textarea"
          value={source}
          onChange={(e) => onChange(e.target.value)}
          onScroll={handleScroll}
          onKeyDown={handleKeyDown}
          placeholder="// Enter EduLang code or select an example above..."
          spellCheck={false}
          autoComplete="off"
          autoCapitalize="off"
        />
      </div>

      <div className="editor-footer">
        <div>
          {lineCount} {lineCount === 1 ? 'line' : 'lines'} · {source.length} characters
        </div>
        <div>Press <strong>⌘ + Enter</strong> (or <strong>Ctrl + Enter</strong>) to compile</div>
      </div>
    </div>
  );
};
